/**
 * @file can_comm.c
 * @brief CAN bus serialization middleware
 *
 * @date Mar 1, 2021
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "can_comm.h"

#include "bal.h"
#include "bms_fsm.h"
#include "cli_bms.h"
#include "pack/current.h"
#include "pack/pack.h"
#include "mainboard_config.h"
#include "usart.h"
#include "bootloader.h"
#include "primary/primary_network.h"
#include "internal_voltage.h"
#include "cell_voltage.h"
#include "temperature.h"
#include "feedback.h"
#include "watchdog.h"
#include "fans_buzzer.h"
#include "soc.h"

#include <string.h>

#ifdef TEMP_GROUP_ERROR_ENABLE
uint16_t temp_errors[CELLBOARD_COUNT];
#endif // TEMP_GROUP_ERROR_ENABLE

uint32_t time_since_last_comm[CELLBOARD_COUNT];
bool can_forward;

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
        if(HAL_GetTick() - tick > timeout)
            return HAL_TIMEOUT;
    }
    return HAL_OK;
}

bool can_is_forwarding() {
    return can_forward;
}
void can_bms_init() {
    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of: 
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    CAN_FilterTypeDef filter = {
        .FilterActivation = CAN_FILTER_ENABLE,
        .FilterBank = 14,
        .FilterFIFOAssignment = CAN_FILTER_FIFO0,
        .FilterIdHigh = ((1U << 11) - 1) << 5, // Take all ids to 2^11 - 1
        .FilterIdLow = 0, // Take all ids from 0
        .FilterMaskIdHigh = 0,
        .FilterMaskIdLow = 0,
        .FilterMode = CAN_FILTERMODE_IDMASK,
        .FilterScale = CAN_FILTERSCALE_16BIT,
        .SlaveStartFilterBank = CAN_SLAVE_START_FILTER_BANK
    };

    // Enable filters and start CAN
    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);
    HAL_CAN_ActivateNotification(&BMS_CAN, CAN_IT_ERROR | CAN_IT_RX_FIFO0_MSG_PENDING);
    HAL_CAN_Start(&BMS_CAN);
}
void can_car_init() {
    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of: 
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    CAN_FilterTypeDef filter = {
        .FilterActivation = CAN_FILTER_ENABLE,
        .FilterBank = 0,
        .FilterFIFOAssignment = CAN_FILTER_FIFO1,
        .FilterIdHigh = ((1U << 11) - 1) << 5, // Take all ids to 2^11 - 1
        .FilterIdLow = 0, // Take all ids from 0
        .FilterMaskIdHigh = 0,
        .FilterMaskIdLow = 0,
        .FilterMode = CAN_FILTERMODE_IDMASK,
        .FilterScale = CAN_FILTERSCALE_16BIT,
        .SlaveStartFilterBank = CAN_SLAVE_START_FILTER_BANK
    };

    // Enable filters and start CAN
    HAL_CAN_ConfigFilter(&CAR_CAN, &filter);
    HAL_CAN_ActivateNotification(&CAR_CAN, CAN_IT_ERROR | CAN_IT_RX_FIFO1_MSG_PENDING);
    HAL_CAN_Start(&CAR_CAN);
}

HAL_StatusTypeDef can_send(CAN_HandleTypeDef * hcan, uint8_t * buffer, CAN_TxHeaderTypeDef * header) {
    // Wait for free mailboxes
    if(_can_wait(hcan, 3) != HAL_OK)
        return HAL_TIMEOUT;

    // Add message to a free mailbox
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, NULL);
    error_toggle_check(status != HAL_OK, ERROR_CAN, 0);

    return status;
}
HAL_StatusTypeDef can_car_send(uint16_t id) {
    // Return if busy
    if(can_forward && id != PRIMARY_HV_CAN_FORWARD_STATUS_FRAME_ID)
        return HAL_BUSY;

    CAN_TxHeaderTypeDef tx_header = {
        .DLC = 0,
        .ExtId = 0,
        .IDE = CAN_ID_STD,
        .RTR = CAN_RTR_DATA,
        .StdId = id,
        .TransmitGlobalTime = DISABLE
    };
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH] = { 0 };

    if (id == PRIMARY_HV_VOLTAGE_FRAME_ID) {
        primary_hv_voltage_t raw_volts = { 0 };
        primary_hv_voltage_converted_t conv_volts = { 0 };

        conv_volts.bus_voltage  = CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp());
        conv_volts.pack_voltage = CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_bat());
        conv_volts.max_cell_voltage = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_max());
        conv_volts.min_cell_voltage = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_min());

        primary_hv_voltage_conversion_to_raw_struct(&raw_volts, &conv_volts);

        int data_len = primary_hv_voltage_pack(buffer, &raw_volts, PRIMARY_HV_VOLTAGE_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_CURRENT_FRAME_ID) {
        primary_hv_current_t raw_curr = { 0 };
        primary_hv_current_converted_t conv_curr = { 0 };

        conv_curr.current = current_get_current();
        conv_curr.power = conv_curr.current * CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp());
        conv_curr.energy = 0;
        conv_curr.soc = 0;

        primary_hv_current_conversion_to_raw_struct(&raw_curr, &conv_curr);

        int data_len = primary_hv_current_pack(buffer, &raw_curr, PRIMARY_HV_CURRENT_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_TS_STATUS_FRAME_ID) {
        primary_ts_status_t raw_status = { 0 };
        primary_ts_status_converted_t conv_status = { 0 };

        switch (fsm_get_state()) {
            case STATE_INIT:
                conv_status.ts_status = primary_ts_status_ts_status_INIT;
                break;
            case STATE_IDLE:
                conv_status.ts_status = primary_ts_status_ts_status_IDLE;
                break;
            case STATE_WAIT_AIRN_CLOSE:
                conv_status.ts_status = primary_ts_status_ts_status_AIRN_CLOSE;
                break;
            case STATE_WAIT_TS_PRECHARGE:
                conv_status.ts_status = primary_ts_status_ts_status_PRECHARGE;
                break;
            case STATE_WAIT_AIRP_CLOSE:
                conv_status.ts_status = primary_ts_status_ts_status_AIRP_CLOSE;
                break;
            case STATE_TS_ON:
                conv_status.ts_status = primary_ts_status_ts_status_TS_ON;
                break;
            case STATE_FATAL_ERROR:
                conv_status.ts_status = primary_ts_status_ts_status_FATAL_ERROR;
                break;
            default:
                conv_status.ts_status = primary_ts_status_ts_status_IDLE;
                break;
        }

        primary_ts_status_conversion_to_raw_struct(&raw_status, &conv_status);
        
        int data_len = primary_ts_status_pack(buffer, &raw_status, PRIMARY_TS_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_TEMP_FRAME_ID) {
        primary_hv_temp_t raw_temp = { 0 };
        primary_hv_temp_converted_t conv_temp = { 0 };
        
        conv_temp.average_temp = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_average());
        conv_temp.min_temp = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_min());
        conv_temp.max_temp = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_max());

        primary_hv_temp_conversion_to_raw_struct(&raw_temp, &conv_temp);

        int data_len = primary_hv_temp_pack(buffer, &raw_temp, PRIMARY_HV_TEMP_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_ERRORS_FRAME_ID) {
        primary_hv_errors_t raw_errors = { 0 };
        primary_hv_errors_converted_t conv_errors  = { 0 };

        error_t errors[100] = { 0 };
        error_dump(errors);

        for(size_t i = 0; i < error_count(); ++i) {
            switch(errors[i].id) {
                case ERROR_CELL_LOW_VOLTAGE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_low_voltage = 1;
                    else
                        conv_errors.errors_cell_low_voltage = 1;
                    break;
                case ERROR_CELL_UNDER_VOLTAGE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_under_voltage = 1;
                    else
                        conv_errors.errors_cell_under_voltage = 1;
                    break;
                case ERROR_CELL_OVER_VOLTAGE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_over_voltage = 1;
                    else
                        conv_errors.errors_cell_over_voltage = 1;
                    break;
                case ERROR_CELL_OVER_TEMPERATURE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_over_temperature = 1;
                    else
                        conv_errors.errors_cell_over_temperature = 1;
                    break;
                case ERROR_CELL_HIGH_TEMPERATURE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cell_high_temperature = 1;
                    else
                        conv_errors.errors_cell_high_temperature = 1;
                    break;
                case ERROR_OVER_CURRENT:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_over_current = 1;
                    else
                        conv_errors.errors_over_current = 1;
                    break;
                case ERROR_CAN:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_can = 1;
                    else
                        conv_errors.errors_can = 1;
                    break;
                case ERROR_INT_VOLTAGE_MISMATCH:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_int_voltage_mismatch = 1;
                    else
                        conv_errors.errors_int_voltage_mismatch = 1;
                    break;
                case ERROR_CELLBOARD_COMM:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cellboard_comm = 1;
                    else
                        conv_errors.errors_cellboard_comm = 1;
                    break;
                case ERROR_CELLBOARD_INTERNAL:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_cellboard_internal = 1;
                    else
                        conv_errors.errors_cellboard_internal = 1;
                    break;
                case ERROR_CONNECTOR_DISCONNECTED:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_connector_disconnected = 1;
                    else
                        conv_errors.errors_connector_disconnected = 1;
                    break;
                case ERROR_FANS_DISCONNECTED:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_fans_disconnected = 1;
                    else
                        conv_errors.errors_fans_disconnected = 1;
                    break;
                case ERROR_FEEDBACK:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_feedback = 1;
                    else
                        conv_errors.errors_feedback = 1;
                    break;
                case ERROR_FEEDBACK_CIRCUITRY:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_feedback_circuitry = 1;
                    else
                        conv_errors.errors_feedback_circuitry = 1;
                    break;
                case ERROR_EEPROM_COMM:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_eeprom_comm = 1;
                    else
                        conv_errors.errors_eeprom_comm = 1;
                    break;
                case ERROR_EEPROM_WRITE:
                    if (errors[i].state == STATE_WARNING)
                        conv_errors.warnings_eeprom_write = 1;
                    else
                        conv_errors.errors_eeprom_write = 1;
                    break;
                default:
                    break;
            }
        }

        primary_hv_errors_conversion_to_raw_struct(&raw_errors, &conv_errors);

        int data_len = primary_hv_errors_pack(buffer, &raw_errors, PRIMARY_HV_ERRORS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_CAN_FORWARD_STATUS_FRAME_ID) {
        primary_hv_can_forward_status_t raw_can_forward = { 0 };
        primary_hv_can_forward_status_converted_t conv_can_forward = { 0 };

        conv_can_forward.can_forward_status = (can_forward) ?
            primary_hv_can_forward_status_can_forward_status_ON :
            primary_hv_can_forward_status_can_forward_status_OFF;

        primary_hv_can_forward_status_conversion_to_raw_struct(&raw_can_forward, &conv_can_forward);

        int data_len = primary_hv_can_forward_status_pack(buffer, &raw_can_forward, PRIMARY_HV_CAN_FORWARD_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_VERSION_FRAME_ID) {
        primary_hv_version_t raw_version = { 0 };
        primary_hv_version_converted_t conv_version = { 0 };

        conv_version.canlib_build_time = CANLIB_BUILD_TIME;
        conv_version.component_version = 1;

        primary_hv_version_conversion_to_raw_struct(&raw_version, &conv_version);

        int data_len = primary_hv_version_pack(buffer, &raw_version, PRIMARY_HV_VERSION_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID) {
        primary_hv_feedbacks_status_t raw_status = { 0 };
        primary_hv_feedbacks_status_converted_t conv_status = { 0 };

        // Get feedbacks status
        feedback_feed_t fbs[FEEDBACK_N] = { 0 };
        feedback_get_all_states(fbs);
        
        // TODO: Set feedback status (is_circuitry)
        for (size_t i = 0; i < FEEDBACK_N; i++) {
            switch(i) {
                case FEEDBACK_IMPLAUSIBILITY_DETECTED_POS:
                    conv_status.feedbacks_status_feedback_implausibility_detected = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_IMD_COCKPIT_POS:
                    conv_status.feedbacks_status_feedback_imd_cockpit = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS:
                    conv_status.feedbacks_status_feedback_tsal_green_fault_latched = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_BMS_COCKPIT_POS:
                    conv_status.feedbacks_status_feedback_bms_cockpit = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_EXT_LATCHED_POS:
                    conv_status.feedbacks_status_feedback_ext_latched = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_TSAL_GREEN_POS:
                    conv_status.feedbacks_status_feedback_tsal_green = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_TS_OVER_60V_STATUS_POS:
                    conv_status.feedbacks_status_feedback_ts_over_60v_status = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_AIRN_STATUS_POS:
                    conv_status.feedbacks_status_feedback_airn_status = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_AIRP_STATUS_POS:
                    conv_status.feedbacks_status_feedback_airp_status = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_AIRP_GATE_POS:
                    conv_status.feedbacks_status_feedback_airp_gate = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_AIRN_GATE_POS:
                    conv_status.feedbacks_status_feedback_airn_gate = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_PRECHARGE_STATUS_POS:
                    conv_status.feedbacks_status_feedback_precharge_status = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_TSP_OVER_60V_STATUS_POS:
                    conv_status.feedbacks_status_feedback_tsp_over_60v_status = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_IMD_FAULT_POS:
                    conv_status.feedbacks_status_feedback_imd_fault = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_CHECK_MUX_POS:
                    conv_status.feedbacks_status_feedback_check_mux = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_SD_END_POS:
                    conv_status.feedbacks_status_feedback_sd_end = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_SD_OUT_POS:
                    conv_status.feedbacks_status_feedback_sd_out = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_SD_IN_POS:
                    conv_status.feedbacks_status_feedback_sd_in = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_SD_BMS_POS:
                    conv_status.feedbacks_status_feedback_sd_bms = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
                case FEEDBACK_SD_IMD_POS:
                    conv_status.feedbacks_status_feedback_sd_imd = fbs[i].real_state == FEEDBACK_STATE_H;
                    break;
            }
        }

        primary_hv_feedbacks_status_conversion_to_raw_struct(&raw_status, &conv_status);

        int data_len = primary_hv_feedbacks_status_pack(buffer, &raw_status, PRIMARY_HV_FEEDBACKS_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_FANS_OVERRIDE_STATUS_FRAME_ID) {
        primary_hv_fans_override_status_t raw_fans = { 0 };
        primary_hv_fans_override_status_converted_t conv_fans = { 0 };

        conv_fans.fans_override = fans_is_overrided();
        conv_fans.fans_speed = fans_get_speed();

        primary_hv_fans_override_status_conversion_to_raw_struct(&raw_fans, &conv_fans);

        int data_len = primary_hv_fans_override_status_pack(buffer, &raw_fans, PRIMARY_HV_FANS_OVERRIDE_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else
        return HAL_ERROR;

    return can_send(&CAR_CAN, buffer, &tx_header);
}
HAL_StatusTypeDef can_bms_send(uint16_t id) {
    // Return if busy
    if(can_forward && id != BMS_JMP_TO_BLT_FRAME_ID)
        return HAL_BUSY;

    CAN_TxHeaderTypeDef tx_header = {
        .DLC = 0,
        .ExtId = 0,
        .IDE = CAN_ID_STD,
        .RTR = CAN_RTR_DATA,
        .StdId = id,
        .TransmitGlobalTime = DISABLE
    };
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH] = { 0 };
    
    if (id == BMS_SET_BALANCING_STATUS_FRAME_ID) {
        size_t errors = 0;

        bool bal_status = bal_need_balancing();
        uint16_t target = MAX(CELL_MIN_VOLTAGE, cell_voltage_get_min());
        uint16_t threshold = MIN(BAL_MAX_VOLTAGE_THRESHOLD, bal_get_threshold());

        for (size_t i = 0; i < CELLBOARD_COUNT; ++i) {
            bms_set_balancing_status_t raw_bal = { 0 };
            bms_set_balancing_status_converted_t conv_bal = { 0 };
            
            conv_bal.balancing_status = bal_status;
            conv_bal.target = target;
            conv_bal.threshold = threshold;

            bms_set_balancing_status_conversion_to_raw_struct(&raw_bal, &conv_bal);

            int data_len = bms_set_balancing_status_pack(buffer, &raw_bal, BMS_SET_BALANCING_STATUS_BYTE_SIZE);
            if (data_len < 0)
                ++errors;
            else {
                tx_header.DLC = data_len;
                if (can_send(&BMS_CAN, buffer, &tx_header) != HAL_OK)
                    ++errors;
            }
        }

        return errors == 0 ? HAL_OK : HAL_ERROR;
    }
    else if (id == BMS_JMP_TO_BLT_FRAME_ID) {
        bms_jmp_to_blt_t raw_jmp = { 0 };
        bms_jmp_to_blt_converted_t conv_jmp = { 0 };

        // TODO: Set cellboard id and board index (not used in cellboard)
        conv_jmp.cellboard_id = 0;
        conv_jmp.board_index = 0;

        // Convert fw update to raw
        bms_jmp_to_blt_conversion_to_raw_struct(&raw_jmp, &conv_jmp);

        int data_len = bms_jmp_to_blt_pack(buffer, &raw_jmp, BMS_JMP_TO_BLT_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        
        tx_header.DLC = data_len;
    }
    else
        return HAL_ERROR;

    return can_send(&BMS_CAN, buffer, &tx_header);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef * hcan) {
    CAN_RxHeaderTypeDef rx_header = { 0 };
    uint8_t rx_data[CAN_MAX_PAYLOAD_LENGTH] = { 0 };

    // Check for communication errors
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        error_set(ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == BMS_CAN.Instance) {
        // Reset can errors
        error_reset(ERROR_CAN, 1);

        // Forward data to the cellboards
        if (can_forward && (rx_header.StdId >= BMS_FLASH_CELLBOARD_0_TX_FRAME_ID && rx_header.StdId <= BMS_FLASH_CELLBOARD_5_RX_FRAME_ID)) {
            CAN_TxHeaderTypeDef tx_header = {
                .DLC = rx_header.DLC,
                .ExtId = 0,
                .IDE = CAN_ID_STD,
                .RTR = CAN_RTR_DATA,
                .StdId = rx_header.StdId,
                .TransmitGlobalTime = DISABLE
            };
            can_send(&CAR_CAN, rx_data, &tx_header);
            return;
        }
        else if (rx_header.StdId == BMS_VOLTAGES_FRAME_ID) {
            bms_voltages_t raw_volts = { 0 };
            bms_voltages_converted_t conv_volts = { 0 };

            if (bms_voltages_unpack(&raw_volts, rx_data, BMS_VOLTAGES_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            bms_voltages_raw_to_conversion_struct(&conv_volts, &raw_volts);
            
            // Reset time since last communication
            time_since_last_comm[conv_volts.cellboard_id] = HAL_GetTick();

            // Forward data
            CAN_TxHeaderTypeDef tx_header = {
                .DLC = 0,
                .ExtId = 0,
                .IDE = CAN_ID_STD,
                .RTR = CAN_RTR_DATA,
                .StdId = PRIMARY_HV_CELLS_VOLTAGE_FRAME_ID,
                .TransmitGlobalTime = DISABLE
            };
            uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH] = { 0 };
            primary_hv_cells_voltage_t raw_fwd_volts = { 0 };
            primary_hv_cells_voltage_converted_t conv_fwd_volts = { 0 };

            // Set start_index of the received cells between all cells of the pack
            conv_fwd_volts.start_index = conv_volts.cellboard_id * CELLBOARD_CELL_COUNT + conv_volts.start_index;
            conv_fwd_volts.voltage_0 = conv_volts.voltage0;
            conv_fwd_volts.voltage_1 = conv_volts.voltage1;
            conv_fwd_volts.voltage_2 = conv_volts.voltage2;

            primary_hv_cells_voltage_conversion_to_raw_struct(&raw_fwd_volts, &conv_fwd_volts);

            int data_len = primary_hv_cells_voltage_pack(buffer, &raw_fwd_volts, PRIMARY_HV_CELLS_VOLTAGE_BYTE_SIZE);
            if (data_len < 0)
                return;
            tx_header.DLC = data_len;

            can_send(&CAR_CAN, buffer, &tx_header);
        }
        else if (rx_header.StdId == BMS_VOLTAGES_INFO_FRAME_ID) {
            bms_voltages_info_t raw_volts = { 0 };
            bms_voltages_info_converted_t conv_volts = { 0 };

            if (bms_voltages_info_unpack(&raw_volts, rx_data, BMS_VOLTAGES_INFO_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            bms_voltages_info_raw_to_conversion_struct(&conv_volts, &raw_volts);

            // Reset time since last communication
            time_since_last_comm[conv_volts.cellboard_id] = HAL_GetTick();

            cell_voltage_set_cells(
                conv_volts.cellboard_id,
                CONVERT_VOLTAGE_TO_VALUE(conv_volts.min_voltage),
                CONVERT_VOLTAGE_TO_VALUE(conv_volts.max_voltage),
                CONVERT_VOLTAGE_TO_VALUE(conv_volts.avg_voltage));
        }
        else if (rx_header.StdId == BMS_TEMPERATURES_FRAME_ID) {
            bms_temperatures_t raw_temps = { 0 };
            bms_temperatures_converted_t conv_temps = { 0 };
            
            if (bms_temperatures_unpack(&raw_temps, rx_data, BMS_TEMPERATURES_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            bms_temperatures_raw_to_conversion_struct(&conv_temps, &raw_temps);

            // Reset time since last communication
            time_since_last_comm[conv_temps.cellboard_id] = HAL_GetTick();

            // TODO: Test
#if defined(TEMP_GROUP_ERROR_ENABLE) && defined(TEMP_ERROR_ENABLE)
            // Add error bit to the temp group
            size_t index = conv_temps.start_index / 4;
            size_t bit = index / 2 * 3;
            if (index % 2) {
                ++bit;
                if (conv_temps.temp0 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    conv_temps.temp1 <= CELL_MIN_TEMPERATURE + 0.01f)
                    temp_errors[conv_temps.cellboard_id] |= 1 << bit;
                else
                    temp_errors[conv_temps.cellboard_id] &= ~(1 << bit);

                ++bit;
                if (conv_temps.temp2 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    conv_temps.temp3 <= CELL_MIN_TEMPERATURE + 0.01f)
                    temp_errors[conv_temps.cellboard_id] |= 1 << bit;
                else
                    temp_errors[conv_temps.cellboard_id] &= ~(1 << bit);
            }
            else {
                if (conv_temps.temp0 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    conv_temps.temp1 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    conv_temps.temp2 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    conv_temps.temp3 <= CELL_MIN_TEMPERATURE + 0.01f)
                    temp_errors[conv_temps.cellboard_id] |= 1 << bit;
                else
                    temp_errors[conv_temps.cellboard_id] &= ~(1 << bit);
            }
            // Toggle temperatures connectors error
            size_t temp_id = 0;
            size_t board_id = 0;
            bool is_error_set = false;
            for (; board_id < CELLBOARD_COUNT && !is_error_set; board_id++) {
                for (; temp_id < TEMP_STRIPS_PER_BUS; temp_id++) {
                    if ((temp_errors[board_id] & (1 << bit)) && (temp_errors[conv_temps.cellboard_id] & (1 << (bit + 1)))) {
                        error_set(ERROR_CONNECTOR_DISCONNECTED, 2, HAL_GetTick());
                        is_error_set = true;
                        break;
                    }
                }
            }
            if (!is_error_set)
                error_reset(ERROR_CONNECTOR_DISCONNECTED, 2);
#endif // TEMP_GROUP_ERROR_ENABLE



            // Forward data
            CAN_TxHeaderTypeDef tx_header = {
                .DLC = 0,
                .ExtId = 0,
                .IDE = CAN_ID_STD,
                .RTR = CAN_RTR_DATA,
                .StdId = PRIMARY_HV_CELLS_TEMP_FRAME_ID,
                .TransmitGlobalTime = DISABLE
            };
            uint8_t buffer[8] = { 0 };
            primary_hv_cells_temp_t raw_fwd_temps = { 0 };
            primary_hv_cells_temp_converted_t conv_fwd_temps = { 0 };

            // Set start_index of the received cells between all cells of the pack
            conv_fwd_temps.start_index = conv_temps.cellboard_id * TEMP_SENSOR_COUNT + conv_temps.start_index;
            conv_fwd_temps.temp_0 = conv_temps.temp0;
            conv_fwd_temps.temp_1 = conv_temps.temp1;
            conv_fwd_temps.temp_2 = conv_temps.temp2;
            conv_fwd_temps.temp_3 = conv_temps.temp3;

            primary_hv_cells_temp_conversion_to_raw_struct(&raw_fwd_temps, &conv_fwd_temps);

            int data_len = primary_hv_cells_temp_pack(buffer, &raw_fwd_temps, PRIMARY_HV_CELLS_TEMP_BYTE_SIZE);
            if (data_len < 0)
                return;
            tx_header.DLC = data_len;

            can_send(&CAR_CAN, buffer, &tx_header);
        }
        else if (rx_header.StdId == BMS_TEMPERATURES_INFO_FRAME_ID) {
            bms_temperatures_info_t raw_temps = { 0 };
            bms_temperatures_info_converted_t conv_temps = { 0 };

            if (bms_temperatures_info_unpack(&raw_temps, rx_data, BMS_TEMPERATURES_INFO_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            bms_temperatures_info_raw_to_conversion_struct(&conv_temps, &raw_temps);

            // Reset time since last communication
            time_since_last_comm[conv_temps.cellboard_id] = HAL_GetTick();

            temperature_set_cells(
                conv_temps.cellboard_id,
                CONVERT_TEMPERATURE_TO_VALUE(conv_temps.min_temp),
                CONVERT_TEMPERATURE_TO_VALUE(conv_temps.max_temp),
                CONVERT_TEMPERATURE_TO_VALUE(conv_temps.avg_temp));
        }
        else if (rx_header.StdId == BMS_BOARD_STATUS_FRAME_ID) {
            // Reset the watchdog timer
            watchdog_reset(rx_header.StdId);
            
            bms_board_status_t raw_status = { 0 };
            bms_board_status_converted_t conv_status = { 0 };

            if (bms_board_status_unpack(&raw_status, rx_data, BMS_BOARD_STATUS_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            bms_board_status_raw_to_conversion_struct(&conv_status, &raw_status);
            
            // Reset time since last communication
            time_since_last_comm[conv_status.cellboard_id] = HAL_GetTick();

            bal_set_is_balancing(conv_status.cellboard_id, conv_status.balancing_status);

            // Check cellboard errors
            uint32_t error_status = conv_status.errors_can_comm |
                conv_status.errors_ltc_comm |
                conv_status.errors_open_wire |
                conv_status.errors_temp_comm_0 | 
                conv_status.errors_temp_comm_1 | 
                conv_status.errors_temp_comm_2 | 
                conv_status.errors_temp_comm_3 |
                conv_status.errors_temp_comm_4 |
                conv_status.errors_temp_comm_5;
            error_toggle_check(error_status != 0, ERROR_CELLBOARD_INTERNAL, conv_status.cellboard_id);

            // Forward data
            CAN_TxHeaderTypeDef tx_header = {
                .DLC = 0,
                .ExtId = 0,
                .IDE = CAN_ID_STD,
                .RTR = CAN_RTR_DATA,
                .StdId = PRIMARY_HV_CELL_BALANCING_STATUS_FRAME_ID,
                .TransmitGlobalTime = DISABLE
            };
            uint8_t buffer[8] = { 0 };
            primary_hv_cell_balancing_status_t raw_fwd_status = { 0 };
            primary_hv_cell_balancing_status_converted_t conv_fwd_status = { 0 };

            conv_fwd_status.cellboard_id = conv_status.cellboard_id;
            conv_fwd_status.balancing_status = conv_status.balancing_status;

            conv_fwd_status.errors_can_comm = conv_status.errors_can_comm;
            conv_fwd_status.errors_ltc_comm = conv_status.errors_ltc_comm;
            conv_fwd_status.errors_open_wire = conv_status.errors_open_wire;
            conv_fwd_status.errors_temp_comm_0 = conv_status.errors_temp_comm_0;
            conv_fwd_status.errors_temp_comm_1 = conv_status.errors_temp_comm_1;
            conv_fwd_status.errors_temp_comm_2 = conv_status.errors_temp_comm_2;
            conv_fwd_status.errors_temp_comm_3 = conv_status.errors_temp_comm_3;
            conv_fwd_status.errors_temp_comm_4 = conv_status.errors_temp_comm_4;
            conv_fwd_status.errors_temp_comm_5 = conv_status.errors_temp_comm_5;

            conv_fwd_status.balancing_cells_cell0 = conv_status.balancing_cells_cell0;
            conv_fwd_status.balancing_cells_cell1 = conv_status.balancing_cells_cell1;
            conv_fwd_status.balancing_cells_cell2 = conv_status.balancing_cells_cell2;
            conv_fwd_status.balancing_cells_cell3 = conv_status.balancing_cells_cell3;
            conv_fwd_status.balancing_cells_cell4 = conv_status.balancing_cells_cell4;
            conv_fwd_status.balancing_cells_cell5 = conv_status.balancing_cells_cell5;
            conv_fwd_status.balancing_cells_cell6 = conv_status.balancing_cells_cell6;
            conv_fwd_status.balancing_cells_cell7 = conv_status.balancing_cells_cell7;
            conv_fwd_status.balancing_cells_cell8 = conv_status.balancing_cells_cell8;
            conv_fwd_status.balancing_cells_cell9 = conv_status.balancing_cells_cell9;
            conv_fwd_status.balancing_cells_cell10 = conv_status.balancing_cells_cell10;
            conv_fwd_status.balancing_cells_cell11 = conv_status.balancing_cells_cell11;
            conv_fwd_status.balancing_cells_cell12 = conv_status.balancing_cells_cell12;
            conv_fwd_status.balancing_cells_cell13 = conv_status.balancing_cells_cell13;
            conv_fwd_status.balancing_cells_cell14 = conv_status.balancing_cells_cell14;
            conv_fwd_status.balancing_cells_cell15 = conv_status.balancing_cells_cell15;
            conv_fwd_status.balancing_cells_cell16 = conv_status.balancing_cells_cell16;
            conv_fwd_status.balancing_cells_cell17 = conv_status.balancing_cells_cell17;

            primary_hv_cell_balancing_status_conversion_to_raw_struct(&raw_fwd_status, &conv_fwd_status);

            int data_len = primary_hv_cell_balancing_status_pack(buffer, &raw_fwd_status, PRIMARY_HV_CELL_BALANCING_STATUS_BYTE_SIZE);
            if (data_len < 0)
                return;
            tx_header.DLC = data_len;

            can_send(&CAR_CAN, buffer, &tx_header);
        }
    }
}
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rx_header = { 0 };
    uint8_t rx_data[CAN_MAX_PAYLOAD_LENGTH] = { 0 };

    // Check for communication errors
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rx_header, rx_data) != HAL_OK) {
        error_set(ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == CAR_CAN.Instance) {
        error_reset(ERROR_CAN, 1);

        if (can_forward && (rx_header.StdId >= BMS_FLASH_CELLBOARD_0_TX_FRAME_ID && rx_header.StdId <= BMS_FLASH_CELLBOARD_5_RX_FRAME_ID)) {
            CAN_TxHeaderTypeDef tx_header = {
                .DLC = rx_header.DLC,
                .ExtId = 0,
                .IDE = CAN_ID_STD,
                .RTR = CAN_RTR_DATA,
                .StdId = rx_header.StdId,
                .TransmitGlobalTime = DISABLE
            };
            can_send(&BMS_CAN, rx_data, &tx_header);
            return;
        }
        else if (rx_header.StdId == PRIMARY_CAR_STATUS_FRAME_ID) {
            // Reset the watchdog timer
            watchdog_reset(rx_header.StdId);
        }
        else if (rx_header.StdId == PRIMARY_SET_TS_STATUS_DAS_FRAME_ID || rx_header.StdId == PRIMARY_SET_TS_STATUS_HANDCART_FRAME_ID) {
            primary_set_ts_status_das_t raw_ts_status = { 0 };
            primary_set_ts_status_das_converted_t conv_ts_status = { 0 };

            if (primary_set_ts_status_das_unpack(&raw_ts_status, rx_data, PRIMARY_SET_TS_STATUS_DAS_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_set_ts_status_das_raw_to_conversion_struct(&conv_ts_status, &raw_ts_status);

            // Request for TS status change
            switch (conv_ts_status.ts_status_set) {
                case primary_set_ts_status_das_ts_status_set_OFF:
                    set_ts_request.is_new = true;
                    set_ts_request.next_state = STATE_IDLE;
                    break;
                case primary_set_ts_status_das_ts_status_set_ON:
                    set_ts_request.is_new = true;
                    set_ts_request.next_state = STATE_WAIT_AIRN_CLOSE;
                    break;
            }
        }
        else if (rx_header.StdId == PRIMARY_SET_CELL_BALANCING_STATUS_FRAME_ID) {
            primary_set_cell_balancing_status_t raw_bal_status = { 0 };
            primary_set_cell_balancing_status_converted_t conv_bal_status = { 0 };

            if (primary_set_cell_balancing_status_unpack(&raw_bal_status, rx_data, PRIMARY_SET_CELL_BALANCING_STATUS_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_set_cell_balancing_status_raw_to_conversion_struct(&conv_bal_status, &raw_bal_status);
            
            // Request for balancing start or stop
            switch(conv_bal_status.set_balancing_status) {
                case PRIMARY_SET_CELL_BALANCING_STATUS_SET_BALANCING_STATUS_ON_CHOICE:
                    bal_start();
                    break;
                case PRIMARY_SET_CELL_BALANCING_STATUS_SET_BALANCING_STATUS_OFF_CHOICE:
                    bal_stop();
                    break;
            }
        }
        else if (rx_header.StdId == PRIMARY_HANDCART_STATUS_FRAME_ID) {
            primary_handcart_status_t raw_handcart_status = { 0 };
            primary_handcart_status_converted_t conv_handcart_status = { 0 };

            if (primary_handcart_status_unpack(&raw_handcart_status, rx_data, PRIMARY_HANDCART_STATUS_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_handcart_status_raw_to_conversion_struct(&conv_handcart_status, &raw_handcart_status);

            // Reset watchdog
            watchdog_reset(PRIMARY_HANDCART_STATUS_FRAME_ID);

            is_handcart_connected = conv_handcart_status.connected;
        }
        else if (rx_header.StdId == PRIMARY_HV_CAN_FORWARD_FRAME_ID) {
            bms_state_t fsm_state = fsm_get_state();
            if (fsm_state != STATE_INIT && fsm_state != STATE_IDLE && fsm_state != STATE_FATAL_ERROR) {
                can_forward = 0;
                return;
            }
            
            primary_hv_can_forward_t raw_can_forward = { 0 };
            primary_hv_can_forward_converted_t conv_can_forward = { 0 };

            if (primary_hv_can_forward_unpack(&raw_can_forward, rx_data, PRIMARY_HV_CAN_FORWARD_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_hv_can_forward_raw_to_conversion_struct(&conv_can_forward, &raw_can_forward);

            // Set can forward status
            switch (conv_can_forward.can_forward_set) {
                case primary_hv_can_forward_can_forward_set_OFF:
                    can_forward = 0;
                    break;
                case primary_hv_can_forward_can_forward_set_ON:
                    can_bms_send(BMS_JMP_TO_BLT_FRAME_ID);
                    can_forward = 1;
                    break;
            }
        }
        else if (rx_header.StdId == PRIMARY_HV_FANS_OVERRIDE_FRAME_ID) {
            primary_hv_fans_override_t raw_fans = { 0 };
            primary_hv_fans_override_converted_t conv_fans = { 0 };

            if (primary_hv_fans_override_unpack(&raw_fans, rx_data, PRIMARY_HV_FANS_OVERRIDE_BYTE_SIZE) < 0) {
                error_set(ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_hv_fans_override_raw_to_conversion_struct(&conv_fans, &raw_fans);

            // Set fans override and speed
            fans_set_override(conv_fans.fans_override);
            fans_set_speed(conv_fans.fans_speed);
        }
        else {
            bms_state_t state = fsm_get_state();
            if (rx_header.StdId == PRIMARY_BMS_HV_JMP_TO_BLT_FRAME_ID && (state == STATE_INIT || state == STATE_IDLE || state == STATE_FATAL_ERROR) && !bal_is_balancing())
                HAL_NVIC_SystemReset();
        }
    }
}

void CAN_change_bitrate(CAN_HandleTypeDef *hcan, CAN_Bitrate bitrate) {
    /* De initialize CAN*/
    HAL_CAN_DeInit(hcan);
    switch (bitrate) {
        case CAN_BITRATE_1MBIT:
            hcan->Init.Prescaler = CAN_1MBIT_PRE;
            hcan->Init.TimeSeg1  = CAN_1MBIT_BS1;
            hcan->Init.TimeSeg2  = CAN_1MBIT_BS2;
            break;
        case CAN_BITRATE_125KBIT:
            hcan->Init.Prescaler = CAN_125KBIT_PRE;
            hcan->Init.TimeSeg1  = CAN_125KBIT_BS1;
            hcan->Init.TimeSeg2  = CAN_125KBIT_BS2;
            break;
    }
    if (HAL_CAN_Init(hcan) != HAL_OK) {
        Error_Handler();
    }
    if (hcan->Instance == BMS_CAN.Instance)
        can_bms_init();
    else if (hcan->Instance == CAR_CAN.Instance)
        can_car_init();
}

void can_cellboards_check() {
    for (size_t i = 0; i < CELLBOARD_COUNT; i++) {
        if (time_since_last_comm[i] > 0) {
            error_toggle_check(HAL_GetTick() - time_since_last_comm[i] >= CELLBOARD_COMM_TIMEOUT, ERROR_CELLBOARD_COMM, i);
        }
    }
}