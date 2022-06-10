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

#include <math.h>
#include <string.h>

#define RETRANSMISSION_MAX_ATTEMPTS 5
uint8_t retransmission_attempts[3] = {0};

HAL_StatusTypeDef _can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    CAN_WAIT(hcan);

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

    if (topic_id == 0/*bms_topic_filter_STATUS*/) {
        bms_BalancingStatus state = BAL_OFF;
        switch (fsm_get_state(bal.fsm)) {
            case BAL_OFF:
                state = bms_BalancingStatus_OFF;
                break;
            case BAL_DISCHARGE:
                state = bms_BalancingStatus_DISCHARGE;
                break;
        }

        switch (cellboard_index) {
            case 0:
                tx_header.StdId = bms_id_BOARD_STATUS_CELLBOARD0;
                break;
            case 1:
                tx_header.StdId = bms_id_BOARD_STATUS_CELLBOARD1;
                break;
            case 2:
                tx_header.StdId = bms_id_BOARD_STATUS_CELLBOARD2;
                break;
            case 3:
                tx_header.StdId = bms_id_BOARD_STATUS_CELLBOARD3;
                break;
            case 4:
                tx_header.StdId = bms_id_BOARD_STATUS_CELLBOARD4;
                break;
            case 7:
            case 5:
                tx_header.StdId = bms_id_BOARD_STATUS_CELLBOARD5;
                break;
            default:
                return;
        }

        tx_header.DLC = bms_serialize_BOARD_STATUS(buffer, errors, state);

        _can_send(&BMS_CAN, buffer, &tx_header);
        return;
    } else if (topic_id == bms_topic_filter_TEMPERATURE_INFO) {
        switch (cellboard_index) {
            case 0:
                tx_header.StdId = bms_id_TEMPERATURES_CELLBOARD0;
                break;
            case 1:
                tx_header.StdId = bms_id_TEMPERATURES_CELLBOARD1;
                break;
            case 2:
                tx_header.StdId = bms_id_TEMPERATURES_CELLBOARD2;
                break;
            case 3:
                tx_header.StdId = bms_id_TEMPERATURES_CELLBOARD3;
                break;
            case 4:
                tx_header.StdId = bms_id_TEMPERATURES_CELLBOARD4;
                break;
            case 7:
            case 5:
                tx_header.StdId = bms_id_TEMPERATURES_CELLBOARD5;
                break;
            default:
                return;
        }

        bms_message_TEMPERATURES raw_temps;
        bms_message_TEMPERATURES_conversion conv_temps;
        register uint8_t i;
        for (i = 0; i < CELLBOARD_TEMP_SENSOR_COUNT; i += 6) {
            conv_temps.start_index = i,
            conv_temps.temp0 = temperatures[i];
            conv_temps.temp1 = temperatures[i+1];
            conv_temps.temp2 = temperatures[i+2];
            conv_temps.temp3 = temperatures[i+3];
            conv_temps.temp4 = temperatures[i+4];
            conv_temps.temp5 = temperatures[i+5];

            bms_conversion_to_raw_TEMPERATURES(&raw_temps, &conv_temps);

            tx_header.DLC = bms_serialize_struct_TEMPERATURES(buffer, &raw_temps);
            _can_send(&BMS_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }

        return;
    } else if (topic_id == bms_topic_filter_VOLTAGE_INFO) {
        switch (cellboard_index) {
            case 0:
                tx_header.StdId = bms_id_VOLTAGES_CELLBOARD0;
                break;
            case 1:
                tx_header.StdId = bms_id_VOLTAGES_CELLBOARD1;
                break;
            case 2:
                tx_header.StdId = bms_id_VOLTAGES_CELLBOARD2;
                break;
            case 3:
                tx_header.StdId = bms_id_VOLTAGES_CELLBOARD3;
                break;
            case 4:
                tx_header.StdId = bms_id_VOLTAGES_CELLBOARD4;
                break;
            case 7:
            case 5:
                tx_header.StdId = bms_id_VOLTAGES_CELLBOARD5;
                break;
            default:
                return;
        }

        register uint8_t i;
        for (i = 0; i < CELLBOARD_CELL_COUNT; i += 3) {
            tx_header.DLC = bms_serialize_VOLTAGES(buffer, voltages[i], voltages[i + 1], voltages[i + 2], i);
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

    if (rx_header.StdId == bms_id_BALANCING) {
        bms_message_BALANCING balancing;
        bms_deserialize_BALANCING(&balancing, rx_data);

        if (balancing.board_index != cellboard_index)
            return;

        bal.cells = balancing.cells;
        if (!bal_is_cells_empty()) {
            fsm_trigger_event(bal.fsm, EV_BAL_START);
        } else {
            fsm_trigger_event(bal.fsm, EV_BAL_STOP);
        }
    } else if (rx_header.StdId == bms_id_FW_UPDATE) {
        JumpToBlt();
    }
}

void can_init_with_filter() {
    CAN_FilterTypeDef filter;
    filter.FilterMode       = CAN_FILTERMODE_IDMASK;
    filter.FilterIdLow      = bms_topic_filter_BALANCING << 5;  // Take all ids from 0
    filter.FilterIdHigh     = bms_topic_filter_BALANCING << 5;  // to 2^11 - 1
    filter.FilterMaskIdHigh = bms_topic_mask_BALANCING << 5;    // Don't care on can id bits
    filter.FilterMaskIdLow  = bms_topic_mask_BALANCING << 5;    // Don't care on can id bits
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
    filter.FilterIdLow      = bms_id_FW_UPDATE << 5;  // Take all ids from 0
    filter.FilterIdHigh     = bms_id_FW_UPDATE << 5;  // to 2^11 - 1
    filter.FilterMaskIdHigh = bms_topic_mask_FIXED_IDS << 5;    // Don't care on can id bits
    filter.FilterMaskIdLow  = bms_topic_mask_FIXED_IDS << 5;    // Don't care on can id bits
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