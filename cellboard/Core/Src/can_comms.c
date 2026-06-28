/**
 * @file can_comms.c
 * @brief CAN communication stuff
 *
 * @date Jul 16, 2021
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "can_comms.h"

#include "bal_fsm.h"
#include "can-bms-api.h"
#include "can-bms.h"
#include "can-version.h"
#include "can.h"
#include "cellboard_config.h"
#include "error.h"
#include "main.h"
#include "spi.h"
#include "temp.h"
#include "volt.h"

#include <math.h>
#include <string.h>
#include <time.h>

#define RETRANSMISSION_MAX_ATTEMPTS 1
uint8_t retransmission_attempts[3] = {0};

// static time_t build_epoch;

/**
 * @brief Wait until the CAN has at least one free mailbox
 *
 * @param hcan The CAN handler structure
 * @param timeout The maximum time to wait (in ms)
 * @return HAL_StatusTypeDef HAL_OK if there are free mailboxes
 * HAL_TIMEOUT otherwise
 */
HAL_StatusTypeDef _can_wait(CAN_HandleTypeDef *hcan, uint32_t timeout) {
    uint32_t tick = HAL_GetTick();
    while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0) {
        if (HAL_GetTick() - tick > timeout)
            return HAL_TIMEOUT;
    }
    return HAL_OK;
}

HAL_StatusTypeDef _can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    // Wait for free mailboxes
    if (_can_wait(hcan, 3) != HAL_OK)
        return HAL_TIMEOUT;

    // Add message to a free mailbox
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, NULL);
    ERROR_TOGGLE_CHECK(status != HAL_OK, ERROR_CAN);

    return status;
}

void can_init_with_filter() {
    // struct tm tm;
    // if (strptime(__DATE__" "__TIME__, "%b %d %Y %H:%M:%S", &tm) != NULL)
    //     build_epoch = mktime(&tm);

    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of:
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    // Add all balancing ids to the filter
    CAN_FilterTypeDef filter = {
        .FilterActivation     = CAN_FILTER_ENABLE,
        .FilterBank           = 0,
        .FilterFIFOAssignment = CAN_FILTER_FIFO0,
        .FilterIdLow          = 0,
        .FilterIdHigh         = ((1U << 11) - 1) << 5,
        .FilterMaskIdHigh     = 0,
        .FilterMaskIdLow      = 0,
        .FilterMode           = CAN_FILTERMODE_IDMASK,
        .FilterScale          = CAN_FILTERSCALE_16BIT,
        .SlaveStartFilterBank = 27};
    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);

    // Add jump to bootloader message id to the filters
    filter.FilterBank       = 1;
    filter.FilterIdHigh     = ((1U << 11) - 1) << 5;
    filter.FilterIdLow      = 0;
    filter.FilterMaskIdHigh = 0;
    filter.FilterMaskIdLow  = 0;
    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);

    // Start CAN
    HAL_CAN_ActivateNotification(
        &BMS_CAN, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_LAST_ERROR_CODE | CAN_IT_ERROR);
    HAL_CAN_Start(&BMS_CAN);
}
void can_send(uint16_t id) {
    // TODO: Check
    // if (HAL_GetTick() < 10000)
    //     return;

    CAN_TxHeaderTypeDef tx_header = {
        .DLC = 0, .ExtId = 0, .IDE = CAN_ID_STD, .RTR = CAN_RTR_DATA, .StdId = id, .TransmitGlobalTime = DISABLE};
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH] = {0};

    union CanBmsMessages message = {0};

    if (id == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_BOARD_STATUS) {
        bal_state_t fsm_state = fsm_get_state();

        message.cellboard_board_status.id = cellboard_index;
        message.cellboard_board_status.balancing =
            (fsm_state == STATE_DISCHARGE || fsm_state == STATE_COOLDOWN)
                ? CAN_BMS_CELLBOARD_SET_BALANCING_STATUS_BALANCINGSTATUS_OFF
                : CAN_BMS_CELLBOARD_SET_BALANCING_STATUS_BALANCINGSTATUS_DISCHARGE;

        message.cellboard_board_status.errorcancomm   = ERROR_GET(ERROR_CAN);
        message.cellboard_board_status.errorltccomm   = ERROR_GET(ERROR_LTC_COMM);
        message.cellboard_board_status.erroropenwire  = ERROR_GET(ERROR_OPEN_WIRE);
        message.cellboard_board_status.errortempcomm0 = ERROR_GET(ERROR_TEMP_COMM_0);
        message.cellboard_board_status.errortempcomm1 = ERROR_GET(ERROR_TEMP_COMM_1);
        message.cellboard_board_status.errortempcomm2 = ERROR_GET(ERROR_TEMP_COMM_2);
        message.cellboard_board_status.errortempcomm3 = ERROR_GET(ERROR_TEMP_COMM_3);
        message.cellboard_board_status.errortempcomm4 = ERROR_GET(ERROR_TEMP_COMM_4);
        message.cellboard_board_status.errortempcomm5 = ERROR_GET(ERROR_TEMP_COMM_5);

        message.cellboard_board_status.balancingcell0  = (bal_params.discharge_cells & 1) != 0;
        message.cellboard_board_status.balancingcell1  = (bal_params.discharge_cells & (1 << 1)) != 0;
        message.cellboard_board_status.balancingcell2  = (bal_params.discharge_cells & (1 << 2)) != 0;
        message.cellboard_board_status.balancingcell3  = (bal_params.discharge_cells & (1 << 3)) != 0;
        message.cellboard_board_status.balancingcell4  = (bal_params.discharge_cells & (1 << 4)) != 0;
        message.cellboard_board_status.balancingcell5  = (bal_params.discharge_cells & (1 << 5)) != 0;
        message.cellboard_board_status.balancingcell6  = (bal_params.discharge_cells & (1 << 6)) != 0;
        message.cellboard_board_status.balancingcell7  = (bal_params.discharge_cells & (1 << 7)) != 0;
        message.cellboard_board_status.balancingcell8  = (bal_params.discharge_cells & (1 << 8)) != 0;
        message.cellboard_board_status.balancingcell9  = (bal_params.discharge_cells & (1 << 9)) != 0;
        message.cellboard_board_status.balancingcell10 = (bal_params.discharge_cells & (1 << 10)) != 0;
        message.cellboard_board_status.balancingcell11 = (bal_params.discharge_cells & (1 << 11)) != 0;
        message.cellboard_board_status.balancingcell12 = (bal_params.discharge_cells & (1 << 12)) != 0;
        message.cellboard_board_status.balancingcell13 = (bal_params.discharge_cells & (1 << 13)) != 0;
        message.cellboard_board_status.balancingcell14 = (bal_params.discharge_cells & (1 << 14)) != 0;
        message.cellboard_board_status.balancingcell15 = (bal_params.discharge_cells & (1 << 15)) != 0;
        message.cellboard_board_status.balancingcell16 = (bal_params.discharge_cells & (1 << 16)) != 0;
        message.cellboard_board_status.balancingcell17 = (bal_params.discharge_cells & (1 << 17)) != 0;
    } else if (id == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_TEMPERATURES) {
        for (size_t i = 0; i < CELLBOARD_TEMP_SENSOR_COUNT; i += 4) {
            message.cellboard_temperatures.id         = cellboard_index;
            message.cellboard_temperatures.startindex = i;
            message.cellboard_temperatures.first_c    = temperatures[i];
            message.cellboard_temperatures.second_c   = temperatures[i + 1];
            message.cellboard_temperatures.third_c    = temperatures[i + 2];
            message.cellboard_temperatures.fourth_c   = temperatures[i + 3];

            // Serialize and send
            int serialize_byte_count = can_bms_api_serialize_from_id(id, &message, buffer);
            if (serialize_byte_count >= 0) {
                tx_header.DLC = serialize_byte_count;
                _can_send(&BMS_CAN, buffer, &tx_header);
                HAL_Delay(1);
            }
        }
        return;
    } else if (id == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_TEMPERATURES_INFO) {
        message.cellboard_temperatures_info.id        = cellboard_index;
        message.cellboard_temperatures_info.min_c     = temp_get_min();
        message.cellboard_temperatures_info.max_c     = temp_get_max();
        message.cellboard_temperatures_info.average_c = temp_get_average();
    } else if (id == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_VOLTAGES) {
        for (size_t i = 0; i < CELLBOARD_CELL_COUNT; i += 3) {
            message.cellboard_voltages.id         = cellboard_index;
            message.cellboard_voltages.startindex = i;
            message.cellboard_voltages.first_v    = CONVERT_VALUE_TO_VOLTAGE(voltages[i]);
            message.cellboard_voltages.second_v   = CONVERT_VALUE_TO_VOLTAGE(voltages[i + 1]);
            message.cellboard_voltages.third_v    = CONVERT_VALUE_TO_VOLTAGE(voltages[i + 2]);

            // Serialize and send
            int serialize_byte_count = can_bms_api_serialize_from_id(id, &message, buffer);
            if (serialize_byte_count >= 0) {
                tx_header.DLC = serialize_byte_count;
                _can_send(&BMS_CAN, buffer, &tx_header);
                HAL_Delay(1);
            }
        }
        return;
    } else if (id == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_VOLTAGES_INFO) {
        message.cellboard_voltages_info.id        = cellboard_index;
        message.cellboard_voltages_info.min_v     = CONVERT_VALUE_TO_VOLTAGE(volt_get_min());
        message.cellboard_voltages_info.max_v     = CONVERT_VALUE_TO_VOLTAGE(volt_get_max());
        message.cellboard_voltages_info.average_v = CONVERT_VALUE_TO_VOLTAGE(volt_get_avg());
    } else if (id == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_VERSION) {
        message.cellboard_version.id                = cellboard_index;
        message.cellboard_version.buildtime_s       = 1;  // build_epoch
        message.cellboard_version.canlibbuildtime_s = can_generation_time;
    } else {
        return;
    }

    // Serialize and send
    int serialize_byte_count = can_bms_api_serialize_from_id(id, &message, buffer);
    if (serialize_byte_count < 0)
        return;
    tx_header.DLC = serialize_byte_count;
    _can_send(&BMS_CAN, buffer, &tx_header);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rx_header           = {0};
    uint8_t rx_data[CAN_MAX_PAYLOAD_LENGTH] = {0};

    // Check for communication errors
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        ERROR_SET(ERROR_CAN);
        return;
    }

    if (hcan->Instance == BMS_CAN.Instance) {
        // Reset can errors
        ERROR_UNSET(ERROR_CAN);

        union CanBmsMessages message = {0};
        int deserialize_status       = can_bms_api_deserialize_from_id(rx_header.StdId, rx_data, &message);
        if (deserialize_status < 0) {
            ERROR_SET(ERROR_CAN);
            return;
        }

        if (rx_header.StdId == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_SET_BALANCING_STATUS) {
            // Set balancing parameters
            bal_params.target    = message.cellboard_set_balancing_status.target;
            bal_params.threshold = message.cellboard_set_balancing_status.threshold;

            // Request for balancing status change
            switch (message.cellboard_set_balancing_status.balancingstatus) {
                case CAN_BMS_CELLBOARD_SET_BALANCING_STATUS_BALANCINGSTATUS_OFF:
                    set_bal_request.is_new     = true;
                    set_bal_request.next_state = STATE_OFF;
                    break;
                case CAN_BMS_CELLBOARD_SET_BALANCING_STATUS_BALANCINGSTATUS_DISCHARGE:
                    set_bal_request.is_new     = true;
                    set_bal_request.next_state = STATE_DISCHARGE;
                    break;
            }
        } else if (rx_header.StdId == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_FLASH && fsm_get_state() == STATE_OFF) {
            if (message.cellboard_flash.id == cellboard_index)
                HAL_NVIC_SystemReset();
        }
    }
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
