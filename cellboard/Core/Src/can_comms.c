/**
 * @file can_comms.c
 * @brief CAN communication stuff
 *
 * @date Jul 16, 2021
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "can_comms.h"

#include <math.h>
#include <string.h>
#include <time.h>

#include "bal_fsm.h"
#include "can.h"
#include "cellboard_config.h"
#include "error.h"
#include "main.h"
#include "spi.h"
#include "temp.h"
#include "volt.h"
#include "bms_network.h"

#define RETRANSMISSION_MAX_ATTEMPTS 1
uint8_t retransmission_attempts[3] = { 0 };

// static time_t build_epoch;

/**
 * @brief Wait until the CAN has at least one free mailbox
 * 
 * @param hcan The CAN handler structure
 * @param timeout The maximum time to wait (in ms)
 * @return HAL_StatusTypeDef HAL_OK if there are free mailboxes
 * HAL_TIMEOUT otherwise
 */
HAL_StatusTypeDef _can_wait(CAN_HandleTypeDef * hcan, uint32_t timeout) {
    uint32_t tick = HAL_GetTick();
    while (HAL_CAN_GetTxMailboxesFreeLevel(hcan) == 0) {
        if (HAL_GetTick() - tick > timeout)
            return HAL_TIMEOUT;
    }
    return HAL_OK;
}

HAL_StatusTypeDef _can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    // Wait for free mailboxes
    if(_can_wait(hcan, 3) != HAL_OK)
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
        .FilterActivation = CAN_FILTER_ENABLE,
        .FilterBank = 0,
        .FilterFIFOAssignment = CAN_FILTER_FIFO0,
        .FilterIdHigh = BMS_SET_BALANCING_STATUS_FRAME_ID << 5,
        .FilterIdLow = BMS_SET_BALANCING_STATUS_FRAME_ID << 5,
        .FilterMaskIdHigh = BMS_TOPIC_MASK_BALANCING << 5,
        .FilterMaskIdLow = BMS_TOPIC_MASK_BALANCING << 5,
        .FilterMode = CAN_FILTERMODE_IDMASK,
        .FilterScale = CAN_FILTERSCALE_16BIT,
        .SlaveStartFilterBank = 27
    };
    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);

    // Add jump to bootloader message id to the filters
    filter.FilterBank = 1;
    filter.FilterIdLow = BMS_JMP_TO_BLT_FRAME_ID << 5;
    filter.FilterIdHigh = BMS_JMP_TO_BLT_FRAME_ID << 5;
    filter.FilterMaskIdHigh = BMS_TOPIC_MASK_FIXED_IDS << 5;
    filter.FilterMaskIdLow = BMS_TOPIC_MASK_FIXED_IDS << 5;
    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);

    // Start CAN
    HAL_CAN_ActivateNotification(&BMS_CAN, CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_LAST_ERROR_CODE | CAN_IT_ERROR);
    HAL_CAN_Start(&BMS_CAN);
}
void can_send(uint16_t id) {
    // TODO: Check
    // if (HAL_GetTick() < 10000)
    //     return;

    CAN_TxHeaderTypeDef tx_header = {
        .DLC = 0,
        .ExtId = 0,
        .IDE = CAN_ID_STD,
        .RTR = CAN_RTR_DATA,
        .StdId = id,
        .TransmitGlobalTime = DISABLE
    };
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH] = { 0 };

    if (id == BMS_BOARD_STATUS_FRAME_ID) {
        bms_board_status_t raw_state = { 0 };
        bms_board_status_converted_t conv_state = { 0 };

        conv_state.cellboard_id = cellboard_index;
        conv_state.balancing_status = BMS_BOARD_STATUS_BALANCING_STATUS_OFF_CHOICE;

        bal_state_t fsm_state = fsm_get_state();
        if (fsm_state == STATE_DISCHARGE || fsm_state == STATE_COOLDOWN)
            conv_state.balancing_status = BMS_BOARD_STATUS_BALANCING_STATUS_DISCHARGE_CHOICE;
        else
            conv_state.balancing_status = BMS_BOARD_STATUS_BALANCING_STATUS_OFF_CHOICE;

        conv_state.errors_can_comm = ERROR_GET(ERROR_CAN);
        conv_state.errors_ltc_comm = ERROR_GET(ERROR_LTC_COMM);
        conv_state.errors_open_wire = ERROR_GET(ERROR_OPEN_WIRE);
        conv_state.errors_temp_comm_0 = ERROR_GET(ERROR_TEMP_COMM_0);
        conv_state.errors_temp_comm_1 = ERROR_GET(ERROR_TEMP_COMM_1);
        conv_state.errors_temp_comm_2 = ERROR_GET(ERROR_TEMP_COMM_2);
        conv_state.errors_temp_comm_3 = ERROR_GET(ERROR_TEMP_COMM_3);
        conv_state.errors_temp_comm_4 = ERROR_GET(ERROR_TEMP_COMM_4);
        conv_state.errors_temp_comm_5 = ERROR_GET(ERROR_TEMP_COMM_5);

        conv_state.balancing_cells_cell0  = (bal_params.discharge_cells & 1) != 0;
        conv_state.balancing_cells_cell1  = (bal_params.discharge_cells & (1 << 1)) != 0;
        conv_state.balancing_cells_cell2  = (bal_params.discharge_cells & (1 << 2)) != 0;
        conv_state.balancing_cells_cell3  = (bal_params.discharge_cells & (1 << 3)) != 0;
        conv_state.balancing_cells_cell4  = (bal_params.discharge_cells & (1 << 4)) != 0;
        conv_state.balancing_cells_cell5  = (bal_params.discharge_cells & (1 << 5)) != 0;
        conv_state.balancing_cells_cell6  = (bal_params.discharge_cells & (1 << 6)) != 0;
        conv_state.balancing_cells_cell7  = (bal_params.discharge_cells & (1 << 7)) != 0;
        conv_state.balancing_cells_cell8  = (bal_params.discharge_cells & (1 << 8)) != 0;
        conv_state.balancing_cells_cell9  = (bal_params.discharge_cells & (1 << 9)) != 0;
        conv_state.balancing_cells_cell10 = (bal_params.discharge_cells & (1 << 10)) != 0;
        conv_state.balancing_cells_cell11 = (bal_params.discharge_cells & (1 << 11)) != 0;
        conv_state.balancing_cells_cell12 = (bal_params.discharge_cells & (1 << 12)) != 0;
        conv_state.balancing_cells_cell13 = (bal_params.discharge_cells & (1 << 13)) != 0;
        conv_state.balancing_cells_cell14 = (bal_params.discharge_cells & (1 << 14)) != 0;
        conv_state.balancing_cells_cell15 = (bal_params.discharge_cells & (1 << 15)) != 0;
        conv_state.balancing_cells_cell16 = (bal_params.discharge_cells & (1 << 16)) != 0;
        conv_state.balancing_cells_cell17 = (bal_params.discharge_cells & (1 << 17)) != 0;

        bms_board_status_conversion_to_raw_struct(&raw_state, &conv_state);

        int data_len = bms_board_status_pack(buffer, &raw_state, BMS_BOARD_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return;
        tx_header.DLC = data_len;
    }
    else if (id == BMS_TEMPERATURES_FRAME_ID) {
        for (size_t i = 0; i < CELLBOARD_TEMP_SENSOR_COUNT; i += 4) {
            bms_temperatures_t raw_temps = { 0 };
            bms_temperatures_converted_t conv_temps = { 0 };

            conv_temps.cellboard_id = cellboard_index;
            conv_temps.start_index = i,
            conv_temps.temp0 = temperatures[i];
            conv_temps.temp1 = temperatures[i + 1];
            conv_temps.temp2 = temperatures[i + 2];
            conv_temps.temp3 = temperatures[i + 3];

            bms_temperatures_conversion_to_raw_struct(&raw_temps, &conv_temps);
            
            int data_len = bms_temperatures_pack(buffer, &raw_temps, BMS_TEMPERATURES_BYTE_SIZE);
            if (data_len >= 0) {
                tx_header.DLC = data_len;
                _can_send(&BMS_CAN, buffer, &tx_header);
                HAL_Delay(1);
            }
        }
        return;
    }
    else if (id == BMS_TEMPERATURES_INFO_FRAME_ID) {
        bms_temperatures_info_t raw_temps = { 0 };
        bms_temperatures_info_converted_t conv_temps = { 0 };

        conv_temps.cellboard_id = cellboard_index;
        conv_temps.min_temp = temp_get_min();
        conv_temps.max_temp = temp_get_max();
        conv_temps.avg_temp = temp_get_average();

        bms_temperatures_info_conversion_to_raw_struct(&raw_temps, &conv_temps);
        
        int data_len = bms_temperatures_info_pack(buffer, &raw_temps, BMS_TEMPERATURES_INFO_BYTE_SIZE);
        if (data_len < 0)
            return;
        tx_header.DLC = data_len;
    } 
    else if (id == BMS_VOLTAGES_FRAME_ID) {
        for (size_t i = 0; i < CELLBOARD_CELL_COUNT; i += 3) {
            bms_voltages_t raw_volts = { 0 };
            bms_voltages_converted_t conv_volts = { 0 };
     
            conv_volts.cellboard_id = cellboard_index;
            conv_volts.start_index = i;
            conv_volts.voltage0 = CONVERT_VALUE_TO_VOLTAGE(voltages[i]);
            conv_volts.voltage1 = CONVERT_VALUE_TO_VOLTAGE(voltages[i + 1]);
            conv_volts.voltage2 = CONVERT_VALUE_TO_VOLTAGE(voltages[i + 2]);

            bms_voltages_conversion_to_raw_struct(&raw_volts, &conv_volts);

            int data_len = bms_voltages_pack(buffer, &raw_volts, BMS_VOLTAGES_BYTE_SIZE);
            if (data_len >= 0) {
                tx_header.DLC = data_len;
                _can_send(&BMS_CAN, buffer, &tx_header);
                HAL_Delay(1);
            }
        }
        return;
    }
    else if (id == BMS_VOLTAGES_INFO_FRAME_ID) {
        bms_voltages_info_t raw_volts = { 0 };
        bms_voltages_info_converted_t conv_volts = { 0 };
     
        conv_volts.cellboard_id = cellboard_index;
        conv_volts.min_voltage = CONVERT_VALUE_TO_VOLTAGE(volt_get_min());
        conv_volts.max_voltage = CONVERT_VALUE_TO_VOLTAGE(volt_get_max());
        conv_volts.avg_voltage = CONVERT_VALUE_TO_VOLTAGE(volt_get_avg());

        bms_voltages_info_conversion_to_raw_struct(&raw_volts, &conv_volts);

        int data_len = bms_voltages_info_pack(buffer, &raw_volts, BMS_VOLTAGES_INFO_BYTE_SIZE);
        if (data_len < 0)
            return;
        tx_header.DLC = data_len;
    }
    else if (id == BMS_CELLBOARD_VERSION_FRAME_ID) {
        bms_cellboard_version_t raw_version = { 0 };
        bms_cellboard_version_converted_t conv_version = { 0 };

        conv_version.canlib_build_time = CANLIB_BUILD_TIME;
        conv_version.cellboard_id = cellboard_index;
        conv_version.component_version = 1; // build_epoch

        bms_cellboard_version_conversion_to_raw_struct(&raw_version, &conv_version);

        int data_len = bms_cellboard_version_pack(buffer, &raw_version, BMS_CELLBOARD_VERSION_BYTE_SIZE);
        if (data_len < 0)
            return;
        tx_header.DLC = data_len;
    }
    else
        return;
    
    _can_send(&BMS_CAN, buffer, &tx_header);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef * hcan) {
    CAN_RxHeaderTypeDef rx_header = { 0 };
    uint8_t rx_data[CAN_MAX_PAYLOAD_LENGTH] = { 0 };

    // Check for communication errors
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        ERROR_SET(ERROR_CAN);
        return;
    }

    if (hcan->Instance == BMS_CAN.Instance) {
        // Reset can errors
        ERROR_UNSET(ERROR_CAN);

        if (rx_header.StdId == BMS_SET_BALANCING_STATUS_FRAME_ID) {
            bms_set_balancing_status_t raw_bal = { 0 };
            bms_set_balancing_status_converted_t conv_bal = { 0 };

            if (bms_set_balancing_status_unpack(&raw_bal, rx_data, BMS_SET_BALANCING_STATUS_BYTE_SIZE) < 0) {
                ERROR_SET(ERROR_CAN);
                return;
            }
            bms_set_balancing_status_raw_to_conversion_struct(&conv_bal, &raw_bal);

            // Set balancing parameters
            bal_params.target = conv_bal.target;
            bal_params.threshold = conv_bal.threshold;
            
            // Request for balancing status change
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
            bms_jmp_to_blt_t raw_jmp;
            bms_jmp_to_blt_converted_t conv_jmp;

            if (bms_jmp_to_blt_unpack(&raw_jmp, rx_data, BMS_JMP_TO_BLT_BYTE_SIZE) < 0) {
                ERROR_SET(ERROR_CAN);
                return;
            }
            bms_jmp_to_blt_raw_to_conversion_struct(&conv_jmp, &raw_jmp);

            if (conv_jmp.cellboard_id == cellboard_index)
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