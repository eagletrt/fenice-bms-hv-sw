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

CAN_TxHeaderTypeDef tx_header;

void can_init() {
    tx_header.ExtId = 0;
    tx_header.IDE   = CAN_ID_STD;
    tx_header.RTR   = CAN_RTR_DATA;
}

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
    filter.FilterBank           = 0;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;
    filter.FilterActivation     = ENABLE;

    HAL_CAN_ConfigFilter(&CAR_CAN, &filter);
    HAL_CAN_ActivateNotification(&CAR_CAN, CAN_IT_ERROR | CAN_IT_RX_FIFO1_MSG_PENDING );
    HAL_CAN_Start(&CAR_CAN);

    can_tx_header_init();
}

HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t buffer, CAN_TxHeaderTypeDef *header) {

    uint32_t mailbox = CAN_TX_MAILBOX0;

    if(HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX1))
        mailbox = CAN_TX_MAILBOX1;
    else if(HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX2))
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
        tx_header.DLC = serialize_Primary_HV_VOLTAGE(
            buffer, voltage_get_internal(), voltage_get_bus(), voltage_get_cell_max(), voltage_get_cell_min());
    } else if (id == ID_HV_CURRENT) {
        tx_header.DLC = serialize_Primary_HV_CURRENT(buffer, current_get_current(), current_get_current() * voltage_get_bus());
    } else if (id == ID_TS_STATUS) {
        tx_header.DLC = serialize_Primary_TS_STATUS(buffer, Primary_Ts_Status_ON);
    } else {
        return HAL_ERROR;
    }

    tx_header.StdId = id;
    
    return can_send(&CAR_CAN, buffer, &tx_header);
}

HAL_StatusTypeDef can_bms_send(uint16_t id) {
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    tx_header.StdId = id;
    
    if(id == ID_MASTER_SYNC){
        tx_header.DLC = serialize_bms_MASTER_SYNC(buffer, HAL_GetTick());
    } else if(id == ID_BALANCING) {
        register int i;
        bms_balancing_cells bal_cells = bms_balancing_cells_default;
        for(i=0; i<bal.cells_count; ++i) {
            flipBit(bal_cells, i); //sets to 1
        }
        tx_header.DLC = serialize_bms_BALANCING(buffer, 0, NULL);
    }

    return can_send(&BMS_CAN, buffer, &tx_header);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    //void HAL_FDCAN_RxFifo0Callback(CAN_HandleTypeDef *hcan, uint32_t RxFifo0ITs) {
    //if (hfdcan->Instance == FDCAN1 && RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) {
    uint8_t rx_data[8] = {'\0'};
    CAN_RxHeaderTypeDef rx_header;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        //if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        error_set(ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == BMS_CAN.Instance) {
        if (rx_header.StdId == ID_VOLTAGES_0) {
            bms_VOLTAGES_0 voltages_0;
            deserialize_bms_VOLTAGES_0(rx_data, &voltages_0);
        }
        if (rx_header.StdId == ID_VOLTAGES_1) {
            bms_VOLTAGES_1 voltages_1;
            deserialize_bms_VOLTAGES_1(rx_data, &voltages_1);
        }
        if (rx_header.StdId == ID_VOLTAGES_2) {
            bms_VOLTAGES_2 voltages_2;
            deserialize_bms_VOLTAGES_2(rx_data, &voltages_2);
        }
        if (rx_header.StdId == ID_VOLTAGES_3) {
            bms_VOLTAGES_3 voltages_3;
            deserialize_bms_VOLTAGES_3(rx_data, &voltages_3);
        }
        if (rx_header.StdId == ID_VOLTAGES_4) {
            bms_VOLTAGES_4 voltages_4;
            deserialize_bms_VOLTAGES_4(rx_data, &voltages_4);
        }
        if (rx_header.StdId == ID_VOLTAGES_5) {
            bms_VOLTAGES_5 voltages_5;
            deserialize_bms_VOLTAGES_5(rx_data, &voltages_5);
        }
        if (rx_header.StdId == ID_TEMP_STATS) {
            bms_TEMP_STATS temp_stats;
            deserialize_bms_TEMP_STATS(rx_data, &temp_stats);
        }
        if (rx_header.StdId == ID_BOARD_STATUS) {
            bms_BOARD_STATUS board_status;
            deserialize_bms_BOARD_STATUS(rx_data, &board_status);
        }
    }
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    //void HAL_FDCAN_RxFifo0Callback(CAN_HandleTypeDef *hcan, uint32_t RxFifo0ITs) {
    //if (hfdcan->Instance == FDCAN1 && RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) {
    uint8_t rx_data[8] = {'\0'};
    CAN_RxHeaderTypeDef rx_header;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rx_header, rx_data) != HAL_OK) {
        //if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        error_set(ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == CAR_CAN.Instance) {
        error_reset(ERROR_CAN, 1);

        //size_t len = rx_header.DLC >> 16;
        //uint8_t len = rx_header.DLC;

        if (rx_header.StdId == ID_SET_TS_STATUS) {
            Primary_SET_TS_STATUS ts_status;
            deserialize_Primary_SET_TS_STATUS(rx_data, &ts_status);

            switch (ts_status.ts_status_set) {
                case Primary_Ts_Status_Set_OFF:
                    fsm_trigger_event(bms.fsm, BMS_EV_TS_OFF);
                    break;
                case Primary_Ts_Status_Set_ON:
                    fsm_trigger_event(bms.fsm, BMS_EV_TS_ON);
                    break;
            }
        }
        if (rx_header.StdId == ID_SET_CHG_STATUS) {
            Primary_SET_CHG_STATUS chg_status;
            deserialize_Primary_SET_CHG_STATUS(rx_data, &chg_status);

            switch (chg_status.status) {
                case Primary_Status_CHG_OFF:
                    break;
                case Primary_Status_CHG_TC:
                    break;
                case Primary_Status_CHG_CC:
                    break;
                case Primary_Status_CHG_CV:
                    break;
            }
        }
    }
}