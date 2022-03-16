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
#include "pack/voltage.h"
#include "pack/temperature.h"

CAN_TxHeaderTypeDef tx_header;

void can_tx_header_init() {
    tx_header.ExtId = 0;
    tx_header.IDE   = CAN_ID_STD;
    tx_header.RTR   = CAN_RTR_DATA;
}

void can_bms_init(){
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
    filter.FilterBank           = 0;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;
    filter.FilterActivation     = ENABLE;
    filter.SlaveStartFilterBank = CAN_SLAVE_START_FILTER_BANK;

    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);
    HAL_CAN_ActivateNotification(&BMS_CAN, CAN_IT_ERROR | CAN_IT_RX_FIFO0_MSG_PENDING );
    HAL_CAN_Start(&BMS_CAN);

    can_tx_header_init();
}

void can_car_init(){
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
    filter.FilterBank           = 14;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;
    filter.FilterActivation     = ENABLE;
    filter.SlaveStartFilterBank = CAN_SLAVE_START_FILTER_BANK;

    HAL_CAN_ConfigFilter(&CAR_CAN, &filter);
    HAL_CAN_ActivateNotification(&CAR_CAN, CAN_IT_ERROR | CAN_IT_RX_FIFO1_MSG_PENDING );
    HAL_CAN_Start(&CAR_CAN);

    can_tx_header_init();
}

HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    CAN_WAIT(hcan);

    uint32_t mailbox = 0;

    if(!HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX0))
        mailbox = CAN_TX_MAILBOX0;
    else if(!HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX1))
        mailbox = CAN_TX_MAILBOX1;
    else if(!HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX2))
        mailbox = CAN_TX_MAILBOX2;


    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, &mailbox);
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
    if (id == ID_HV_VOLTAGE) {
        tx_header.DLC = serialize_primary_HV_VOLTAGE(
            buffer, voltage_get_internal(), voltage_get_bus(), voltage_get_cell_max(), voltage_get_cell_min());
    } else if (id == ID_HV_CURRENT) {
        tx_header.DLC = serialize_primary_HV_CURRENT(buffer, current_get_current(), current_get_current() * voltage_get_bus());
    } else if (id == ID_TS_STATUS) {
        primary_Ts_Status status = primary_Ts_Status_OFF;
        switch(fsm_get_state(bms.fsm)) {
            case BMS_IDLE:
                status = primary_Ts_Status_OFF;
                break;
            case BMS_TS_ON:
            case BMS_AIRN_CLOSE:
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
        tx_header.DLC = serialize_primary_HV_TEMP(buffer, temperature_get_average(), temperature_get_max(), temperature_get_min());
    } else if (id == ID_HV_ERRORS) {
        primary_Hv_Errors errors = {0};
        tx_header.DLC = serialize_primary_HV_ERRORS(buffer, errors, errors);
    } else {
        return HAL_ERROR;
    }

    tx_header.StdId = id;
    
    return can_send(&CAR_CAN, buffer, &tx_header);
}

HAL_StatusTypeDef can_bms_send(uint16_t id) {
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    tx_header.StdId = id;
    
    if(id == ID_BALANCING) {
        register uint16_t i;
        for(i=0; i<LTC6813_COUNT; ++i){
            tx_header.DLC = serialize_bms_BALANCING(buffer, i, bal.cells[i]);
            can_send(&BMS_CAN, buffer, &tx_header);
        }
        return HAL_OK; //TODO: ugly
    }
    else if(id == ID_FW_UPDATE) {
        tx_header.DLC = serialize_bms_FW_UPDATE(buffer, 0); //TODO: set board_index
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
        if((rx_header.StdId & TOPIC_VOLTAGE_INFO_MASK) == TOPIC_VOLTAGE_INFO_FILTER) {
            uint8_t offset = 0;
            bms_VOLTAGES voltages;
            deserialize_bms_VOLTAGES(rx_data, &voltages);
            switch (rx_header.StdId)
            {
            case ID_VOLTAGES_0:
                offset = VOLTAGE_CELLBOARD_0;
                break;
            case ID_VOLTAGES_1:
                offset = VOLTAGE_CELLBOARD_1;
                break;
            case ID_VOLTAGES_2:
                offset = VOLTAGE_CELLBOARD_2;
                break;
            case ID_VOLTAGES_3:
                offset = VOLTAGE_CELLBOARD_3;
                break;
            case ID_VOLTAGES_4:
                offset = VOLTAGE_CELLBOARD_4;
                break;
            case ID_VOLTAGES_5:
                offset = VOLTAGE_CELLBOARD_5;
                break;
            default:
                break;
            }
            voltage_set_cells(voltages.start_index + offset, voltages.voltage0, voltages.voltage1, voltages.voltage2);
        }
        else if ((rx_header.StdId & TOPIC_TEMPERATURE_INFO_MASK) == TOPIC_TEMPERATURE_INFO_FILTER) {
            uint8_t offset = 0;
            bms_TEMPERATURES temperatures;
            deserialize_bms_TEMPERATURES(rx_data, &temperatures);
            switch (rx_header.StdId)
            {
            case ID_TEMPERATURES_0:
                offset = TEMP_CELLBOARD_0;
                break;
            case ID_TEMPERATURES_1:
                offset = TEMP_CELLBOARD_1;
                break;
            case ID_TEMPERATURES_2:
                offset = TEMP_CELLBOARD_2;
                break;
            case ID_TEMPERATURES_3:
                offset = TEMP_CELLBOARD_3;
                break;
            case ID_TEMPERATURES_4:
                offset = TEMP_CELLBOARD_4;
                break;
            case ID_TEMPERATURES_5:
                offset = TEMP_CELLBOARD_5;
                break;
            default:
                break;
            }
            temperature_set_cells(temperatures.start_index + offset, temperatures.temp0, temperatures.temp1, temperatures.temp2,
                                                                    temperatures.temp3, temperatures.temp4, temperatures.temp5);
        }
        else if ((rx_header.StdId & TOPIC_STATUS_MASK) == TOPIC_STATUS_FILTER) {
            uint8_t index = 0;
            bms_BOARD_STATUS status;
            deserialize_bms_BOARD_STATUS(rx_data, &status);
            switch (rx_header.StdId)
            {
            case ID_BOARD_STATUS_0:
                index = 0;
                break;
            case ID_BOARD_STATUS_1:
                index = 1;
                break;
            case ID_BOARD_STATUS_2:
                index = 2;
                break;
            case ID_BOARD_STATUS_3:
                index = 3;
                break;
            case ID_BOARD_STATUS_4:
                index = 4;
                break;
            case ID_BOARD_STATUS_5:
                index = 5;
                break;
            
            default:
                break;
            }
            bal.status[index] = status.balancing_status;
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
        }
        else if (rx_header.StdId == ID_SET_CELL_BALANCING_STATUS) {
            primary_SET_CELL_BALANCING_STATUS balancing_status;
            deserialize_primary_SET_CELL_BALANCING_STATUS(rx_data, &balancing_status);

            if (balancing_status.set_balancing_status == primary_Set_Balancing_Status_ON) {
                fsm_trigger_event(bal.fsm, EV_BAL_START);
            }
            else if (balancing_status.set_balancing_status == primary_Set_Balancing_Status_OFF) {
                fsm_trigger_event(bal.fsm, EV_BAL_STOP);
            }
        }
        else if (rx_header.StdId == ID_HANDCART_STATUS) {

        }
    }
}