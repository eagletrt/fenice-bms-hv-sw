/**
 * @file		can_comms.c
 * @brief		CAN communication stuff
 *
 * @date		Jul 16, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "can_comms.h"

#include "bal_fsm.h"
#include "bootloader.h"
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

#define RETRANSMISSION_MAX_ATTEMPTS 5
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
    CAN_TxHeaderTypeDef tx_header;
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    tx_header.ExtId = 0;
    tx_header.IDE   = CAN_ID_STD;
    tx_header.RTR   = CAN_RTR_DATA;

    if (topic_id == BMS_BOARD_STATUS_FRAME_ID /*bms_topic_filter_STATUS*/) {
        bms_board_status_t raw_state;
        bms_board_status_converted_t conv_state;
        
        conv_state.balancing_status = BMS_BOARD_STATUS_BALANCING_STATUS_OFF_CHOICE;

        switch (fsm_get_state(bal.fsm)) {
            case BAL_OFF:
                conv_state.balancing_status = BMS_BOARD_STATUS_BALANCING_STATUS_OFF_CHOICE;
                break;
            case BAL_COMPUTE:
            case BAL_DISCHARGE:
            case BAL_COOLDOWN:
                conv_state.balancing_status = BMS_BOARD_STATUS_BALANCING_STATUS_DISCHARGE_CHOICE;
                break;
        }

        // Add cellboard index in the payload
        conv_state.cellboard_id = (cellboard_index == 7)
            ? bms_board_status_cellboard_id_CELLBOARD_5
            : cellboard_index;

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
    } else if (topic_id == BMS_VOLTAGES_FRAME_ID) {
        tx_header.StdId = BMS_VOLTAGES_FRAME_ID;

        register uint8_t i;
        for (i = 0; i < CELLBOARD_CELL_COUNT; i += 3) {
            bms_voltages_t raw_volts;
            bms_voltages_converted_t conv_volts;
     
            // Add cellboard index in the payload
            conv_volts.cellboard_id = (cellboard_index == 7)
                ? bms_voltages_cellboard_id_CELLBOARD_5
                : cellboard_index;

            conv_volts.start_index = i;
            conv_volts.voltage0 = voltages[i];
            conv_volts.voltage1 = voltages[i + 1];
            conv_volts.voltage2 = voltages[i + 2];

            // Convert voltage to raw
            bms_voltages_conversion_to_raw_struct(&raw_volts, &conv_volts);

            tx_header.DLC = bms_voltages_pack(buffer, &raw_volts, BMS_VOLTAGES_BYTE_SIZE);
            _can_send(&BMS_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }

        return;
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

    // TODO: Handle balancing CAN message
    if (rx_header.StdId == BMS_BALANCING_FRAME_ID) {
        bms_balancing_t balancing;
        bms_balancing_unpack(&balancing, rx_data, BMS_BALANCING_BYTE_SIZE);

        if (balancing.board_index != cellboard_index)
            return;

        bms_balancing_raw_to_conversion_struct(&bal.cells, &balancing);

        if (!bal_is_cells_empty()) {
            fsm_trigger_event(bal.fsm, EV_BAL_START);
        } else {
            fsm_trigger_event(bal.fsm, EV_BAL_STOP);
        }
    } else if (rx_header.StdId == BMS_FW_UPDATE_FRAME_ID) {
        JumpToBlt();
    }
}

void can_init_with_filter() {
    CAN_FilterTypeDef filter;
    filter.FilterMode       = CAN_FILTERMODE_IDMASK;
    // TODO: Missing topic filters and masks
    // filter.FilterIdLow      = bms_TOPIC_FILTER_BALANCING << 5;  // Take all ids from 0
    // filter.FilterIdHigh     = bms_TOPIC_FILTER_BALANCING << 5;  // to 2^11 - 1
    // filter.FilterMaskIdHigh = bms_TOPIC_MASK_BALANCING << 5;    // Don't care on can id bits
    // filter.FilterMaskIdLow  = bms_TOPIC_MASK_BALANCING << 5;    // Don't care on can id bits
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
    // TODO: Missing topic filters and masks
    // filter.FilterIdLow      = BMS_FW_UPDATE_FRAME_ID << 5;  // Take all ids from 0
    // filter.FilterIdHigh     = BMS_FW_UPDATE_FRAME_ID << 5;  // to 2^11 - 1
    // filter.FilterMaskIdHigh = bms_TOPIC_MASK_FIXED_IDS << 5;    // Don't care on can id bits
    // filter.FilterMaskIdLow  = bms_TOPIC_MASK_FIXED_IDS << 5;    // Don't care on can id bits
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