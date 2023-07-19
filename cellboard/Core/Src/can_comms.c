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
#include "spi.h"
#include "temp.h"
#include "volt.h"
#include "bms/bms_network.h"

#include <math.h>
#include <string.h>

#define RETRANSMISSION_MAX_ATTEMPTS 1
uint8_t retransmission_attempts[3] = {0};

HAL_StatusTypeDef CAN_WAIT(CAN_HandleTypeDef *hcan, uint8_t timeout) {
    uint32_t tick = HAL_GetTick();
    while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0) {
        if (HAL_GetTick() - tick > timeout)
            return HAL_TIMEOUT;
    }
    return HAL_OK;
}

HAL_StatusTypeDef _can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    if(CAN_WAIT(hcan, 3) != HAL_OK) {
        return HAL_TIMEOUT;
    }

    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, NULL);
    if (status == HAL_OK) {
        ERROR_UNSET(ERROR_CAN);
    } else {
        ERROR_SET(ERROR_CAN);
    }
    return status;
}

void can_send(uint16_t topic_id) {
    if (HAL_GetTick() < 10000)
        return;

    CAN_TxHeaderTypeDef tx_header;
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH] = { 0 };

    tx_header.ExtId = 0;
    tx_header.IDE   = CAN_ID_STD;
    tx_header.RTR   = CAN_RTR_DATA;

    if (topic_id == BMS_BOARD_STATUS_FRAME_ID /*bms_topic_filter_STATUS*/) {
        bms_board_status_t raw_state;
        bms_board_status_converted_t conv_state;
        
        conv_state.balancing_status = BMS_BOARD_STATUS_BALANCING_STATUS_OFF_CHOICE;

        switch (fsm_get_state()) {
            case STATE_INIT:
            case STATE_OFF:
                conv_state.balancing_status = BMS_BOARD_STATUS_BALANCING_STATUS_OFF_CHOICE;
                break;
            case STATE_DISCHARGE:
            case STATE_COOLDOWN:
                conv_state.balancing_status = BMS_BOARD_STATUS_BALANCING_STATUS_DISCHARGE_CHOICE;
                break;
        }

        // Add cellboard index in the payload
        conv_state.cellboard_id = (cellboard_index == 7)
            ? bms_board_status_cellboard_id_CELLBOARD_5
            : cellboard_index;

        conv_state.errors_can_comm = ERROR_GET(ERROR_CAN);
        conv_state.errors_ltc_comm = ERROR_GET(ERROR_LTC_COMM);
        conv_state.errors_open_wire = ERROR_GET(ERROR_OPEN_WIRE);
        conv_state.errors_temp_comm_0 = ERROR_GET(ERROR_TEMP_COMM_0);
        conv_state.errors_temp_comm_1 = ERROR_GET(ERROR_TEMP_COMM_1);
        conv_state.errors_temp_comm_2 = ERROR_GET(ERROR_TEMP_COMM_2);
        conv_state.errors_temp_comm_3 = ERROR_GET(ERROR_TEMP_COMM_3);
        conv_state.errors_temp_comm_4 = ERROR_GET(ERROR_TEMP_COMM_4);
        conv_state.errors_temp_comm_5 = ERROR_GET(ERROR_TEMP_COMM_5);

        conv_state.balancing_cells_cell0  = bal_params.discharge_cells & 1;
        conv_state.balancing_cells_cell1  = bal_params.discharge_cells & (1 << 1);
        conv_state.balancing_cells_cell2  = bal_params.discharge_cells & (1 << 2);
        conv_state.balancing_cells_cell3  = bal_params.discharge_cells & (1 << 3);
        conv_state.balancing_cells_cell4  = bal_params.discharge_cells & (1 << 4);
        conv_state.balancing_cells_cell5  = bal_params.discharge_cells & (1 << 5);
        conv_state.balancing_cells_cell6  = bal_params.discharge_cells & (1 << 6);
        conv_state.balancing_cells_cell7  = bal_params.discharge_cells & (1 << 7);
        conv_state.balancing_cells_cell8  = bal_params.discharge_cells & (1 << 8);
        conv_state.balancing_cells_cell9  = bal_params.discharge_cells & (1 << 9);
        conv_state.balancing_cells_cell10 = bal_params.discharge_cells & (1 << 10);
        conv_state.balancing_cells_cell11 = bal_params.discharge_cells & (1 << 11);
        conv_state.balancing_cells_cell12 = bal_params.discharge_cells & (1 << 12);
        conv_state.balancing_cells_cell13 = bal_params.discharge_cells & (1 << 13);
        conv_state.balancing_cells_cell14 = bal_params.discharge_cells & (1 << 14);
        conv_state.balancing_cells_cell15 = bal_params.discharge_cells & (1 << 15);
        conv_state.balancing_cells_cell16 = bal_params.discharge_cells & (1 << 16);
        conv_state.balancing_cells_cell17 = bal_params.discharge_cells & (1 << 17);

        bms_board_status_conversion_to_raw_struct(&raw_state, &conv_state);

        tx_header.StdId = BMS_BOARD_STATUS_FRAME_ID;
        tx_header.DLC = bms_board_status_pack(buffer, &raw_state, BMS_BOARD_STATUS_BYTE_SIZE);

        _can_send(&BMS_CAN, buffer, &tx_header);
        return;
    } else if (topic_id == BMS_TEMPERATURES_FRAME_ID) {
        tx_header.StdId = BMS_TEMPERATURES_FRAME_ID;

        register uint8_t i;
        for (i = 0; i < CELLBOARD_TEMP_SENSOR_COUNT; i += 4) {
            bms_temperatures_t raw_temps;
            bms_temperatures_converted_t conv_temps;

            // Add cellboard index in the payload
            conv_temps.cellboard_id = (cellboard_index == 7)
                ? bms_temperatures_cellboard_id_CELLBOARD_5
                : cellboard_index;

            conv_temps.start_index = i,
            conv_temps.temp0 = temperatures[i];
            conv_temps.temp1 = temperatures[i + 1];
            conv_temps.temp2 = temperatures[i + 2];
            conv_temps.temp3 = temperatures[i + 3];

            bms_temperatures_conversion_to_raw_struct(&raw_temps, &conv_temps);
            
            tx_header.DLC = bms_temperatures_pack(buffer, &raw_temps, BMS_TEMPERATURES_BYTE_SIZE);
            _can_send(&BMS_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        return;
    }
    else if (topic_id == BMS_TEMPERATURES_INFO_FRAME_ID) {
        tx_header.StdId = BMS_TEMPERATURES_INFO_FRAME_ID;

        register uint8_t i;
        for (i = 0; i < CELLBOARD_TEMP_SENSOR_COUNT; i += 4) {
            bms_temperatures_info_t raw_temps;
            bms_temperatures_info_converted_t conv_temps;

            conv_temps.cellboard_id = cellboard_index;
            conv_temps.min_temp = temp_get_min();
            conv_temps.max_temp = temp_get_max();
            conv_temps.avg_temp = temp_get_average();

            bms_temperatures_info_conversion_to_raw_struct(&raw_temps, &conv_temps);
            
            tx_header.DLC = bms_temperatures_info_pack(buffer, &raw_temps, BMS_TEMPERATURES_INFO_BYTE_SIZE);
            _can_send(&BMS_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        return;
    } 
    else if (topic_id == BMS_VOLTAGES_FRAME_ID) {
        tx_header.StdId = BMS_VOLTAGES_FRAME_ID;

        for (size_t i = 0; i < CELLBOARD_CELL_COUNT; i += 3) {
            bms_voltages_t raw_volts;
            bms_voltages_converted_t conv_volts;
     
            // Add cellboard index in the payload
            conv_volts.cellboard_id = cellboard_index;

            conv_volts.start_index = i;
            conv_volts.voltage0 = CONVERT_VALUE_TO_VOLTAGE(voltages[i]);
            conv_volts.voltage1 = CONVERT_VALUE_TO_VOLTAGE(voltages[i + 1]);
            conv_volts.voltage2 = CONVERT_VALUE_TO_VOLTAGE(voltages[i + 2]);

            // Convert voltage to raw
            bms_voltages_conversion_to_raw_struct(&raw_volts, &conv_volts);

            tx_header.DLC = bms_voltages_pack(buffer, &raw_volts, BMS_VOLTAGES_BYTE_SIZE);
            _can_send(&BMS_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        return;
    }
    else if (topic_id == BMS_VOLTAGES_INFO_FRAME_ID) {
        tx_header.StdId = BMS_VOLTAGES_INFO_FRAME_ID;

        register uint8_t i;
        for (i = 0; i < CELLBOARD_CELL_COUNT; i += 3) {
            bms_voltages_info_t raw_volts;
            bms_voltages_info_converted_t conv_volts;
     
            // Add cellboard index in the payload
            conv_volts.cellboard_id = cellboard_index;
            conv_volts.min_voltage = CONVERT_VALUE_TO_VOLTAGE(volt_get_min());
            conv_volts.max_voltage = CONVERT_VALUE_TO_VOLTAGE(volt_get_max());
            conv_volts.avg_voltage = CONVERT_VALUE_TO_VOLTAGE(volt_get_avg());

            // Convert voltage to raw
            bms_voltages_info_conversion_to_raw_struct(&raw_volts, &conv_volts);

            tx_header.DLC = bms_voltages_info_pack(buffer, &raw_volts, BMS_VOLTAGES_INFO_BYTE_SIZE);
            _can_send(&BMS_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef * hcan) {
    uint8_t rx_data[8] = { '\0' };
    CAN_RxHeaderTypeDef rx_header;
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        ERROR_SET(ERROR_CAN);
        return;
    }
    ERROR_UNSET(ERROR_CAN);

    if (rx_header.StdId == BMS_SET_BALANCING_STATUS_FRAME_ID) {
        bms_set_balancing_status_t raw_bal;
        bms_set_balancing_status_converted_t conv_bal;
        bms_set_balancing_status_unpack(&raw_bal, rx_data, BMS_SET_BALANCING_STATUS_BYTE_SIZE);

        bms_set_balancing_status_raw_to_conversion_struct(&conv_bal, &raw_bal);

        // Set balancing parameters
        bal_params.target = conv_bal.target;
        bal_params.threshold = conv_bal.threshold;
        
        switch(conv_bal.balancing_status) {
            case bms_set_balancing_status_balancing_status_OFF:
                set_bal_request.is_new = true;
                set_bal_request.next_state = STATE_OFF;
                break;
            case bms_set_balancing_status_balancing_status_DISCHARGE:
                set_bal_request.is_new = true;
                set_bal_request.next_state = STATE_DISCHARGE;
                break;
        }
    } else if (rx_header.StdId == BMS_JMP_TO_BLT_FRAME_ID && fsm_get_state() == STATE_OFF) {
        HAL_NVIC_SystemReset();
    }
}

void can_init_with_filter() {
    CAN_FilterTypeDef filter;
    filter.FilterMode       = CAN_FILTERMODE_IDMASK;
    filter.FilterIdLow      = BMS_TOPIC_FILTER_BALANCING << 5;  // Take all ids from 0
    filter.FilterIdHigh     = BMS_TOPIC_FILTER_BALANCING << 5;  // to 2^11 - 1
    filter.FilterMaskIdHigh = BMS_TOPIC_MASK_BALANCING << 5;    // Don't care on can id bits
    filter.FilterMaskIdLow  = BMS_TOPIC_MASK_BALANCING << 5;    // Don't care on can id bits
    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of: 
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterBank           = 0;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;
    filter.FilterActivation     = ENABLE;

    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);


    filter.FilterMode       = CAN_FILTERMODE_IDMASK;
    filter.FilterIdLow      = BMS_JMP_TO_BLT_FRAME_ID << 5;  // Take all ids from 0
    filter.FilterIdHigh     = BMS_JMP_TO_BLT_FRAME_ID << 5;  // to 2^11 - 1
    filter.FilterMaskIdHigh = BMS_TOPIC_MASK_FIXED_IDS << 5;    // Don't care on can id bits
    filter.FilterMaskIdLow  = BMS_TOPIC_MASK_FIXED_IDS << 5;    // Don't care on can id bits
    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of: 
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterBank           = 1;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;
    filter.FilterActivation     = ENABLE;

    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);

    HAL_CAN_ActivateNotification(
        &BMS_CAN, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_LAST_ERROR_CODE | CAN_IT_ERROR);
    HAL_CAN_Start(&BMS_CAN);
}

void CAN_TxCallback(CAN_HandleTypeDef *hcan, uint32_t mailbox) {
    switch (mailbox) {
        case CAN_TX_MAILBOX0:
            retransmission_attempts[0] = 0;
            break;
        case CAN_TX_MAILBOX1:
            retransmission_attempts[1] = 0;
            break;
        case CAN_TX_MAILBOX2:
            retransmission_attempts[2] = 0;
            break;
        default:
            break;
    }
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
    if (hcan->ErrorCode & (HAL_CAN_ERROR_TX_ALST0 | HAL_CAN_ERROR_TX_TERR0))
        ++retransmission_attempts[0];
    if (hcan->ErrorCode & (HAL_CAN_ERROR_TX_ALST1 | HAL_CAN_ERROR_TX_TERR1))
        ++retransmission_attempts[1];
    if (hcan->ErrorCode & (HAL_CAN_ERROR_TX_ALST2 | HAL_CAN_ERROR_TX_TERR2))
        ++retransmission_attempts[2];

    uint32_t mailboxes = 0;
    if (retransmission_attempts[0] >= RETRANSMISSION_MAX_ATTEMPTS) {
        mailboxes |= CAN_TX_MAILBOX0;
        retransmission_attempts[0] = 0;
    }
    if (retransmission_attempts[1] >= RETRANSMISSION_MAX_ATTEMPTS) {
        mailboxes |= CAN_TX_MAILBOX1;
        retransmission_attempts[1] = 0;
    }
    if (retransmission_attempts[2] >= RETRANSMISSION_MAX_ATTEMPTS) {
        mailboxes |= CAN_TX_MAILBOX2;
        retransmission_attempts[1] = 0;
    }

    if (hcan->ErrorCode & (HAL_CAN_ERROR_BD | HAL_CAN_ERROR_BR))
        mailboxes = CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2;

    if (mailboxes != 0)
        HAL_CAN_AbortTxRequest(hcan, mailboxes);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan) {
    CAN_TxCallback(hcan, CAN_TX_MAILBOX0);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan) {
    CAN_TxCallback(hcan, CAN_TX_MAILBOX1);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan) {
    CAN_TxCallback(hcan, CAN_TX_MAILBOX2);
}