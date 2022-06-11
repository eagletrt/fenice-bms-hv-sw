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

HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    CAN_WAIT(hcan);

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

    if(can_forward && id != primary_id_HV_CAN_FORWARD_STATUS) return HAL_BUSY;

    tx_header.StdId = id;

    if (id == primary_id_HV_VOLTAGE) {
        primary_message_HV_VOLTAGE raw_volts = {0};
        primary_message_HV_VOLTAGE_conversion conv_volts = {0};

        conv_volts.bus_voltage = voltage_get_vts_p();
        conv_volts.pack_voltage = voltage_get_vbat_adc();

        primary_conversion_to_raw_HV_VOLTAGE(&raw_volts, &conv_volts);

        raw_volts.max_cell_voltage = voltage_get_cell_max(NULL);
        raw_volts.min_cell_voltage = voltage_get_cell_min(NULL);

        tx_header.DLC = primary_serialize_struct_HV_VOLTAGE(buffer, &raw_volts);
    } else if (id == primary_id_HV_CURRENT) {
        primary_message_HV_CURRENT raw_curr;
        primary_message_HV_CURRENT_conversion conv_curr;

        conv_curr.current = current_get_current();
        conv_curr.power = current_get_current() * voltage_get_vts_p();

        primary_conversion_to_raw_HV_CURRENT(&raw_curr, &conv_curr);

        tx_header.DLC = primary_serialize_struct_HV_CURRENT(buffer, &raw_curr);
    } else if (id == primary_id_TS_STATUS) {
        primary_TsStatus status = primary_TsStatus_OFF;
        switch (fsm_get_state(bms.fsm)) {
            case BMS_IDLE:
                status = primary_TsStatus_OFF;
                break;
            case BMS_AIRN_CLOSE:
            case BMS_AIRN_STATUS:
            case BMS_PRECHARGE:
                status = primary_TsStatus_PRECHARGE;
                break;
            case BMS_ON:
                status = primary_TsStatus_ON;
                break;
            case BMS_FAULT:
                status = primary_TsStatus_FATAL;
                break;
        }
        tx_header.DLC = primary_serialize_TS_STATUS(buffer, status);
    } else if (id == primary_id_HV_TEMP) {
        tx_header.DLC = primary_serialize_HV_TEMP(buffer, (uint16_t)temperature_get_average() / 2.56 * 655.36, (uint16_t)temperature_get_max() / 2.56 * 655.36, (uint16_t)temperature_get_min() / 2.56 * 655.36);
    } else if (id == primary_id_HV_ERRORS) {
        primary_HvErrors warnings = 0;
        primary_HvErrors errors = 0;
        error_t error_array[100];
        error_dump(error_array);

        for(uint8_t i=0; i<error_count(); ++i) {
            if(error_array[i].state == STATE_WARNING)
                CANLIB_BITSET(warnings, error_array[i].id);
            else if(error_array[i].state == STATE_FATAL)
                CANLIB_BITSET(errors, error_array[i].id);
        }

        tx_header.DLC = primary_serialize_HV_ERRORS(buffer, warnings, errors);
    } else if (id == primary_id_HV_CELL_BALANCING_STATUS) {
        primary_Toggle bal_status;
        switch (fsm_get_state(bal.fsm)) {
            case BAL_OFF:
                bal_status = primary_Toggle_OFF;
                break;
            case BAL_COMPUTE:
            case BAL_COOLDOWN:
            case BAL_DISCHARGE:
                bal_status = primary_Toggle_ON;
                break;
            default:
                bal_status = primary_Toggle_OFF;
                break;
        }
        tx_header.DLC = primary_serialize_HV_CELL_BALANCING_STATUS(buffer, bal_status);
    } else if (id == primary_id_HV_CELLS_TEMP) {
        uint8_t status = 0;
        temperature_t *temps = temperature_get_all();
        for (uint8_t i = 0; i < PACK_TEMP_COUNT; i += 6) {
            tx_header.DLC = primary_serialize_HV_CELLS_TEMP(
                buffer, i, temps[i], temps[i + 1], temps[i + 2], temps[i + 3], temps[i + 4], temps[i + 5], 0);
            status += can_send(&CAR_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        return status == 0 ? HAL_OK : HAL_ERROR;
    } else if (id == primary_id_HV_CELLS_VOLTAGE) {
        uint8_t status = 0;
        voltage_t *volts = voltage_get_cells();
        for (uint8_t i = 0; i < PACK_CELL_COUNT; i += 3) {
            tx_header.DLC = primary_serialize_HV_CELLS_VOLTAGE(buffer, volts[i], volts[i + 1], volts[i + 2], i);
            status += can_send(&CAR_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        return status == 0 ? HAL_OK : HAL_ERROR;
    } else if (id == primary_id_HV_CAN_FORWARD_STATUS) {
        tx_header.DLC = can_forward ? primary_serialize_HV_CAN_FORWARD_STATUS(buffer, primary_Toggle_ON) :
                                        primary_serialize_HV_CAN_FORWARD_STATUS(buffer, primary_Toggle_OFF);
    } else {
        return HAL_ERROR;
    }

    return can_send(&CAR_CAN, buffer, &tx_header);
}

HAL_StatusTypeDef can_bms_send(uint16_t id) {
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    if(can_forward && id != bms_id_FW_UPDATE) return HAL_BUSY;
    tx_header.StdId = id;

    if (id == bms_id_BALANCING) {
        uint8_t status = 0;
        uint8_t *distr = bms_get_cellboard_distribution();
        register uint16_t i;
        for (i = 0; i < CELLBOARD_COUNT; ++i) {
            tx_header.DLC = bms_serialize_BALANCING(buffer, bal.cells[i], distr[i]);
            status += can_send(&BMS_CAN, buffer, &tx_header);
        }
        return status == 0 ? HAL_OK : HAL_ERROR;  //TODO: ugly
    } else if (id == bms_id_FW_UPDATE) {
        tx_header.DLC = bms_serialize_FW_UPDATE(buffer, 0);  //TODO: set board_index
        return can_send(&BMS_CAN, buffer, &tx_header);
    } else {
        return HAL_ERROR;
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    uint8_t rx_data[8] = {'\0'};
    CAN_RxHeaderTypeDef rx_header;

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        error_set(ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == BMS_CAN.Instance) {
        if (can_forward) {
            tx_header.StdId = rx_header.StdId;
            tx_header.DLC = rx_header.DLC;
            can_send(&CAR_CAN, rx_data, &tx_header);
            return;
        }

        error_reset(ERROR_CAN, 1);
        if ((rx_header.StdId & bms_topic_mask_VOLTAGE_INFO) == bms_topic_filter_VOLTAGE_INFO) {
            uint8_t offset = 0;
            bms_message_VOLTAGES raw_volts;
            bms_deserialize_VOLTAGES(&raw_volts, rx_data);
            switch (rx_header.StdId) {
                case bms_id_VOLTAGES_CELLBOARD0:
                    ++cellboards_msgs.cellboard0;
                    offset = voltage_get_cellboard_offset(0);
                    break;
                case bms_id_VOLTAGES_CELLBOARD1:
                    ++cellboards_msgs.cellboard1;
                    offset = voltage_get_cellboard_offset(1);
                    break;
                case bms_id_VOLTAGES_CELLBOARD2:
                    ++cellboards_msgs.cellboard2;
                    offset = voltage_get_cellboard_offset(2);
                    break;
                case bms_id_VOLTAGES_CELLBOARD3:
                    ++cellboards_msgs.cellboard3;
                    offset = voltage_get_cellboard_offset(3);
                    break;
                case bms_id_VOLTAGES_CELLBOARD4:
                    ++cellboards_msgs.cellboard4;
                    offset = voltage_get_cellboard_offset(4);
                    break;
                case bms_id_VOLTAGES_CELLBOARD5:
                    ++cellboards_msgs.cellboard5;
                    offset = voltage_get_cellboard_offset(5);
                    break;
                default:
                    break;
            }
            voltage_set_cells(raw_volts.start_index + offset, raw_volts.voltage0, raw_volts.voltage1, raw_volts.voltage2);
        } else if ((rx_header.StdId & bms_topic_mask_TEMPERATURE_INFO) == bms_topic_filter_TEMPERATURE_INFO) {
            uint8_t offset = 0;
            bms_message_TEMPERATURES raw_temps;
            bms_deserialize_TEMPERATURES(&raw_temps, rx_data);
            switch (rx_header.StdId) {
                case bms_id_TEMPERATURES_CELLBOARD0:
                    ++cellboards_msgs.cellboard0;
                    offset = temperature_get_cellboard_offset(0);
                    break;
                case bms_id_TEMPERATURES_CELLBOARD1:
                    ++cellboards_msgs.cellboard1;
                    offset = temperature_get_cellboard_offset(1);
                    break;
                case bms_id_TEMPERATURES_CELLBOARD2:
                    ++cellboards_msgs.cellboard2;
                    offset = temperature_get_cellboard_offset(2);
                    break;
                case bms_id_TEMPERATURES_CELLBOARD3:
                    ++cellboards_msgs.cellboard3;
                    offset = temperature_get_cellboard_offset(3);
                    break;
                case bms_id_TEMPERATURES_CELLBOARD4:
                    ++cellboards_msgs.cellboard4;
                    offset = temperature_get_cellboard_offset(4);
                    break;
                case bms_id_TEMPERATURES_CELLBOARD5:
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
                raw_temps.temp3,
                raw_temps.temp4,
                raw_temps.temp5);
        } else if ((rx_header.StdId & bms_topic_mask_STATUS) == 0/*TODO: to be fixed in canlib */) {
            uint8_t index = 0;
            bms_message_BOARD_STATUS status;
            bms_deserialize_BOARD_STATUS(&status, rx_data);
            switch (rx_header.StdId) {
                case bms_id_BOARD_STATUS_CELLBOARD0:
                    ++cellboards_msgs.cellboard0;
                    index = 0;
                    break;
                case bms_id_BOARD_STATUS_CELLBOARD1:
                    ++cellboards_msgs.cellboard1;
                    index = 1;
                    break;
                case bms_id_BOARD_STATUS_CELLBOARD2:
                    ++cellboards_msgs.cellboard2;
                    index = 2;
                    break;
                case bms_id_BOARD_STATUS_CELLBOARD3:
                    ++cellboards_msgs.cellboard3;
                    index = 3;
                    break;
                case bms_id_BOARD_STATUS_CELLBOARD4:
                    ++cellboards_msgs.cellboard4;
                    index = 4;
                    break;
                case bms_id_BOARD_STATUS_CELLBOARD5:
                    ++cellboards_msgs.cellboard5;
                    index = 5;
                    break;

                default:
                    break;
            }
            bal.status[index] = status.balancing_status;

            if (index == 0)
                status.errors &= ~0b00001100;
            else if (index == 3)
                status.errors &= ~0b10000000;  //those adc are not working

            error_toggle_check(status.errors != 0, ERROR_CELLBOARD_INTERNAL, index);
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
        if (can_forward && rx_header.StdId != primary_id_HV_CAN_FORWARD) {
            uint8_t forward_data[8];
            tx_header.StdId = rx_header.StdId;
            tx_header.DLC = rx_header.DLC;
            *((uint64_t*)forward_data) = *((uint64_t*)rx_data);
            can_send(&BMS_CAN, forward_data, &tx_header);
            return;
        }

        error_reset(ERROR_CAN, 1);

        if (rx_header.StdId == primary_id_SET_TS_STATUS_DAS || rx_header.StdId == primary_id_SET_TS_STATUS_HANDCART) {
            primary_message_SET_TS_STATUS ts_status;
            primary_deserialize_SET_TS_STATUS(&ts_status, rx_data);

            switch (ts_status.ts_status_set) {
                case primary_Toggle_OFF:
                    fsm_trigger_event(bms.fsm, BMS_EV_TS_OFF);
                    break;
                case primary_Toggle_ON:
                    fsm_trigger_event(bms.fsm, BMS_EV_TS_ON);
                    break;
            }
        } else if (rx_header.StdId == primary_id_SET_CELL_BALANCING_STATUS) {
            primary_message_SET_CELL_BALANCING_STATUS balancing_status;
            primary_deserialize_SET_CELL_BALANCING_STATUS(&balancing_status, rx_data);

            if (balancing_status.set_balancing_status == primary_Toggle_ON) {
                fsm_trigger_event(bal.fsm, EV_BAL_START);
            } else if (balancing_status.set_balancing_status == primary_Toggle_OFF) {
                fsm_trigger_event(bal.fsm, EV_BAL_STOP);
            }
        } else if (rx_header.StdId == primary_id_HANDCART_STATUS) {
            primary_message_HANDCART_STATUS handcart_status;
            primary_deserialize_HANDCART_STATUS(&handcart_status, rx_data);
            bms.handcart_connected = handcart_status.connected;
        } else if (rx_header.StdId == primary_id_HV_CAN_FORWARD) {
            primary_message_HV_CAN_FORWARD hv_can_forward;
            primary_deserialize_HV_CAN_FORWARD(&hv_can_forward, rx_data);

            switch (hv_can_forward.can_forward_set) {
                case primary_Toggle_OFF:
                    can_forward = 0;
                    break;
                case primary_Toggle_ON:
                    can_bms_send(bms_id_FW_UPDATE);
                    can_forward = 1;
                    break;
            }
        } else if (rx_header.StdId == primary_id_BMS_HV_JMP_TO_BLT) {
            //JumpToBlt();
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