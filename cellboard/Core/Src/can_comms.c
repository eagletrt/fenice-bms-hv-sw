/**
 * @file		can_comms.c
 * @brief		CAN communication stuff
 *
 * @date		Jul 16, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "can_comms.h"

#include "bal_fsm.h"
#include "can.h"
#include "cellboard_config.h"
#include "error.h"
#include "main.h"
#include "temp.h"
#include "volt.h"

#include <math.h>
#include <string.h>


void _can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    CAN_WAIT(hcan);

    uint32_t mailbox = 0;

    if(!HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX0))
        mailbox = CAN_TX_MAILBOX0;
    else if(!HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX1))
        mailbox = CAN_TX_MAILBOX1;
    else if(!HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX2))
        mailbox = CAN_TX_MAILBOX2;

    if (HAL_CAN_AddTxMessage(hcan, header, buffer, &mailbox) == HAL_OK) {
        ERROR_UNSET(ERROR_CAN);
    } else {
        ERROR_SET(ERROR_CAN);
    }
}

int serialize_bms_TOPIC_STATUS_FILTER(uint8_t *buffer, bms_errors errors, bms_balancing_status balancing_status) {
    switch (cellboard_index)
    {
        case 0: return serialize_bms_BOARD_STATUS_0(buffer, errors, balancing_status);
        case 1: return serialize_bms_BOARD_STATUS_1(buffer, errors, balancing_status);
        case 2: return serialize_bms_BOARD_STATUS_2(buffer, errors, balancing_status);
        case 3: return serialize_bms_BOARD_STATUS_3(buffer, errors, balancing_status);
        case 4: return serialize_bms_BOARD_STATUS_4(buffer, errors, balancing_status);
        case 7:
        case 5: return serialize_bms_BOARD_STATUS_5(buffer, errors, balancing_status);
        default: return -1;
    }
}

int serialize_bms_TOPIC_TEMPERATURE_INFO(uint8_t* buffer, uint8_t start_index, uint8_t temp0, uint8_t temp1, uint8_t temp2, uint8_t temp3, uint8_t temp4, uint8_t temp5) {
    switch (cellboard_index)
    {
        case 0: return serialize_bms_TEMP_STATS_0(buffer, start_index, temp0, temp1, temp2, temp3, temp4, temp5);
        case 1: return serialize_bms_TEMP_STATS_1(buffer, start_index, temp0, temp1, temp2, temp3, temp4, temp5);
        case 2: return serialize_bms_TEMP_STATS_2(buffer, start_index, temp0, temp1, temp2, temp3, temp4, temp5);
        case 3: return serialize_bms_TEMP_STATS_3(buffer, start_index, temp0, temp1, temp2, temp3, temp4, temp5);
        case 4: return serialize_bms_TEMP_STATS_4(buffer, start_index, temp0, temp1, temp2, temp3, temp4, temp5);
        case 7:
        case 5: return serialize_bms_TEMP_STATS_5(buffer, start_index, temp0, temp1, temp2, temp3, temp4, temp5);        
        default: return -1;
    }
}

int serialize_bms_TOPIC_VOLTAGE_INFO(uint8_t *buffer, uint8_t start_index, uint16_t voltage0, uint16_t voltage1, uint16_t voltage2) {
    switch (cellboard_index)
    {
        case 0: return serialize_bms_VOLTAGES_0(buffer, start_index, voltage0, voltage1, voltage2);
        case 1: return serialize_bms_VOLTAGES_1(buffer, start_index, voltage0, voltage1, voltage2);
        case 2: return serialize_bms_VOLTAGES_2(buffer, start_index, voltage0, voltage1, voltage2);
        case 3: return serialize_bms_VOLTAGES_3(buffer, start_index, voltage0, voltage1, voltage2);
        case 4: return serialize_bms_VOLTAGES_4(buffer, start_index, voltage0, voltage1, voltage2);
        case 7:
        case 5: return serialize_bms_VOLTAGES_5(buffer, start_index, voltage0, voltage1, voltage2);      
        default: return -1;
    }
}

void can_send(uint16_t topic_id) {
    CAN_TxHeaderTypeDef tx_header;
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    tx_header.ExtId = 0;
    tx_header.IDE   = CAN_ID_STD;
    tx_header.RTR   = CAN_RTR_DATA;

    if(topic_id == TOPIC_STATUS_FILTER){
        bms_balancing_status state = BAL_OFF;
        switch (fsm_get_state(bal.fsm)) {
            case BAL_OFF:
                state = bms_balancing_status_OFF;
                break;
            case BAL_DISCHARGE:
                state = bms_balancing_status_DISCHARGE;
                break;
        }

        switch(cellboard_index){
            case 0:
                tx_header.StdId = ID_BOARD_STATUS_0;
                break;
            case 1: 
                tx_header.StdId = ID_BOARD_STATUS_1;
                break;
            case 2: 
                tx_header.StdId = ID_BOARD_STATUS_2;
                break;
            case 3: 
                tx_header.StdId = ID_BOARD_STATUS_3;
                break;
            case 4: 
                tx_header.StdId = ID_BOARD_STATUS_4;
                break;
            case 7:
            case 5: 
                tx_header.StdId = ID_BOARD_STATUS_5;
                break;
            default: return;
        }

        tx_header.DLC = serialize_bms_TOPIC_STATUS_FILTER(buffer, &errors, state);
    } else if (topic_id == TOPIC_TEMPERATURE_INFO_FILTER) {
        switch(cellboard_index){
            case 0: 
                tx_header.StdId = ID_TEMP_STATS_0;
                break;
            case 1: 
                tx_header.StdId = ID_TEMP_STATS_1;
                break;
            case 2: 
                tx_header.StdId = ID_TEMP_STATS_2;
                break;
            case 3: 
                tx_header.StdId = ID_TEMP_STATS_3;
                break;
            case 4: 
                tx_header.StdId = ID_TEMP_STATS_4;
                break;
            case 7:
            case 5: 
                tx_header.StdId = ID_TEMP_STATS_5;
                break;
            default: return;
        }

        register uint8_t i;
        for(i=0; i<CELLBOARD_TEMP_SENSOR_COUNT; i+=6){
            tx_header.DLC = serialize_bms_TOPIC_TEMPERATURE_INFO(buffer, i, 
                temp_serialize(temperatures[i]),
                temp_serialize(temperatures[i+1]),
                temp_serialize(temperatures[i+2]),
                temp_serialize(temperatures[i+3]),
                temp_serialize(temperatures[i+4]),
                temp_serialize(temperatures[i+5])
            );
            _can_send(&BMS_CAN, buffer, &tx_header);
        }

        return;
    } else if (topic_id == TOPIC_VOLTAGE_INFO_FILTER) {
        switch(cellboard_index){
            case 0: 
                tx_header.StdId = ID_VOLTAGES_0;
                break;
            case 1:
                tx_header.StdId = ID_VOLTAGES_1;
                break;
            case 2: 
                tx_header.StdId = ID_VOLTAGES_2; 
                break;
            case 3: 
                tx_header.StdId = ID_VOLTAGES_3; 
                break;
            case 4: 
                tx_header.StdId = ID_VOLTAGES_4; 
                break;
            case 7:
            case 5: 
                tx_header.StdId = ID_VOLTAGES_5; 
                break;
            default: return;
        }

        register uint8_t i;
        for(i=0; i<CELLBOARD_CELL_COUNT; i+=3){
            tx_header.DLC = serialize_bms_TOPIC_VOLTAGE_INFO(buffer, i, voltages[i], voltages[i+1], voltages[i+2]);
            _can_send(&BMS_CAN, buffer, &tx_header);
        }

        return;
    }
    
    _can_send(&BMS_CAN, buffer, &tx_header);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    //void HAL_FDCAN_RxFifo0Callback(CAN_HandleTypeDef *hcan, uint32_t RxFifo0ITs) {
    //if (hfdcan->Instance == FDCAN1 && RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) {
    uint8_t rx_data[8] = {'\0'};
    CAN_RxHeaderTypeDef rx_header;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        //if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        ERROR_SET(ERROR_CAN);
        return;
    }

    ERROR_UNSET(ERROR_CAN);

    if(rx_header.StdId == ID_BALANCING){
        bms_BALANCING balancing;
        deserialize_bms_BALANCING(rx_data, &balancing);

        if(balancing.board_index != cellboard_index) return;

        fsm_trigger_event(bal.fsm, EV_BAL_STOP);

        memcpy(bal.cells, balancing.cells, sizeof(bal.cells));
        if(!bal_is_cells_empty()) {
            fsm_trigger_event(bal.fsm, EV_BAL_START);
        }
    }
    
}

void can_init_with_filter(){

    CAN_FilterTypeDef filter;
    filter.FilterMode       = CAN_FILTERMODE_IDMASK;
    filter.FilterIdLow      = TOPIC_BALANCING_FILTER << 5;                 // Take all ids from 0
    filter.FilterIdHigh     = TOPIC_BALANCING_FILTER << 5;  // to 2^11 - 1
    filter.FilterMaskIdHigh = TOPIC_BALANCING_MASK << 5;                 // Don't care on can id bits
    filter.FilterMaskIdLow  = TOPIC_BALANCING_MASK << 5;                 // Don't care on can id bits
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

}