/**
 * @file		can_comm.c
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "can_comm.h"

#include "bal_fsm.h"
#include "bms_fsm.h"
#include "cli_bms.h"
#include "pack/current.h"
#include "pack/pack.h"
#include "pack/temperature.h"
#include "pack/voltage.h"
#include "mainboard_config.h"
#include "usart.h"
#include "bootloader.h"
#include "primary/network.h"

#include <string.h>

struct {
    uint16_t cellboard0;
    uint16_t cellboard1;
    uint16_t cellboard2;
    uint16_t cellboard3;
    uint16_t cellboard4;
    uint16_t cellboard5;
} cellboards_msgs;

CAN_TxHeaderTypeDef tx_header;

bool can_forward;

void can_tx_header_init() {
    tx_header.ExtId = 0;
    tx_header.IDE   = CAN_ID_STD;
    tx_header.RTR   = CAN_RTR_DATA;
}

void can_bms_init() {
    CAN_FilterTypeDef filter;
    filter.FilterMode       = CAN_FILTERMODE_IDMASK;
    filter.FilterIdLow      = 0 << 5;                 // Take all ids from 0
    filter.FilterIdHigh     = ((1U << 11) - 1) << 5;  // to 2^11 - 1
    filter.FilterMaskIdHigh = 0 << 5;                 // Don't care on can id bits
    filter.FilterMaskIdLow  = 0 << 5;                 // Don't care on can id bits
    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of: 
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterBank           = 14;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;
    filter.FilterActivation     = ENABLE;
    filter.SlaveStartFilterBank = CAN_SLAVE_START_FILTER_BANK;

    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);
    HAL_CAN_ActivateNotification(&BMS_CAN, CAN_IT_ERROR | CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_Start(&BMS_CAN);

    can_tx_header_init();
}

void can_car_init() {
    CAN_FilterTypeDef filter;
    filter.FilterMode       = CAN_FILTERMODE_IDMASK;
    filter.FilterIdLow      = 0 << 5;                 // Take all ids from 0
    filter.FilterIdHigh     = ((1U << 11) - 1) << 5;  // to 2^11 - 1
    filter.FilterMaskIdHigh = 0 << 5;                 // Don't care on can id bits
    filter.FilterMaskIdLow  = 0 << 5;                 // Don't care on can id bits
    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of: 
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO1;
    filter.FilterBank           = 0;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;
    filter.FilterActivation     = ENABLE;
    filter.SlaveStartFilterBank = CAN_SLAVE_START_FILTER_BANK;

    HAL_CAN_ConfigFilter(&CAR_CAN, &filter);
    HAL_CAN_ActivateNotification(&CAR_CAN, CAN_IT_ERROR | CAN_IT_RX_FIFO1_MSG_PENDING);
    HAL_CAN_Start(&CAR_CAN);

    can_tx_header_init();
}

HAL_StatusTypeDef CAN_WAIT(CAN_HandleTypeDef *hcan, uint8_t timeout) {
    uint32_t tick = HAL_GetTick();
    while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0) {
        if(HAL_GetTick() - tick > timeout) return HAL_TIMEOUT;
    }
    return HAL_OK;
}

HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    if(CAN_WAIT(hcan, 3) != HAL_OK) return HAL_TIMEOUT;

    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, NULL);
    if (status != HAL_OK) {
        error_set(ERROR_CAN, 0, HAL_GetTick());
        //cli_bms_debug("CAN: Error sending message", 27);

    } else {
        error_reset(ERROR_CAN, 0);
    }

    return status;
}

HAL_StatusTypeDef can_car_send(uint16_t id) {
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    if(can_forward && id != PRIMARY_HV_CAN_FORWARD_STATUS_FRAME_ID)
        return HAL_BUSY;

    tx_header.StdId = id;

    if (id == PRIMARY_HV_VOLTAGE_FRAME_ID) {
        primary_hv_voltage_t raw_volts = { 0 };
        primary_hv_voltage_converted_t conv_volts = { 0 };

        conv_volts.bus_voltage = voltage_get_vts_p();
        conv_volts.pack_voltage = voltage_get_vbat_adc();

        primary_hv_voltage_conversion_to_raw_struct(&raw_volts, &conv_volts);

        raw_volts.max_cell_voltage = voltage_get_cell_max(NULL);
        raw_volts.min_cell_voltage = voltage_get_cell_min(NULL);

        tx_header.DLC = primary_hv_voltage_pack(buffer, &raw_volts, CAN_MAX_PAYLOAD_LENGTH);
    } else if (id == PRIMARY_HV_CURRENT_FRAME_ID) {
        primary_hv_current_t raw_curr;
        primary_hv_current_converted_t conv_curr;

        conv_curr.current = current_get_current();
        conv_curr.power = conv_curr.current * voltage_get_vts_p();

        primary_hv_current_conversion_to_raw_struct(&raw_curr, &conv_curr);

        tx_header.DLC = primary_hv_current_pack(buffer, &raw_curr, CAN_MAX_PAYLOAD_LENGTH);
    } else if (id == PRIMARY_TS_STATUS_FRAME_ID) {
        primary_ts_status_t raw_status;
        primary_ts_status_converted_t conv_status;
        conv_status.ts_status = primary_ts_status_ts_status_OFF;

        switch (fsm_get_state(bms.fsm)) {
            case BMS_IDLE:
                conv_status.ts_status = primary_ts_status_ts_status_OFF;
                break;
            case BMS_AIRN_CLOSE:
            case BMS_AIRN_STATUS:
            case BMS_PRECHARGE:
                conv_status.ts_status = primary_ts_status_ts_status_PRECHARGE;
                break;
            case BMS_ON:
                conv_status.ts_status = primary_ts_status_ts_status_ON;
                break;
            case BMS_FAULT:
                conv_status.ts_status = primary_ts_status_ts_status_FATAL;
                break;
        }

        // Convert ts status to raw
        primary_ts_status_conversion_to_raw_struct(&raw_status, &conv_status);
        
        tx_header.DLC = primary_ts_status_pack(buffer, &raw_status, CAN_MAX_PAYLOAD_LENGTH);
    } else if (id == PRIMARY_HV_TEMP_FRAME_ID) {
        primary_hv_temp_t raw_temp;
        primary_hv_temp_converted_t conv_temp;
        
        conv_temp.average_temp = (float)temperature_get_average() / 2.56f * 655.36f;
        conv_temp.min_temp = (float)temperature_get_min() / 2.56f * 655.36f;
        conv_temp.max_temp = (float)temperature_get_max() / 2.56f * 655.36f;

        // Convert temperatures to raw
        primary_hv_temp_conversion_to_raw_struct(&raw_temp, &conv_temp);

        tx_header.DLC = primary_hv_temp_pack(buffer, &raw_temp, CAN_MAX_PAYLOAD_LENGTH);
    } else if (id == PRIMARY_HV_ERRORS_FRAME_ID) {
        primary_hv_errors_t raw_errors = { 0 };
        primary_hv_errors_converted_t conv_errors  = { 0 };

        error_t errors[100];
        error_dump(errors);

        for(uint8_t i = 0; i < error_count(); ++i) {
            switch(errors[i].id) {
                case ERROR_CELL_LOW_VOLTAGE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_low_voltage = 1;
                    else
                        conv_errors.errors_cell_low_voltage = 1;
                    break;
                case ERROR_CELL_UNDER_VOLTAGE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_under_voltage = 1;
                    else
                        conv_errors.errors_cell_under_voltage = 1;
                    break;
                case ERROR_CELL_OVER_VOLTAGE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_over_voltage = 1;
                    else
                        conv_errors.errors_cell_over_voltage = 1;
                    break;
                case ERROR_CELL_OVER_TEMPERATURE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_over_temperature = 1;
                    else
                        conv_errors.errors_cell_over_temperature = 1;
                    break;
                case ERROR_CELL_HIGH_TEMPERATURE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_high_temperature = 1;
                    else
                        conv_errors.errors_cell_high_temperature = 1;
                    break;
                case ERROR_OVER_CURRENT:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_over_current = 1;
                    else
                        conv_errors.errors_over_current = 1;
                    break;
                case ERROR_CAN:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_can = 1;
                    else
                        conv_errors.errors_can = 1;
                    break;
                case ERROR_INT_VOLTAGE_MISMATCH:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_int_voltage_mismatch = 1;
                    else
                        conv_errors.errors_int_voltage_mismatch = 1;
                    break;
                case ERROR_CELLBOARD_COMM:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cellboard_comm = 1;
                    else
                        conv_errors.errors_cellboard_comm = 1;
                    break;
                case ERROR_CELLBOARD_INTERNAL:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cellboard_internal = 1;
                    else
                        conv_errors.errors_cellboard_internal = 1;
                    break;
                case ERROR_FEEDBACK:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_feedback = 1;
                    else
                        conv_errors.errors_feedback = 1;
                    break;
                case ERROR_FEEDBACK_CIRCUITRY:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_feedback_circuitry = 1;
                    else
                        conv_errors.errors_feedback_circuitry = 1;
                    break;
                case ERROR_EEPROM_COMM:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_eeprom_comm = 1;
                    else
                        conv_errors.errors_eeprom_comm = 1;
                    break;
                case ERROR_EEPROM_WRITE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_eeprom_write = 1;
                    else
                        conv_errors.errors_eeprom_write = 1;
                    break;
                default:
                    break;
            }
        }

        // Convert errors to raw
        primary_hv_errors_conversion_to_raw_struct(&raw_errors, &conv_errors);

        tx_header.DLC = primary_hv_errors_pack(buffer, &raw_errors, CAN_MAX_PAYLOAD_LENGTH);
    } else if (id == PRIMARY_HV_CELL_BALANCING_STATUS_FRAME_ID) {
        primary_hv_cell_balancing_status_t raw_bal_status;
        primary_hv_cell_balancing_status_converted_t conv_bal_status;

        switch (fsm_get_state(bal.fsm)) {
            case BAL_OFF:
                conv_bal_status.balancing_status = primary_hv_cell_balancing_status_balancing_status_OFF;
                break;
            case BAL_COMPUTE:
            case BAL_COOLDOWN:
            case BAL_DISCHARGE:
                conv_bal_status.balancing_status = primary_hv_cell_balancing_status_balancing_status_ON;
                break;
            default:
                conv_bal_status.balancing_status = primary_hv_cell_balancing_status_balancing_status_OFF;
                break;
        }

        // Convert bal status to raw
        primary_hv_cell_balancing_status_conversion_to_raw_struct(&raw_bal_status, &conv_bal_status);

        tx_header.DLC = primary_hv_cell_balancing_status_pack(buffer, &raw_bal_status, CAN_MAX_PAYLOAD_LENGTH);
    } else if (id == PRIMARY_HV_CELLS_TEMP_FRAME_ID) {
        uint8_t status = 0;
        temperature_t * temps = temperature_get_all();

        primary_hv_cells_temp_t raw_temps;
        primary_hv_cells_temp_converted_t conv_temps;

        for (uint8_t i = 0; i < PACK_TEMP_COUNT; i += 6) {
            conv_temps.start_index = i;
            conv_temps.temp_0 = temps[i];
            conv_temps.temp_1 = temps[i + 1];
            conv_temps.temp_2 = temps[i + 2];
            conv_temps.temp_3 = temps[i + 3];
            conv_temps.temp_4 = temps[i + 4];
            conv_temps.temp_5 = temps[i + 5];

            // Convert temperatures to raw
            primary_hv_cells_temp_conversion_to_raw_struct(&raw_temps, &conv_temps);

            tx_header.DLC = primary_hv_cells_temp_pack(buffer, &raw_temps, CAN_MAX_PAYLOAD_LENGTH);
            status |= can_send(&CAR_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        
        return status == 0 ? HAL_OK : HAL_ERROR;
    } else if (id == PRIMARY_HV_CELLS_VOLTAGE_FRAME_ID) {
        uint8_t status = 0;
        voltage_t * volts = voltage_get_cells();

        primary_hv_cells_voltage_t raw_volts;
        primary_hv_cells_voltage_converted_t conv_volts;

        for (uint8_t i = 0; i < PACK_CELL_COUNT; i += 3) {
            conv_volts.start_index = i;
            conv_volts.voltage_0 = volts[i];
            conv_volts.voltage_1 = volts[i + 1];
            conv_volts.voltage_2 = volts[i + 2];

            // Convert volatges to raw
            primary_hv_cells_voltage_conversion_to_raw_struct(&raw_volts, &conv_volts);

            tx_header.DLC = primary_hv_cells_voltage_pack(buffer, &raw_volts, CAN_MAX_PAYLOAD_LENGTH);
            status |= can_send(&CAR_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }

        return status == 0 ? HAL_OK : HAL_ERROR;
    } else if (id == PRIMARY_HV_CAN_FORWARD_STATUS_FRAME_ID) {
        primary_hv_can_forward_status_t raw_can_forward;
        primary_hv_can_forward_status_converted_t conv_can_forward;

        conv_can_forward.can_forward_status = primary_hv_can_forward_status_can_forward_status_OFF;
        if (can_forward)
            conv_can_forward.can_forward_status = primary_hv_can_forward_status_can_forward_status_ON;

        // Convert can forward to raw
        primary_hv_can_forward_status_conversion_to_raw_struct(&raw_can_forward, &conv_can_forward);

        tx_header.DLC = primary_hv_can_forward_status_pack(buffer, &raw_can_forward, CAN_MAX_PAYLOAD_LENGTH);
    } else if (id == PRIMARY_HV_VERSION_FRAME_ID) {
        primary_hv_version_t raw_version;
        primary_hv_version_converted_t conv_version;

        conv_version.canlib_build_time = 1;
        conv_version.component_version = 1;

        primary_hv_version_conversion_to_raw_struct(&raw_version, &conv_version);

        tx_header.DLC = primary_hv_version_pack(buffer, &raw_version, CAN_MAX_PAYLOAD_LENGTH);
    } else
        return HAL_ERROR;

    return can_send(&CAR_CAN, buffer, &tx_header);
}

HAL_StatusTypeDef can_bms_send(uint16_t id) {
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    if(can_forward && id != BMS_FW_UPDATE_FRAME_ID)
        return HAL_BUSY;
    tx_header.StdId = id;

    if (id == BMS_BALANCING_FRAME_ID) {
        uint8_t status = 0;

        register uint16_t i;
        for (i = 0; i < CELLBOARD_COUNT; ++i) {
            bms_balancing_t raw_bal;

            // Convert bal to raw
            bms_balancing_conversion_to_raw_struct(&raw_bal, &bal.cells[i]);

            tx_header.DLC = bms_balancing_pack(buffer, &raw_bal, CAN_MAX_PAYLOAD_LENGTH);
            status |= can_send(&BMS_CAN, buffer, &tx_header);
        }
        return status == 0 ? HAL_OK : HAL_ERROR;
    } else if (id == BMS_FW_UPDATE_FRAME_ID) {
        bms_fw_update_t raw_fw_update;
        bms_fw_update_converted_t conv_fw_update;

        conv_fw_update.board_index = 0;

        // Convert fw update to raw
        bms_fw_update_conversion_to_raw_struct(&raw_fw_update, &conv_fw_update);

        tx_header.DLC = bms_fw_update_pack(buffer, &raw_fw_update, CAN_MAX_PAYLOAD_LENGTH);  //TODO: set board_index
        return can_send(&BMS_CAN, buffer, &tx_header);
    } else
        return HAL_ERROR;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef * hcan) {
    uint8_t rx_data[8] = { '\0' };
    CAN_RxHeaderTypeDef rx_header;

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        error_set(ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == BMS_CAN.Instance) {
        if (can_forward && (rx_header.StdId >= BMS_FLASH_CELLBOARD_0_RX_FRAME_ID && rx_header.StdId <= BMS_FLASH_CELLBOARD_5_RX_FRAME_ID)) {
            uint8_t forward_data[8];
            tx_header.StdId = rx_header.StdId;
            tx_header.DLC = rx_header.DLC;
            *((uint64_t *)forward_data) = *((uint64_t *)rx_data);
            can_send(&CAR_CAN, rx_data, &tx_header);
            return;
        }

        error_reset(ERROR_CAN, 1);
        if (rx_header.StdId == BMS_VOLTAGES_FRAME_ID) {
            uint8_t offset = 0;
            bms_voltages_t raw_volts;
            bms_voltages_unpack(&raw_volts, rx_data, CAN_MAX_PAYLOAD_LENGTH);
            switch (rx_header.DLC) {
                case BMS_VOLTAGES_CELLBOARD_ID_CELLBOARD_0_CHOICE:
                    ++cellboards_msgs.cellboard0;
                    offset = voltage_get_cellboard_offset(0);
                    break;
                case BMS_VOLTAGES_CELLBOARD_ID_CELLBOARD_1_CHOICE:
                    ++cellboards_msgs.cellboard1;
                    offset = voltage_get_cellboard_offset(1);
                    break;
                case BMS_VOLTAGES_CELLBOARD_ID_CELLBOARD_2_CHOICE:
                    ++cellboards_msgs.cellboard2;
                    offset = voltage_get_cellboard_offset(2);
                    break;
                case BMS_VOLTAGES_CELLBOARD_ID_CELLBOARD_3_CHOICE:
                    ++cellboards_msgs.cellboard3;
                    offset = voltage_get_cellboard_offset(3);
                    break;
                case BMS_VOLTAGES_CELLBOARD_ID_CELLBOARD_4_CHOICE:
                    ++cellboards_msgs.cellboard4;
                    offset = voltage_get_cellboard_offset(4);
                    break;
                case BMS_VOLTAGES_CELLBOARD_ID_CELLBOARD_5_CHOICE:
                    ++cellboards_msgs.cellboard5;
                    offset = voltage_get_cellboard_offset(5);
                    break;
                default:
                    break;
            }

            voltage_set_cells(raw_volts.start_index + offset, raw_volts.voltage0, raw_volts.voltage1, raw_volts.voltage2);
        } else if (rx_header.StdId == BMS_TEMPERATURES_FRAME_ID) {
            uint8_t offset = 0;
            bms_temperatures_t raw_temps;

            bms_temperatures_unpack(&raw_temps, rx_data, CAN_MAX_PAYLOAD_LENGTH);

            switch (rx_header.StdId) {
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_0_CHOICE:
                    ++cellboards_msgs.cellboard0;
                    offset = temperature_get_cellboard_offset(0);
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_1_CHOICE:
                    ++cellboards_msgs.cellboard1;
                    offset = temperature_get_cellboard_offset(1);
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_2_CHOICE:
                    ++cellboards_msgs.cellboard2;
                    offset = temperature_get_cellboard_offset(2);
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_3_CHOICE:
                    ++cellboards_msgs.cellboard3;
                    offset = temperature_get_cellboard_offset(3);
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_4_CHOICE:
                    ++cellboards_msgs.cellboard4;
                    offset = temperature_get_cellboard_offset(4);
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_5_CHOICE:
                    ++cellboards_msgs.cellboard5;
                    offset = temperature_get_cellboard_offset(5);
                    break;
                default:
                    break;
            }
            temperature_set_cells(
                raw_temps.start_index + offset,
                raw_temps.temp0,
                raw_temps.temp1,
                raw_temps.temp2,
                raw_temps.temp3);
        } else if (rx_header.StdId == BMS_BOARD_STATUS_FRAME_ID) {
            uint8_t index = 0;
            bms_board_status_t status;

            bms_board_status_unpack(&status, rx_data, CAN_MAX_PAYLOAD_LENGTH);
            switch (rx_header.StdId) {
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_0_CHOICE:
                    ++cellboards_msgs.cellboard0;
                    index = 0;
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_1_CHOICE:
                    ++cellboards_msgs.cellboard1;
                    index = 1;
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_2_CHOICE:
                    ++cellboards_msgs.cellboard2;
                    index = 2;
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_3_CHOICE:
                    ++cellboards_msgs.cellboard3;
                    index = 3;
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_4_CHOICE:
                    ++cellboards_msgs.cellboard4;
                    index = 4;
                    break;
                case BMS_TEMPERATURES_CELLBOARD_ID_CELLBOARD_5_CHOICE:
                    ++cellboards_msgs.cellboard5;
                    index = 5;
                    break;
            }
            bal.status[index] = status.balancing_status;
            /*
            if (index == 0)
                status.errors &= ~0b00001100;
            else if (index == 3)
                status.errors &= ~0b10000000;  //those adc are not working
            */
            uint32_t error_status = status.errors_can_comm |
                status.errors_ltc_comm |
                status.errors_open_wire |
                status.errors_temp_comm_0 | 
                status.errors_temp_comm_1 | 
                status.errors_temp_comm_2 | 
                status.errors_temp_comm_3 | 
                status.errors_temp_comm_4 | 
                status.errors_temp_comm_5;

            error_toggle_check(error_status != 0, ERROR_CELLBOARD_INTERNAL, index);
        } else {
            char buffer[50] = {0};
            sprintf(buffer, "%lx#%lx%lx\r\n", rx_header.StdId, *(uint32_t*)rx_data, *(((uint32_t*)rx_data)+1));
            HAL_UART_Transmit(&CLI_UART, (uint8_t*)buffer, strlen(buffer), 100);
        }
    }
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    uint8_t rx_data[8] = {'\0'};
    CAN_RxHeaderTypeDef rx_header;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rx_header, rx_data) != HAL_OK) {
        error_set(ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == CAR_CAN.Instance) {
        if (can_forward && (rx_header.StdId >= BMS_FLASH_CELLBOARD_0_TX_FRAME_ID && rx_header.StdId <= BMS_FLASH_CELLBOARD_5_TX_FRAME_ID)) {
            uint8_t forward_data[8];
            tx_header.StdId = rx_header.StdId;
            tx_header.DLC = rx_header.DLC;
            *((uint64_t *)forward_data) = *((uint64_t *)rx_data);
            can_send(&BMS_CAN, forward_data, &tx_header);
            return;
        }

        error_reset(ERROR_CAN, 1);

        if (rx_header.StdId == PRIMARY_SET_TS_STATUS_HANDCART_FRAME_ID) { // || rx_header.StdId == primary_ID_SET_TS_STATUS_DAS
            primary_set_ts_status_handcart_t ts_status;

            primary_set_ts_status_handcart_unpack(&ts_status, rx_data, CAN_MAX_PAYLOAD_LENGTH);

            switch (ts_status.ts_status_set) {
                case primary_set_ts_status_handcart_ts_status_set_OFF:
                    fsm_trigger_event(bms.fsm, BMS_EV_TS_OFF);
                    break;
                case primary_set_ts_status_handcart_ts_status_set_ON:
                    fsm_trigger_event(bms.fsm, BMS_EV_TS_ON);
                    break;
            }
        } else if (rx_header.StdId == PRIMARY_SET_CELL_BALANCING_STATUS_FRAME_ID) {
            primary_set_cell_balancing_status_t balancing_status;

            primary_set_cell_balancing_status_unpack(&balancing_status, rx_data, CAN_MAX_PAYLOAD_LENGTH);

            switch (balancing_status.set_balancing_status) {
                case primary_set_cell_balancing_status_set_balancing_status_ON:
                    fsm_trigger_event(bal.fsm, EV_BAL_START);
                    break;
                case primary_set_cell_balancing_status_set_balancing_status_OFF:
                    fsm_trigger_event(bal.fsm, EV_BAL_STOP);
                    break;
            }
        } else if (rx_header.StdId == PRIMARY_HANDCART_STATUS_FRAME_ID) {
            primary_handcart_status_t handcart_status;

            primary_handcart_status_unpack(&handcart_status, rx_data, CAN_MAX_PAYLOAD_LENGTH);
            
            bms.handcart_connected = handcart_status.connected;
        } else if (rx_header.StdId == PRIMARY_HV_CAN_FORWARD_FRAME_ID) {
            primary_hv_can_forward_t hv_can_forward;

            primary_hv_can_forward_unpack(&hv_can_forward, rx_data, CAN_MAX_PAYLOAD_LENGTH);

            switch (hv_can_forward.can_forward_set) {
                case primary_hv_can_forward_can_forward_set_OFF:
                    can_forward = 0;
                    break;
                case primary_hv_can_forward_can_forward_set_ON:
                    can_bms_send(BMS_FW_UPDATE_FRAME_ID);
                    can_forward = 1;
                    break;
            }
        } else if (rx_header.StdId == PRIMARY_BMS_HV_JMP_TO_BLT_FRAME_ID) {
            // JumpToBlt();
            HAL_NVIC_SystemReset();
        }
    }
}

void CAN_change_bitrate(CAN_HandleTypeDef *hcan, CAN_Bitrate bitrate) {
    /* De initialize CAN*/
    HAL_CAN_DeInit(hcan);
    switch (bitrate) {
        case CAN_BITRATE_1MBIT:
            hcan->Init.Prescaler = CAN_1MBIT_PRE;
            hcan->Init.TimeSeg1  = CAN_1MBIT_BS1;
            hcan->Init.TimeSeg2  = CAN_1MBIT_BS2;
            break;
        case CAN_BITRATE_125KBIT:
            hcan->Init.Prescaler = CAN_125KBIT_PRE;
            hcan->Init.TimeSeg1  = CAN_125KBIT_BS1;
            hcan->Init.TimeSeg2  = CAN_125KBIT_BS2;
            break;
    }
    if (HAL_CAN_Init(hcan) != HAL_OK) {
        Error_Handler();
    }
    if (hcan->Instance == BMS_CAN.Instance)
        can_bms_init();
    else if (hcan->Instance == CAR_CAN.Instance)
        can_car_init();
}

void can_cellboards_check() {
    error_toggle_check(cellboards_msgs.cellboard0 == 0, ERROR_CELLBOARD_COMM, 0);
    error_toggle_check(cellboards_msgs.cellboard1 == 0, ERROR_CELLBOARD_COMM, 1);
    error_toggle_check(cellboards_msgs.cellboard2 == 0, ERROR_CELLBOARD_COMM, 2);
    error_toggle_check(cellboards_msgs.cellboard3 == 0, ERROR_CELLBOARD_COMM, 3);
    error_toggle_check(cellboards_msgs.cellboard4 == 0, ERROR_CELLBOARD_COMM, 4);
    error_toggle_check(cellboards_msgs.cellboard5 == 0, ERROR_CELLBOARD_COMM, 5);

    memset(&cellboards_msgs, 0, sizeof(cellboards_msgs));
}