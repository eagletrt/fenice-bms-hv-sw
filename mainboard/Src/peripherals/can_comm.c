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

    tx_header.StdId = id;

    if (id == ID_HV_VOLTAGE) {
        tx_header.DLC = serialize_primary_HV_VOLTAGE(
            buffer, voltage_get_internal(), voltage_get_bus(), voltage_get_cell_max(NULL), voltage_get_cell_min(NULL));
    } else if (id == ID_HV_CURRENT) {
        tx_header.DLC =
            serialize_primary_HV_CURRENT(buffer, current_get_current(), current_get_current() * voltage_get_bus());
    } else if (id == ID_TS_STATUS) {
        primary_Ts_Status status = primary_Ts_Status_OFF;
        switch (fsm_get_state(bms.fsm)) {
            case BMS_IDLE:
                status = primary_Ts_Status_OFF;
                break;
            case BMS_TS_ON:
            case BMS_AIRN_CLOSE:
            case BMS_AIRN_STATUS:
            case BMS_PRECHARGE:
                status = primary_Ts_Status_PRECHARGE;
                break;
            case BMS_ON:
                status = primary_Ts_Status_ON;
                break;
            case BMS_FAULT:
                status = primary_Ts_Status_FATAL;
                break;
        }
        tx_header.DLC = serialize_primary_TS_STATUS(buffer, status);
    } else if (id == ID_HV_TEMP) {
        tx_header.DLC =
            serialize_primary_HV_TEMP(buffer, temperature_get_average(), temperature_get_max(), temperature_get_min());
    } else if (id == ID_HV_ERRORS) {
        primary_Hv_Errors errors = {0};
        tx_header.DLC            = serialize_primary_HV_ERRORS(buffer, errors, errors);
    } else if (id == ID_HV_CELL_BALANCING_STATUS) {
        primary_Balancing_Status bal_status;
        switch (fsm_get_state(bal.fsm)) {
            case BAL_OFF:
                bal_status = primary_Balancing_Status_OFF;
                break;
            case BAL_COMPUTE:
            case BAL_COOLDOWN:
            case BAL_DISCHARGE:
                bal_status = primary_Balancing_Status_ON;
                break;
            default:
                bal_status = primary_Balancing_Status_OFF;
                break;
        }
        tx_header.DLC = serialize_primary_HV_CELL_BALANCING_STATUS(buffer, bal_status);
    } else if (id == ID_HV_CELLS_TEMP) {
        temperature_t *temps = temperature_get_all();
        for (uint8_t i = 0; i < PACK_TEMP_COUNT; i += 6) {
            tx_header.DLC = serialize_primary_HV_CELLS_TEMP(
                buffer, i, temps[i], temps[i + 1], temps[i + 2], temps[i + 3], temps[i + 4], temps[i + 5], 0);
            can_send(&CAR_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        return HAL_OK;
    } else if (id == ID_HV_CELLS_VOLTAGE) {
        voltage_t *volts = voltage_get_cells();
        for (uint8_t i = 0; i < PACK_CELL_COUNT; i += 3) {
            tx_header.DLC = serialize_primary_HV_CELLS_VOLTAGE(buffer, i, volts[i], volts[i + 1], volts[i + 2]);
            can_send(&CAR_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        return HAL_OK;
    } else {
        return HAL_ERROR;
    }

    return can_send(&CAR_CAN, buffer, &tx_header);
}

HAL_StatusTypeDef can_bms_send(uint16_t id) {
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    tx_header.StdId = id;

    if (id == ID_BALANCING) {
        uint8_t *distr = bms_get_cellboard_distribution();
        register uint16_t i;
        for (i = 0; i < CELLBOARD_COUNT; ++i) {
            tx_header.DLC = serialize_bms_BALANCING(buffer, distr[i], bal.cells[i]);
            can_send(&BMS_CAN, buffer, &tx_header);
        }
        return HAL_OK;  //TODO: ugly
    } else if (id == ID_FW_UPDATE) {
        tx_header.DLC = serialize_bms_FW_UPDATE(buffer, 0);  //TODO: set board_index
        can_send(&BMS_CAN, buffer, &tx_header);
    }

    return can_send(&BMS_CAN, buffer, &tx_header);
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
        error_reset(ERROR_CAN, 1);
        if ((rx_header.StdId & TOPIC_VOLTAGE_INFO_MASK) == TOPIC_VOLTAGE_INFO_FILTER) {
            uint8_t offset = 0;
            bms_VOLTAGES voltages;
            deserialize_bms_VOLTAGES(rx_data, &voltages);
            switch (rx_header.StdId) {
                case ID_VOLTAGES_0:
                    ++cellboards_msgs.cellboard0;
                    offset = voltage_get_cellboard_offset(0);
                    break;
                case ID_VOLTAGES_1:
                    ++cellboards_msgs.cellboard1;
                    offset = voltage_get_cellboard_offset(1);
                    break;
                case ID_VOLTAGES_2:
                    ++cellboards_msgs.cellboard2;
                    offset = voltage_get_cellboard_offset(2);
                    break;
                case ID_VOLTAGES_3:
                    ++cellboards_msgs.cellboard3;
                    offset = voltage_get_cellboard_offset(3);
                    break;
                case ID_VOLTAGES_4:
                    ++cellboards_msgs.cellboard4;
                    offset = voltage_get_cellboard_offset(4);
                    break;
                case ID_VOLTAGES_5:
                    ++cellboards_msgs.cellboard5;
                    offset = voltage_get_cellboard_offset(5);
                    break;
                default:
                    break;
            }
            voltage_set_cells(voltages.start_index + offset, voltages.voltage0, voltages.voltage1, voltages.voltage2);
        } else if ((rx_header.StdId & TOPIC_TEMPERATURE_INFO_MASK) == TOPIC_TEMPERATURE_INFO_FILTER) {
            uint8_t offset = 0;
            bms_TEMPERATURES temperatures;
            deserialize_bms_TEMPERATURES(rx_data, &temperatures);
            switch (rx_header.StdId) {
                case ID_TEMPERATURES_0:
                    ++cellboards_msgs.cellboard0;
                    offset = temperature_get_cellboard_offset(0);
                    break;
                case ID_TEMPERATURES_1:
                    ++cellboards_msgs.cellboard1;
                    offset = temperature_get_cellboard_offset(1);
                    break;
                case ID_TEMPERATURES_2:
                    ++cellboards_msgs.cellboard2;
                    offset = temperature_get_cellboard_offset(2);
                    break;
                case ID_TEMPERATURES_3:
                    ++cellboards_msgs.cellboard3;
                    offset = temperature_get_cellboard_offset(3);
                    break;
                case ID_TEMPERATURES_4:
                    ++cellboards_msgs.cellboard4;
                    offset = temperature_get_cellboard_offset(4);
                    break;
                case ID_TEMPERATURES_5:
                    ++cellboards_msgs.cellboard5;
                    offset = temperature_get_cellboard_offset(5);
                    break;
                default:
                    break;
            }
            temperature_set_cells(
                temperatures.start_index + offset,
                temperatures.temp0,
                temperatures.temp1,
                temperatures.temp2,
                temperatures.temp3,
                temperatures.temp4,
                temperatures.temp5);
        } else if ((rx_header.StdId & TOPIC_STATUS_MASK) == TOPIC_STATUS_FILTER) {
            uint8_t index = 0;
            bms_BOARD_STATUS status;
            deserialize_bms_BOARD_STATUS(rx_data, &status);
            switch (rx_header.StdId) {
                case ID_BOARD_STATUS_0:
                    ++cellboards_msgs.cellboard0;
                    index = 0;
                    break;
                case ID_BOARD_STATUS_1:
                    ++cellboards_msgs.cellboard1;
                    index = 1;
                    break;
                case ID_BOARD_STATUS_2:
                    ++cellboards_msgs.cellboard2;
                    index = 2;
                    break;
                case ID_BOARD_STATUS_3:
                    ++cellboards_msgs.cellboard3;
                    index = 3;
                    break;
                case ID_BOARD_STATUS_4:
                    ++cellboards_msgs.cellboard4;
                    index = 4;
                    break;
                case ID_BOARD_STATUS_5:
                    ++cellboards_msgs.cellboard5;
                    index = 5;
                    break;

                default:
                    break;
            }
            bal.status[index] = status.balancing_status;

            if (index == 0)
                *(status.errors) &= ~0b00001100;
            else if (index == 3)
                *(status.errors) &= ~0b10000000;  //those adc are not working

            error_toggle_check(*(status.errors) != 0, ERROR_CELLBOARD_INTERNAL, index);
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
        error_reset(ERROR_CAN, 1);

        if (rx_header.StdId == ID_SET_TS_STATUS) {
            primary_SET_TS_STATUS ts_status;
            deserialize_primary_SET_TS_STATUS(rx_data, &ts_status);

            switch (ts_status.ts_status_set) {
                case primary_Ts_Status_Set_OFF:
                    fsm_trigger_event(bms.fsm, BMS_EV_TS_OFF);
                    break;
                case primary_Ts_Status_Set_ON:
                    fsm_trigger_event(bms.fsm, BMS_EV_TS_ON);
                    break;
            }
        } else if (rx_header.StdId == ID_SET_CELL_BALANCING_STATUS) {
            primary_SET_CELL_BALANCING_STATUS balancing_status;
            deserialize_primary_SET_CELL_BALANCING_STATUS(rx_data, &balancing_status);

            if (balancing_status.set_balancing_status == primary_Set_Balancing_Status_ON) {
                fsm_trigger_event(bal.fsm, EV_BAL_START);
            } else if (balancing_status.set_balancing_status == primary_Set_Balancing_Status_OFF) {
                fsm_trigger_event(bal.fsm, EV_BAL_STOP);
            }
        } else if (rx_header.StdId == ID_HANDCART_STATUS) {
            primary_HANDCART_STATUS handcart_status;
            deserialize_primary_HANDCART_STATUS(rx_data, &handcart_status);
            bms.handcart_connected = handcart_status.connected;
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