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

void _can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    uint32_t mailbox = 0;

    if(HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX0))
        mailbox = CAN_TX_MAILBOX0;
    else if(HAL_CAN_IsTxMessagePending(hcan, CAN_TX_MAILBOX1))
        mailbox = CAN_TX_MAILBOX1;
    else
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
        case 0:
            return serialize_bms_BOARD_STATUS_0(buffer, errors, balancing_status);
        case 1:
            return serialize_bms_BOARD_STATUS_1(buffer, errors, balancing_status);
        case 2:
            return serialize_bms_BOARD_STATUS_2(buffer, errors, balancing_status);
        case 3:
            return serialize_bms_BOARD_STATUS_3(buffer, errors, balancing_status);
        case 4:
            return serialize_bms_BOARD_STATUS_4(buffer, errors, balancing_status);
        case 5:
            return serialize_bms_BOARD_STATUS_5(buffer, errors, balancing_status);
        
        default: return -1;
    }
}

int serialize_bms_TOPIC_TEMPERATURE_INFO(uint8_t *buffer, uint8_t average, uint8_t max, uint8_t min) {
    switch (cellboard_index)
    {
        case 0:
            return serialize_bms_TEMP_STATS_0(buffer, average, max, min);
        case 1:
            return serialize_bms_TEMP_STATS_1(buffer, average, max, min);
        case 2:
            return serialize_bms_TEMP_STATS_2(buffer, average, max, min);
        case 3:
            return serialize_bms_TEMP_STATS_3(buffer, average, max, min);
        case 4:
            return serialize_bms_TEMP_STATS_4(buffer, average, max, min);
        case 5:
            return serialize_bms_TEMP_STATS_5(buffer, average, max, min);        
        default: return -1;
    }
}

int serialize_bms_TOPIC_VOLTAGE_INFO(uint8_t *buffer, uint8_t start_index, uint16_t voltage0, uint16_t voltage1, uint16_t voltage2) {
    switch (cellboard_index)
    {
        case 0:
            return serialize_bms_VOLTAGES_0(buffer, start_index, voltage0, voltage1, voltage2);
        case 1:
            return serialize_bms_VOLTAGES_1(buffer, start_index, voltage0, voltage1, voltage2);
        case 2:
            return serialize_bms_VOLTAGES_2(buffer, start_index, voltage0, voltage1, voltage2);
        case 3:
            return serialize_bms_VOLTAGES_3(buffer, start_index, voltage0, voltage1, voltage2);
        case 4:
            return serialize_bms_VOLTAGES_4(buffer, start_index, voltage0, voltage1, voltage2);
        case 5:
            return serialize_bms_VOLTAGES_5(buffer, start_index, voltage0, voltage1, voltage2);      
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
            case BAL_COMPUTE:
                state = bms_balancing_status_COMPUTE;
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
            case 5: 
                tx_header.StdId = ID_BOARD_STATUS_5;
                break;
            default: return;
        }

        tx_header.DLC = serialize_bms_TOPIC_STATUS_FILTER(buffer, (uint8_t*)errors, state);
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
            case 5: 
                tx_header.StdId = ID_TEMP_STATS_5;
                break;
            default: return;
        }
        tx_header.DLC = serialize_bms_TOPIC_TEMPERATURE_INFO(buffer, temp_serialize(temp_get_average()), temp_serialize(temp_get_max()), temp_serialize(temp_get_min()));
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
            case 5: 
                tx_header.StdId = ID_VOLTAGES_5; 
                break;
            default: return;
        }

        register uint8_t i;
        for(i=0; i<18; i+=3){
            tx_header.DLC == serialize_bms_TOPIC_VOLTAGE_INFO(buffer, i, voltages[i], voltages[i+1], voltages[i+2]);
            _can_send(&BMS_CAN, buffer, &tx_header);
        }
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
    }
}