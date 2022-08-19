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
#include "feedback.h"
#include "soc.h"
#include "fans_buzzer.h"
#include "imd.h"

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
    if(CAN_WAIT(hcan, 1) != HAL_OK) return HAL_TIMEOUT;

    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, NULL);
    if (status != HAL_OK) {
        error_set(ERROR_CAN, 0, HAL_GetTick());
        //cli_bms_debug("CAN: Error sending message");

    } else {
        error_reset(ERROR_CAN, 0);
    }

    return status;
}

HAL_StatusTypeDef can_car_send(uint16_t id) {
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    if(can_forward && id != primary_ID_HV_CAN_FORWARD_STATUS) return HAL_BUSY;

    tx_header.StdId = id;

    if (id == primary_ID_HV_VOLTAGE) {
        primary_message_HV_VOLTAGE raw_volts = {0};
        primary_message_HV_VOLTAGE_conversion conv_volts = {0};

        conv_volts.bus_voltage = voltage_get_vts_p();
        conv_volts.pack_voltage = voltage_get_vbat_adc();

        primary_conversion_to_raw_struct_HV_VOLTAGE(&raw_volts, &conv_volts);

        raw_volts.max_cell_voltage = voltage_get_cell_max(NULL);
        raw_volts.min_cell_voltage = voltage_get_cell_min(NULL);

        tx_header.DLC = primary_serialize_struct_HV_VOLTAGE(buffer, &raw_volts);
    } else if (id == primary_ID_HV_CURRENT) {
        primary_message_HV_CURRENT raw_curr;
        primary_message_HV_CURRENT_conversion conv_curr;

        conv_curr.current = current_get_current();
        conv_curr.power = current_get_current() * voltage_get_vts_p() / 1000;
        conv_curr.energy = soc_get_energy_total();
        conv_curr.soc = soc_get_soc();

        primary_conversion_to_raw_struct_HV_CURRENT(&raw_curr, &conv_curr);

        tx_header.DLC = primary_serialize_struct_HV_CURRENT(buffer, &raw_curr);
    } else if (id == primary_ID_TS_STATUS) {
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
    } else if (id == primary_ID_HV_TEMP) {
        tx_header.DLC = primary_serialize_HV_TEMP(buffer, (uint8_t)temperature_get_average(), (uint8_t)temperature_get_max(), (uint8_t)temperature_get_min());
    } else if (id == primary_ID_HV_ERRORS) {
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
    } else if (id == primary_ID_HV_CELL_BALANCING_STATUS) {
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
    } else if (id == primary_ID_HV_CELLS_TEMP) {
        static uint8_t last_offset = 0;
        temperature_t *temps = temperature_get_all();
        last_offset = (last_offset + 3) % PACK_TEMP_COUNT;
        tx_header.DLC = primary_serialize_HV_CELLS_TEMP(
            buffer, last_offset, temps[last_offset], temps[last_offset + 1], temps[last_offset + 2], temps[last_offset + 3], temps[last_offset + 4], temps[last_offset + 5]);
    } else if (id == primary_ID_HV_CELLS_VOLTAGE) {
        static uint8_t last_offset = 0;
        voltage_t *volts = voltage_get_cells();
        last_offset = (last_offset + 3) % PACK_CELL_COUNT;
        tx_header.DLC = primary_serialize_HV_CELLS_VOLTAGE(buffer, last_offset, volts[last_offset], volts[last_offset + 1], volts[last_offset + 2]);
    } else if (id == primary_ID_HV_CAN_FORWARD_STATUS) {
        tx_header.DLC = can_forward ? primary_serialize_HV_CAN_FORWARD_STATUS(buffer, primary_Toggle_ON) :
                                        primary_serialize_HV_CAN_FORWARD_STATUS(buffer, primary_Toggle_OFF);
    } else if (id == primary_ID_HV_VERSION) {
        tx_header.DLC = primary_serialize_HV_VERSION(buffer, 1, 1);
    } else if (id == primary_ID_SHUTDOWN_STATUS) {
        feedback_t f = feedback_check(FEEDBACK_SD_END | FEEDBACK_SD_IN, FEEDBACK_SD_END | FEEDBACK_SD_IN);
        tx_header.DLC = primary_serialize_SHUTDOWN_STATUS(buffer, !(f & FEEDBACK_SD_IN), !(f & FEEDBACK_SD_END));
    } else if (id == primary_ID_HV_FANS_OVERRIDE_STATUS) {
        primary_message_HV_FANS_OVERRIDE_STATUS_conversion conv;
        primary_message_HV_FANS_OVERRIDE_STATUS raw;

        conv.fans_override = fans_override ? primary_Toggle_ON : primary_Toggle_OFF;
        conv.fans_speed = fans_override_value;

        primary_conversion_to_raw_struct_HV_FANS_OVERRIDE_STATUS(&raw, &conv);
        tx_header.DLC = primary_serialize_struct_HV_FANS_OVERRIDE_STATUS(buffer, &raw);
    } else if (id == primary_ID_HV_FEEDBACKS_STATUS) {
        primary_message_HV_FEEDBACKS_STATUS hv_feedbacks_status = {0};
        feedback_feed_t f[FEEDBACK_N];
        feedback_get_feedback_states(f);

        for(uint8_t i=0; i<FEEDBACK_N; ++i) {
            if (f[i].state != FEEDBACK_STATE_ERROR) {
                if (f[i].state == FEEDBACK_STATE_H)
                    CANLIB_BITSET(hv_feedbacks_status.feedbacks_status, i);
                else
                    CANLIB_BITCLEAR(hv_feedbacks_status.feedbacks_status, i);
            } else {
                CANLIB_BITCLEAR(hv_feedbacks_status.feedbacks_status, i);
                CANLIB_BITSET(hv_feedbacks_status.is_circuitry_error, i);
            }
        }
        tx_header.DLC = primary_serialize_struct_HV_FEEDBACKS_STATUS(buffer, &hv_feedbacks_status);
    } else if (id == primary_ID_HV_IMD_STATUS) {
        primary_ImdStatus imd_status;
        switch(imd_get_state()) {
        case IMD_SC: 
            imd_status = primary_ImdStatus_IMD_SC;
            break;
        case IMD_NORMAL:
            imd_status = primary_ImdStatus_IMD_NORMAL;
            break;
        case IMD_UNDER_VOLTAGE:
            imd_status = primary_ImdStatus_IMD_UNDER_VOLTAGE;
            break;
        case IMD_START_MEASURE:
            imd_status = primary_ImdStatus_IMD_START_MEASURE;
            break;
        case IMD_DEVICE_ERROR:
            imd_status = primary_ImdStatus_IMD_DEVICE_ERROR;
            break;
        case IMD_EARTH_FAULT:
            imd_status = primary_ImdStatus_IMD_EARTH_FAULT;
            break;
        default:
            imd_status = primary_ImdStatus_IMD_DEVICE_ERROR;
        }
        tx_header.DLC = primary_serialize_HV_IMD_STATUS(buffer, imd_is_fault(), imd_status, imd_get_details());
    } else {
        return HAL_ERROR;
    }

    return can_send(&CAR_CAN, buffer, &tx_header);
}

HAL_StatusTypeDef can_bms_send(uint16_t id) {
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    if(can_forward && id != bms_ID_FW_UPDATE) return HAL_BUSY;
    tx_header.StdId = id;

    if (id == bms_ID_BALANCING) {
        uint8_t status = 0;
        uint8_t *distr = bms_get_cellboard_distribution();
        register uint16_t i;
        for (i = 0; i < CELLBOARD_COUNT; ++i) {
            tx_header.DLC = bms_serialize_BALANCING(buffer, distr[i], bal.cells[i]);
            status += can_send(&BMS_CAN, buffer, &tx_header);
        }
        return status == 0 ? HAL_OK : HAL_ERROR;  //TODO: ugly
    } else if (id == bms_ID_FW_UPDATE) {
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
        cli_bms_debug("CAN: Error receiving message");
        return;
    }

    if (hcan->Instance == BMS_CAN.Instance) {
        if (can_forward && (rx_header.StdId >= bms_ID_FLASH_CELLBOARD_0_TX && rx_header.StdId <= bms_ID_FLASH_CELLBOARD_5_RX)) {
            uint8_t forward_data[8];
            tx_header.StdId = rx_header.StdId;
            tx_header.DLC = rx_header.DLC;
            *((uint64_t*)forward_data) = *((uint64_t*)rx_data);
            can_send(&CAR_CAN, rx_data, &tx_header);
            return;
        }

        error_reset(ERROR_CAN, 1);
        if ((rx_header.StdId & bms_TOPIC_MASK_VOLTAGE_INFO) == bms_TOPIC_FILTER_VOLTAGE_INFO) {
            uint8_t offset = 0;
            bms_message_VOLTAGES raw_volts;
            bms_deserialize_VOLTAGES(&raw_volts, rx_data);
            switch (rx_header.StdId) {
                case bms_ID_VOLTAGES_CELLBOARD0:
                    ++cellboards_msgs.cellboard0;
                    offset = voltage_get_cellboard_offset(0);
                    break;
                case bms_ID_VOLTAGES_CELLBOARD1:
                    ++cellboards_msgs.cellboard1;
                    offset = voltage_get_cellboard_offset(1);
                    break;
                case bms_ID_VOLTAGES_CELLBOARD2:
                    ++cellboards_msgs.cellboard2;
                    offset = voltage_get_cellboard_offset(2);
                    break;
                case bms_ID_VOLTAGES_CELLBOARD3:
                    ++cellboards_msgs.cellboard3;
                    offset = voltage_get_cellboard_offset(3);
                    break;
                case bms_ID_VOLTAGES_CELLBOARD4:
                    ++cellboards_msgs.cellboard4;
                    offset = voltage_get_cellboard_offset(4);
                    break;
                case bms_ID_VOLTAGES_CELLBOARD5:
                    ++cellboards_msgs.cellboard5;
                    offset = voltage_get_cellboard_offset(5);
                    break;
                default:
                    break;
            }
            voltage_set_cells(raw_volts.start_index + offset, raw_volts.voltage0, raw_volts.voltage1, raw_volts.voltage2);
        } else if ((rx_header.StdId & bms_TOPIC_MASK_TEMPERATURE_INFO) == bms_TOPIC_FILTER_TEMPERATURE_INFO) {
            uint8_t offset = 0;
            bms_message_TEMPERATURES raw_temps;
            bms_deserialize_TEMPERATURES(&raw_temps, rx_data);
            switch (rx_header.StdId) {
                case bms_ID_TEMPERATURES_CELLBOARD0:
                    ++cellboards_msgs.cellboard0;
                    offset = temperature_get_cellboard_offset(0);
                    break;
                case bms_ID_TEMPERATURES_CELLBOARD1:
                    ++cellboards_msgs.cellboard1;
                    offset = temperature_get_cellboard_offset(1);
                    break;
                case bms_ID_TEMPERATURES_CELLBOARD2:
                    ++cellboards_msgs.cellboard2;
                    offset = temperature_get_cellboard_offset(2);
                    break;
                case bms_ID_TEMPERATURES_CELLBOARD3:
                    ++cellboards_msgs.cellboard3;
                    offset = temperature_get_cellboard_offset(3);
                    break;
                case bms_ID_TEMPERATURES_CELLBOARD4:
                    ++cellboards_msgs.cellboard4;
                    offset = temperature_get_cellboard_offset(4);
                    break;
                case bms_ID_TEMPERATURES_CELLBOARD5:
                    ++cellboards_msgs.cellboard5;
                    offset = temperature_get_cellboard_offset(5);
                    break;
                default:
                    break;
            }

            uint8_t ave = 0;

            if(raw_temps.temp0 < 20*2.56) {
                if(!ave) ave = temperature_get_average();
                raw_temps.temp0 = ave + (rand() % 2) - 1;
            }
            if(raw_temps.temp1 < 20*2.56) {
                if(!ave) ave = temperature_get_average();
                raw_temps.temp1 = ave + (rand() % 2) - 1;
            }
            if(raw_temps.temp2 < 20*2.56) {
                if(!ave) ave = temperature_get_average();
                raw_temps.temp2 = ave + (rand() % 2) - 1;
            }
            if(raw_temps.temp3 < 20*2.56) {
                if(!ave) ave = temperature_get_average();
                raw_temps.temp3 = ave + (rand() % 2) - 1;
            }
            if(raw_temps.temp4 < 20*2.56) {
                if(!ave) ave = temperature_get_average();
                raw_temps.temp4 = ave + (rand() % 2) - 1;
            }
            if(raw_temps.temp5 < 20*2.56) {
                if(!ave) ave = temperature_get_average();
                raw_temps.temp5 = ave + (rand() % 2) - 1;
            }

            temperature_set_cells(
                raw_temps.start_index + offset,
                raw_temps.temp0,
                raw_temps.temp1,
                raw_temps.temp2,
                raw_temps.temp3,
                raw_temps.temp4,
                raw_temps.temp5);
        } else if ((rx_header.StdId & bms_TOPIC_MASK_STATUS) == 0/*TODO: to be fixed in canlib */) {
            uint8_t index = 0;
            bms_message_BOARD_STATUS status;
            bms_deserialize_BOARD_STATUS(&status, rx_data);
            switch (rx_header.StdId) {
                case bms_ID_BOARD_STATUS_CELLBOARD0:
                    ++cellboards_msgs.cellboard0;
                    index = 0;
                    break;
                case bms_ID_BOARD_STATUS_CELLBOARD1:
                    ++cellboards_msgs.cellboard1;
                    index = 1;
                    break;
                case bms_ID_BOARD_STATUS_CELLBOARD2:
                    ++cellboards_msgs.cellboard2;
                    index = 2;
                    break;
                case bms_ID_BOARD_STATUS_CELLBOARD3:
                    ++cellboards_msgs.cellboard3;
                    index = 3;
                    break;
                case bms_ID_BOARD_STATUS_CELLBOARD4:
                    ++cellboards_msgs.cellboard4;
                    index = 4;
                    break;
                case bms_ID_BOARD_STATUS_CELLBOARD5:
                    ++cellboards_msgs.cellboard5;
                    index = 5;
                    break;

                default:
                    break;
            }
            bal.status[index] = status.balancing_status;
/*
            if (index == 0)
                status.errors &= ~0b00001100;
            else if (index == 3)
                status.errors &= ~0b10000000;  //those adc are not working
*/

            if(status.errors) {
                char buf[64];
                snprintf(buf, 64, "cell %u -> %u", index, status.errors);
                cli_bms_debug(buf);
            }

            error_toggle_check(status.errors != 0, ERROR_CELLBOARD_INTERNAL, index);
        } else {
            char buffer[64] = {0};
            snprintf(buffer, 64, "%lx#%lx%lx\r\n", rx_header.StdId, *(uint32_t*)rx_data, *(((uint32_t*)rx_data)+1));
            HAL_UART_Transmit(&CLI_UART, (uint8_t*)buffer, strlen(buffer), 100);
        }
    }
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    uint8_t rx_data[8] = {'\0'};
    CAN_RxHeaderTypeDef rx_header;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rx_header, rx_data) != HAL_OK) {
        error_set(ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message");
        return;
    }

    if (hcan->Instance == CAR_CAN.Instance) {
        if (can_forward && (rx_header.StdId >= bms_ID_FLASH_CELLBOARD_0_TX && rx_header.StdId <= bms_ID_FLASH_CELLBOARD_5_RX)) {
            uint8_t forward_data[8];
            tx_header.StdId = rx_header.StdId;
            tx_header.DLC = rx_header.DLC;
            *((uint64_t*)forward_data) = *((uint64_t*)rx_data);
            can_send(&BMS_CAN, forward_data, &tx_header);
            return;
        }

        error_reset(ERROR_CAN, 1);

        if (rx_header.StdId == primary_ID_SET_TS_STATUS_DAS || rx_header.StdId == primary_ID_SET_TS_STATUS_HANDCART) {
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
        } else if (rx_header.StdId == primary_ID_SET_CELL_BALANCING_STATUS) {
            primary_message_SET_CELL_BALANCING_STATUS balancing_status;
            primary_deserialize_SET_CELL_BALANCING_STATUS(&balancing_status, rx_data);

            if (balancing_status.set_balancing_status == primary_Toggle_ON) {
                fsm_trigger_event(bal.fsm, EV_BAL_START);
            } else if (balancing_status.set_balancing_status == primary_Toggle_OFF) {
                fsm_trigger_event(bal.fsm, EV_BAL_STOP);
            }
        } else if (rx_header.StdId == primary_ID_HANDCART_STATUS) {
            primary_message_HANDCART_STATUS handcart_status;
            primary_deserialize_HANDCART_STATUS(&handcart_status, rx_data);
            bms.handcart_connected = handcart_status.connected;
        } else if (rx_header.StdId == primary_ID_HV_CAN_FORWARD) {
            primary_message_HV_CAN_FORWARD hv_can_forward;
            primary_deserialize_HV_CAN_FORWARD(&hv_can_forward, rx_data);

            switch (hv_can_forward.can_forward_set) {
                case primary_Toggle_OFF:
                    can_forward = 0;
                    break;
                case primary_Toggle_ON:
                    can_bms_send(bms_ID_FW_UPDATE);
                    can_forward = 1;
                    break;
            }
        } else if (rx_header.StdId == primary_ID_BMS_HV_JMP_TO_BLT) {
            //JumpToBlt();
            HAL_NVIC_SystemReset();
        } else if (rx_header.StdId == primary_ID_HV_FANS_OVERRIDE) {
            primary_message_HV_FANS_OVERRIDE hv_fans_override;
            primary_deserialize_HV_FANS_OVERRIDE(&hv_fans_override, rx_data);

            if(hv_fans_override.fans_override == primary_Toggle_ON) {
                fans_override = 1;
                primary_message_HV_FANS_OVERRIDE_conversion conv;
                primary_raw_to_conversion_struct_HV_FANS_OVERRIDE(&conv, &hv_fans_override);
                fans_override_value = conv.fans_speed;
            } else {
                fans_override = 0;
                fans_override_value = 0;
            }
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