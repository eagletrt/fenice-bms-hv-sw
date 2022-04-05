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
#include "spi.h"
#include "bootloader.h"

#include <math.h>
#include <string.h>


void _can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    CAN_WAIT(hcan);

    if (HAL_CAN_AddTxMessage(hcan, header, buffer, NULL) == HAL_OK) {
        ERROR_UNSET(ERROR_CAN);
    } else {
        ERROR_SET(ERROR_CAN);
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

        tx_header.DLC = serialize_bms_BOARD_STATUS(buffer, &errors, state);
    
        _can_send(&BMS_CAN, buffer, &tx_header);
        return;
    } else if (topic_id == TOPIC_TEMPERATURE_INFO_FILTER) {
        switch(cellboard_index){
            case 0: 
                tx_header.StdId = ID_TEMPERATURES_0;
                break;
            case 1: 
                tx_header.StdId = ID_TEMPERATURES_1;
                break;
            case 2: 
                tx_header.StdId = ID_TEMPERATURES_2;
                break;
            case 3: 
                tx_header.StdId = ID_TEMPERATURES_3;
                break;
            case 4: 
                tx_header.StdId = ID_TEMPERATURES_4;
                break;
            case 7:
            case 5: 
                tx_header.StdId = ID_TEMPERATURES_5;
                break;
            default: return;
        }

        register uint8_t i;
        for(i=0; i<CELLBOARD_TEMP_SENSOR_COUNT; i+=6){

            tx_header.DLC = serialize_bms_TEMPERATURES(buffer, i, 
                temp_serialize(temperatures[i]),
                temp_serialize(temperatures[i+1]),
                temp_serialize(temperatures[i+2]),
                temp_serialize(temperatures[i+3]),
                temp_serialize(temperatures[i+4]),
                temp_serialize(temperatures[i+5])
            );
            _can_send(&BMS_CAN, buffer, &tx_header);
            HAL_Delay(1);
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
            tx_header.DLC = serialize_bms_VOLTAGES(buffer, i, voltages[i], voltages[i+1], voltages[i+2]);
            _can_send(&BMS_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }

        return;
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    uint8_t rx_data[8] = {'\0'};
    CAN_RxHeaderTypeDef rx_header;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        ERROR_SET(ERROR_CAN);
        return;
    }

    ERROR_UNSET(ERROR_CAN);

    if(rx_header.StdId == ID_BALANCING){
        bms_BALANCING balancing;
        deserialize_bms_BALANCING(rx_data, &balancing);

        if(balancing.board_index != cellboard_index) return;

        memcpy(bal.cells, balancing.cells, sizeof(bal.cells));
        if(!bal_is_cells_empty()) {
            fsm_trigger_event(bal.fsm, EV_BAL_START);
        } else {
            fsm_trigger_event(bal.fsm, EV_BAL_STOP);
        }
    }
    else if(rx_header.StdId == ID_FW_UPDATE) {
        bms_FW_UPDATE fw_update;
        deserialize_bms_FW_UPDATE(rx_data, &fw_update);

        if(fw_update.board_index != cellboard_index) return;

        BootLoaderInit();
    }
}

void can_init_with_filter(){
    CAN_FilterTypeDef filter;
    filter.FilterMode       = CAN_FILTERMODE_IDMASK;
    filter.FilterIdLow      = TOPIC_BALANCING_FILTER << 5;                 // Take all ids from 0
    filter.FilterIdHigh     = TOPIC_FW_UPDATE_FILTER << 5;                  // to 2^11 - 1
    filter.FilterMaskIdHigh = TOPIC_FW_UPDATE_MASK << 5;                 // Don't care on can id bits
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