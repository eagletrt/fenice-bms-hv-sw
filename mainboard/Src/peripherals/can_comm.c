/**
 * @file can_comm.c
 * @brief CAN bus serialization middleware
 *
 * @date Mar 1, 2021
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "can_comm.h"

#include <string.h>
#include <math.h>
#include <time.h>

#include "bal.h"
#include "bms_fsm.h"
#include "cli_bms.h"
#include "pack/current.h"
#include "pack/pack.h"
#include "mainboard_config.h"
#include "usart.h"
#include "bootloader.h"
#include "primary_network.h"
#include "internal_voltage.h"
#include "cell_voltage.h"
#include "temperature.h"
#include "feedback.h"
#include "watchdog.h"
#include "fans_buzzer.h"
#include "soc.h"
#include "imd.h"
#include "error/error-handler.h"

#ifdef TEMP_GROUP_ERROR_ENABLE
uint16_t temp_errors[CELLBOARD_COUNT];
#endif // TEMP_GROUP_ERROR_ENABLE

uint32_t time_since_last_comm[CELLBOARD_COUNT];
bool can_forward;
uint8_t flash_cellboard_id;
float debug_signal;

primary_hv_debug_signals_converted_t conv_debug;

static time_t build_epoch;

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
    struct tm tm;
    if (strptime(__DATE__" "__TIME__, "%b %d %Y %H:%M:%S", &tm) != NULL)
        build_epoch = mktime(&tm);

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
    // Initialize build time

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

    uint32_t mailbox = 0;
    // if (hcan == &CAR_CAN && header->StdId == 0)
    //     return HAL_OK;

    // Add message to a free mailbox
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, &mailbox);
    ERROR_TOGGLE_IF(status != HAL_OK, ERROR_GROUP_ERROR_CAN, 0, HAL_GetTick());

    return status;
}
HAL_StatusTypeDef can_car_send(uint16_t id) {
    // Return if busy
    if(can_forward) // && id != PRIMARY_HV_CAN_FORWARD_STATUS_FRAME_ID)
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

    if (id == PRIMARY_HV_TOTAL_VOLTAGE_FRAME_ID) {
        primary_hv_total_voltage_t raw_volts = { 0 };
        primary_hv_total_voltage_converted_t conv_volts = { 0 };

        conv_volts.bus = CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp());
        conv_volts.pack = CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_bat());
        conv_volts.sum_cell = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_sum());

        primary_hv_total_voltage_conversion_to_raw_struct(&raw_volts, &conv_volts);

        int data_len = primary_hv_total_voltage_pack(buffer, &raw_volts, PRIMARY_HV_TOTAL_VOLTAGE_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_CELLS_VOLTAGE_STATS_FRAME_ID) {
        primary_hv_cells_voltage_stats_t raw_volts = { 0 };
        primary_hv_cells_voltage_stats_converted_t conv_volts = { 0 };

        conv_volts.max = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_max());
        conv_volts.min = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_min());
        conv_volts.avg = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_avg());
        conv_volts.delta = conv_volts.max - conv_volts.min;

        primary_hv_cells_voltage_stats_conversion_to_raw_struct(&raw_volts, &conv_volts);

        int data_len = primary_hv_cells_voltage_stats_pack(buffer, &raw_volts, PRIMARY_HV_CELLS_VOLTAGE_STATS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_CURRENT_FRAME_ID) {
        primary_hv_current_t raw_curr = { 0 };
        primary_hv_current_converted_t conv_curr = { 0 };

        conv_curr.current = current_get_current();

        primary_hv_current_conversion_to_raw_struct(&raw_curr, &conv_curr);

        int data_len = primary_hv_current_pack(buffer, &raw_curr, PRIMARY_HV_CURRENT_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_POWER_FRAME_ID) {
        primary_hv_power_t raw_pow = { 0 };
        primary_hv_power_converted_t conv_pow = { 0 };

        conv_pow.power = (current_get_current() * CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp())) / 1000.f;

        primary_hv_power_conversion_to_raw_struct(&raw_pow, &conv_pow);

        int data_len = primary_hv_power_pack(buffer, &raw_pow, PRIMARY_HV_POWER_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_ENERGY_FRAME_ID) {
        primary_hv_energy_t raw_energy = { 0 };
        primary_hv_energy_converted_t conv_energy = { 0 };

        // TODO: Add energy? (old code)
        conv_energy.energy = 0;

        primary_hv_energy_conversion_to_raw_struct(&raw_energy, &conv_energy);

        int data_len = primary_hv_energy_pack(buffer, &raw_energy, PRIMARY_HV_ENERGY_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_SOC_FRAME_ID) {
        primary_hv_soc_t raw_soc = { 0 };
        primary_hv_soc_converted_t conv_soc = { 0 };

        // TODO: Add soc
        conv_soc.soc = 0;

        primary_hv_soc_conversion_to_raw_struct(&raw_soc, &conv_soc);

        int data_len = primary_hv_soc_pack(buffer, &raw_soc, PRIMARY_HV_SOC_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_STATUS_FRAME_ID) {
        primary_hv_status_t raw_status = { 0 };
        primary_hv_status_converted_t conv_status = { 0 };

        switch (fsm_get_state()) {
            case STATE_INIT:
                conv_status.status = primary_hv_status_status_init;
                break;
            case STATE_IDLE:
                conv_status.status = primary_hv_status_status_idle;
                break;
            case STATE_WAIT_AIRN_CLOSE:
                conv_status.status = primary_hv_status_status_airn_close;
                break;
            case STATE_WAIT_TS_PRECHARGE:
                conv_status.status = primary_hv_status_status_precharge;
                break;
            case STATE_WAIT_AIRP_CLOSE:
                conv_status.status = primary_hv_status_status_airp_close;
                break;
            case STATE_TS_ON:
                conv_status.status = primary_hv_status_status_ts_on;
                break;
            case STATE_FATAL_ERROR:
                conv_status.status = primary_hv_status_status_fatal_error;
                break;
            default:
                conv_status.status = primary_hv_status_status_idle;
                break;
        }

        primary_hv_status_conversion_to_raw_struct(&raw_status, &conv_status);
        
        int data_len = primary_hv_status_pack(buffer, &raw_status, PRIMARY_HV_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_CELLS_TEMP_STATS_FRAME_ID) {
        primary_hv_cells_temp_stats_t raw_temp = { 0 };
        primary_hv_cells_temp_stats_converted_t conv_temp = { 0 };
        
        conv_temp.avg = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_average());
        conv_temp.min = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_min());
        conv_temp.max = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_max());

        primary_hv_cells_temp_stats_conversion_to_raw_struct(&raw_temp, &conv_temp);

        int data_len = primary_hv_cells_temp_stats_pack(buffer, &raw_temp, PRIMARY_HV_CELLS_TEMP_STATS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_ERRORS_FRAME_ID) {
        primary_hv_errors_t raw_errors = { 0 };
        primary_hv_errors_converted_t conv_errors  = { 0 };

        ErrorGroup expired[ERROR_GROUP_COUNT] = { 0 };
        size_t expired_count = error_dump_expired_groups(expired);

        for (size_t i = 0; i < expired_count; ++i) {
            switch (expired[i]) {
                case ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE:
                    conv_errors.errors_cell_under_voltage = 1;
                    break;
                case ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE:
                    conv_errors.errors_cell_over_voltage = 1;
                    break;
                case ERROR_GROUP_ERROR_CELL_UNDER_TEMPERATURE:
                    // TODO: Add under temperature to canlib
                    // conv_errors.errors_cell_under_temperature = 1;
                    break;
                case ERROR_GROUP_ERROR_CELL_OVER_TEMPERATURE:
                    conv_errors.errors_cell_over_temperature = 1;
                    break;
                case ERROR_GROUP_ERROR_OVER_CURRENT:
                    conv_errors.errors_over_current = 1;
                    break;
                case ERROR_GROUP_ERROR_CAN:
                    conv_errors.errors_can = 1;
                    break;
                case ERROR_GROUP_ERROR_INT_VOLTAGE_MISMATCH:
                    conv_errors.errors_int_voltage_mismatch = 1;
                    break;
                case ERROR_GROUP_ERROR_CELLBOARD_COMM:
                    conv_errors.errors_cellboard_comm = 1;
                    break;
                case ERROR_GROUP_ERROR_CELLBOARD_INTERNAL:
                    conv_errors.errors_cellboard_internal = 1;
                    break;
                case ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED:
                    conv_errors.errors_connector_disconnected = 1;
                    break;
                case ERROR_GROUP_ERROR_FANS_DISCONNECTED:
                    conv_errors.errors_fans_disconnected = 1;
                    break;
                case ERROR_GROUP_ERROR_FEEDBACK:
                    conv_errors.errors_feedback = 1;
                    break;
                case ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY:
                    conv_errors.errors_feedback_circuitry = 1;
                    break;
                case ERROR_GROUP_ERROR_EEPROM_COMM:
                    conv_errors.errors_eeprom_comm = 1;
                    break;
                case ERROR_GROUP_ERROR_EEPROM_WRITE:
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
    /*
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
    */
    else if (id == PRIMARY_HV_MAINBOARD_VERSION_FRAME_ID) {
        primary_hv_mainboard_version_t raw_version = { 0 };
        primary_hv_mainboard_version_converted_t conv_version = { 0 };

        conv_version.canlib_build_time = CANLIB_BUILD_TIME;
        conv_version.component_build_time = build_epoch;

        primary_hv_mainboard_version_conversion_to_raw_struct(&raw_version, &conv_version);

        int data_len = primary_hv_mainboard_version_pack(buffer, &raw_version, PRIMARY_HV_MAINBOARD_VERSION_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_FEEDBACK_STATUS_FRAME_ID) {
        primary_hv_feedback_status_t raw_status = { 0 };
        primary_hv_feedback_status_converted_t conv_status = { 0 };

        // Get feedbacks status
        feedback_feed_t fbs[FEEDBACK_N] = { 0 };
        feedback_get_all_states(fbs);
        
        // TODO: Set feedbacks status (is_circuitry)
        for (size_t i = 0; i < FEEDBACK_N; i++) {
            switch(i) {
                case FEEDBACK_IMPLAUSIBILITY_DETECTED_POS:
                    conv_status.feedback_implausibility_detected = fbs[i].real_state;
                    break;
                case FEEDBACK_IMD_COCKPIT_POS:
                    conv_status.feedback_imd_cockpit = fbs[i].real_state;
                    break;
                case FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS:
                    conv_status.feedback_tsal_green_fault_latched = fbs[i].real_state;
                    break;
                case FEEDBACK_BMS_COCKPIT_POS:
                    conv_status.feedback_bms_cockpit = fbs[i].real_state;
                    break;
                case FEEDBACK_EXT_LATCHED_POS:
                    conv_status.feedback_ext_latched = fbs[i].real_state;
                    break;
                case FEEDBACK_TSAL_GREEN_POS:
                    conv_status.feedback_tsal_green = fbs[i].real_state;
                    break;
                case FEEDBACK_TS_OVER_60V_STATUS_POS:
                    conv_status.feedback_ts_over_60v_status = fbs[i].real_state;
                    break;
                case FEEDBACK_AIRN_STATUS_POS:
                    conv_status.feedback_airn_status = fbs[i].real_state;
                    break;
                case FEEDBACK_AIRP_STATUS_POS:
                    conv_status.feedback_airp_status = fbs[i].real_state;
                    break;
                case FEEDBACK_AIRP_GATE_POS:
                    conv_status.feedback_airp_gate = fbs[i].real_state;
                    break;
                case FEEDBACK_AIRN_GATE_POS:
                    conv_status.feedback_airn_gate = fbs[i].real_state;
                    break;
                case FEEDBACK_PRECHARGE_STATUS_POS:
                    conv_status.feedback_precharge_status = fbs[i].real_state;
                    break;
                case FEEDBACK_TSP_OVER_60V_STATUS_POS:
                    conv_status.feedback_tsp_over_60v_status = fbs[i].real_state;
                    break;
                case FEEDBACK_IMD_FAULT_POS:
                    conv_status.feedback_imd_fault = fbs[i].real_state;
                    break;
                case FEEDBACK_CHECK_MUX_POS:
                    conv_status.feedback_check_mux = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_END_POS:
                    conv_status.feedback_sd_end = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_OUT_POS:
                    conv_status.feedback_sd_out = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_IN_POS:
                    conv_status.feedback_sd_in = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_BMS_POS:
                    conv_status.feedback_sd_bms = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_IMD_POS:
                    conv_status.feedback_sd_imd = fbs[i].real_state;
                    break;
            }
        }

        primary_hv_feedback_status_conversion_to_raw_struct(&raw_status, &conv_status);

        int data_len = primary_hv_feedback_status_pack(buffer, &raw_status, PRIMARY_HV_FEEDBACK_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_FANS_STATUS_FRAME_ID) {
        primary_hv_fans_status_t raw_fans = { 0 };
        primary_hv_fans_status_converted_t conv_fans = { 0 };

        conv_fans.fans_override = fans_is_overrided();
        conv_fans.fans_speed = fans_get_speed();

        primary_hv_fans_status_conversion_to_raw_struct(&raw_fans, &conv_fans);

        int data_len = primary_hv_fans_status_pack(buffer, &raw_fans, PRIMARY_HV_FANS_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_IMD_STATUS_FRAME_ID) {
        primary_hv_imd_status_t raw_imd = { 0 };
        primary_hv_imd_status_converted_t conv_imd = { 0 };
        
        conv_imd.imd_details = imd_get_details();
        conv_imd.imd_duty_cycle = imd_get_duty_cycle_percentage();
        conv_imd.imd_fault = imd_is_fault();
        conv_imd.imd_freq = imd_get_freq();
        conv_imd.imd_period = imd_get_period();
        conv_imd.imd_status = imd_get_state();

        primary_hv_imd_status_conversion_to_raw_struct(&raw_imd, &conv_imd);

        int data_len = primary_hv_imd_status_pack(buffer, &raw_imd, PRIMARY_HV_IMD_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_FEEDBACK_TS_VOLTAGE_FRAME_ID) {
        primary_hv_feedback_ts_voltage_t raw_ts_feedbacks = { 0 };
        primary_hv_feedback_ts_voltage_converted_t conv_ts_feedbacks = { 0 };

        conv_ts_feedbacks.airn_gate = feedback_get_voltage(FEEDBACK_AIRN_GATE_POS);
        conv_ts_feedbacks.airp_gate = feedback_get_voltage(FEEDBACK_AIRP_GATE_POS);
        conv_ts_feedbacks.airn_status = feedback_get_voltage(FEEDBACK_AIRN_STATUS_POS);
        conv_ts_feedbacks.airp_status = feedback_get_voltage(FEEDBACK_AIRN_STATUS_POS);
        conv_ts_feedbacks.precharge_status = feedback_get_voltage(FEEDBACK_PRECHARGE_STATUS_POS);
        conv_ts_feedbacks.ts_over_60v_status = feedback_get_voltage(FEEDBACK_TS_OVER_60V_STATUS_POS);
        conv_ts_feedbacks.tsp_over_60v_status = feedback_get_voltage(FEEDBACK_TSP_OVER_60V_STATUS_POS);

        primary_hv_feedback_ts_voltage_conversion_to_raw_struct(&raw_ts_feedbacks, &conv_ts_feedbacks);
        
        int data_len = primary_hv_feedback_ts_voltage_pack(buffer, &raw_ts_feedbacks, PRIMARY_HV_FEEDBACK_TS_VOLTAGE_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_FEEDBACK_SD_VOLTAGE_FRAME_ID) {
        primary_hv_feedback_sd_voltage_t raw_sd_feedbacks = { 0 };
        primary_hv_feedback_sd_voltage_converted_t conv_sd_feedbacks = { 0 };

        conv_sd_feedbacks.sd_bms = feedback_get_voltage(FEEDBACK_SD_BMS_POS);
        conv_sd_feedbacks.sd_end = feedback_get_voltage(FEEDBACK_SD_END_POS);
        conv_sd_feedbacks.sd_imd = feedback_get_voltage(FEEDBACK_SD_IMD_POS);
        conv_sd_feedbacks.sd_in = feedback_get_voltage(FEEDBACK_SD_IN_POS);
        conv_sd_feedbacks.sd_out = feedback_get_voltage(FEEDBACK_SD_OUT_POS);

        primary_hv_feedback_sd_voltage_conversion_to_raw_struct(&raw_sd_feedbacks, &conv_sd_feedbacks);
        
        int data_len = primary_hv_feedback_sd_voltage_pack(buffer, &raw_sd_feedbacks, PRIMARY_HV_FEEDBACK_SD_VOLTAGE_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_FEEDBACK_MISC_VOLTAGE_FRAME_ID) {
        primary_hv_feedback_misc_voltage_t raw_misc_feedbacks = { 0 };
        primary_hv_feedback_misc_voltage_converted_t conv_misc_feedbacks = { 0 };

        conv_misc_feedbacks.implausibility_detected = feedback_get_voltage(FEEDBACK_IMPLAUSIBILITY_DETECTED_POS);
        conv_misc_feedbacks.bms_cockpit = feedback_get_voltage(FEEDBACK_BMS_COCKPIT_POS);
        conv_misc_feedbacks.imd_cockpit = feedback_get_voltage(FEEDBACK_IMD_COCKPIT_POS);
        conv_misc_feedbacks.tsal_green_fault_latched = feedback_get_voltage(FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS);
        conv_misc_feedbacks.ext_latched = feedback_get_voltage(FEEDBACK_EXT_LATCHED_POS);
        conv_misc_feedbacks.tsal_green = feedback_get_voltage(FEEDBACK_TSAL_GREEN_POS);
        conv_misc_feedbacks.imd_fault = feedback_get_voltage(FEEDBACK_IMD_FAULT_POS);
        conv_misc_feedbacks.check_mux = feedback_get_voltage(FEEDBACK_CHECK_MUX_POS);

        primary_hv_feedback_misc_voltage_conversion_to_raw_struct(&raw_misc_feedbacks, &conv_misc_feedbacks);
        
        int data_len = primary_hv_feedback_misc_voltage_pack(buffer, &raw_misc_feedbacks, PRIMARY_HV_FEEDBACK_MISC_VOLTAGE_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_DEBUG_SIGNALS_FRAME_ID) {
        primary_hv_debug_signals_t raw_debug;
    
        primary_hv_debug_signals_conversion_to_raw_struct(&raw_debug, &conv_debug);

        int data_len = primary_hv_debug_signals_pack(buffer, &raw_debug, PRIMARY_HV_DEBUG_SIGNALS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    // else if (id == PRIMARY_DEBUG_SIGNAL_2_FRAME_ID) {
    //     Error err[ERROR_INSTANCE_COUNT] = { 0 };
    //     error_dump_running(err);


    //     primary_debug_signal_2_t raw_debug;
    //     primary_debug_signal_2_converted_t conv_debug = {
    //         .device_id = primary_debug_signal_2_device_id_hv_mainboard,
    //         .field_1 = err[0].group / (float)ERROR_GROUP_COUNT,
    //         .field_2 = err[0].is_running,
    //         .field_3 = err[0].is_expired
    //     };
    
    //     primary_debug_signal_2_conversion_to_raw_struct(&raw_debug, &conv_debug);
    //     tx_header.DLC = primary_debug_signal_2_pack(buffer, &raw_debug, PRIMARY_DEBUG_SIGNAL_2_BYTE_SIZE);
    // }
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
        BalRequest request = bal_get_request();

        bms_set_balancing_status_t raw_bal = { 0 };
        bms_set_balancing_status_converted_t conv_bal = {
            .balancing_status = request.status,
            .target = MAX(CELL_MIN_VOLTAGE, cell_voltage_get_min()),
            .threshold = request.threshold
        };
        bms_set_balancing_status_conversion_to_raw_struct(&raw_bal, &conv_bal);
        if ((tx_header.DLC = bms_set_balancing_status_pack(buffer, &raw_bal, BMS_SET_BALANCING_STATUS_BYTE_SIZE)) < 0)
            return HAL_ERROR;

        size_t errors = 0;
        for (size_t i = 0; i < CELLBOARD_COUNT; ++i) {
            if (can_send(&BMS_CAN, buffer, &tx_header) != HAL_OK)
                ++errors;
        }        
        return errors == 0 ? HAL_OK : HAL_ERROR;
    }
    else if (id == BMS_JMP_TO_BLT_FRAME_ID) {
        bms_jmp_to_blt_t raw_jmp = { 0 };
        bms_jmp_to_blt_converted_t conv_jmp = { 0 };

        conv_jmp.cellboard_id = flash_cellboard_id;
        // TODO: Board index (not used in cellboard)
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
        error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == BMS_CAN.Instance) {
        // Reset can errors
        error_reset(ERROR_GROUP_ERROR_CAN, 1);

        // Forward data to the cellboards
        if (rx_header.StdId >= BMS_FLASH_CELLBOARD_0_TX_FRAME_ID && rx_header.StdId <= BMS_FLASH_CELLBOARD_5_RX_FRAME_ID) {
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
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
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
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
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
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            bms_temperatures_raw_to_conversion_struct(&conv_temps, &raw_temps);

            // Reset time since last communication
            time_since_last_comm[conv_temps.cellboard_id] = HAL_GetTick();

            // TODO: Test
#if defined(TEMP_GROUP_ERROR_ENABLE) && defined(TEMP_ERROR_ENABLE)
            // Add error bit to the temp group
            size_t index = conv_temps.start_index / 4;
            size_t bit = (index < 2) ? index : (index + ((index - 2) / 3 + 1));
            if (index % 3 == 1) {
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
            bool is_error_set = false;
            for (size_t board_id = 0; board_id < CELLBOARD_COUNT && !is_error_set; board_id++) {
                for (size_t temp_bit = 0; temp_bit < TEMP_STRIPS_PER_BUS * 2; temp_bit += 2) {
                    if ((temp_errors[board_id] & (1 << temp_bit)) && (temp_errors[board_id] & (1 << (temp_bit + 1)))) {
                        error_set(ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED, 2, HAL_GetTick());
                        is_error_set = true;
                        break;
                    }
                }
            }
            if (!is_error_set)
                error_reset(ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED, 2);
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
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
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
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            bms_board_status_raw_to_conversion_struct(&conv_status, &raw_status);
            
            // Reset time since last communication
            time_since_last_comm[conv_status.cellboard_id] = HAL_GetTick();

            bal_update_status(conv_status.cellboard_id, conv_status.balancing_status);

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
            ERROR_TOGGLE_IF(error_status != 0, ERROR_GROUP_ERROR_CELLBOARD_INTERNAL, conv_status.cellboard_id, HAL_GetTick());

            // Forward data
            CAN_TxHeaderTypeDef tx_header = {
                .DLC = 0,
                .ExtId = 0,
                .IDE = CAN_ID_STD,
                .RTR = CAN_RTR_DATA,
                .StdId = PRIMARY_HV_BALANCING_STATUS_FRAME_ID,
                .TransmitGlobalTime = DISABLE
            };
            uint8_t buffer[8] = { 0 };
            primary_hv_balancing_status_t raw_fwd_status = { 0 };
            primary_hv_balancing_status_converted_t conv_fwd_status = { 0 };

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

            primary_hv_balancing_status_conversion_to_raw_struct(&raw_fwd_status, &conv_fwd_status);

            int data_len = primary_hv_balancing_status_pack(buffer, &raw_fwd_status, PRIMARY_HV_BALANCING_STATUS_BYTE_SIZE);
            if (data_len < 0)
                return;
            tx_header.DLC = data_len;

            can_send(&CAR_CAN, buffer, &tx_header);
        }
        else if (rx_header.StdId == BMS_CELLBOARD_VERSION_FRAME_ID) {
            bms_cellboard_version_t raw_version = { 0 };
            bms_cellboard_version_converted_t conv_version = { 0 };

            if (bms_cellboard_version_unpack(&raw_version, rx_data, BMS_CELLBOARD_VERSION_BYTE_SIZE) < 0) {
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            bms_cellboard_version_raw_to_conversion_struct(&conv_version, &raw_version);
            
            // Reset time since last communication
            time_since_last_comm[conv_version.cellboard_id] = HAL_GetTick();

            // Forward data
            CAN_TxHeaderTypeDef tx_header = {
                .DLC = 0,
                .ExtId = 0,
                .IDE = CAN_ID_STD,
                .RTR = CAN_RTR_DATA,
                .StdId = PRIMARY_HV_CELLBOARD_VERSION_FRAME_ID,
                .TransmitGlobalTime = DISABLE
            };

            uint8_t buffer[8] = { 0 };
            primary_hv_cellboard_version_t raw_fwd_version = { 0 };
            primary_hv_cellboard_version_converted_t conv_fwd_version = { 0 };

            conv_fwd_version.cellboard_id = conv_version.cellboard_id;
            conv_fwd_version.canlib_build_time = conv_version.canlib_build_time;
            conv_fwd_version.component_version = conv_version.component_version;
            
            primary_hv_cellboard_version_conversion_to_raw_struct(&raw_fwd_version, &conv_fwd_version);

            int data_len = primary_hv_cellboard_version_pack(buffer, &raw_fwd_version, PRIMARY_HV_CELLBOARD_VERSION_BYTE_SIZE);
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
        error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == CAR_CAN.Instance) {
        error_reset(ERROR_GROUP_ERROR_CAN, 1);

        if (rx_header.StdId >= BMS_FLASH_CELLBOARD_0_TX_FRAME_ID && rx_header.StdId <= BMS_FLASH_CELLBOARD_5_RX_FRAME_ID) {
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
        else if (rx_header.StdId == PRIMARY_ECU_STATUS_FRAME_ID) {
            // Reset the watchdog timer
            watchdog_reset(rx_header.StdId);
        }
        else if (rx_header.StdId == PRIMARY_HV_SET_STATUS_ECU_FRAME_ID || rx_header.StdId == PRIMARY_HV_SET_STATUS_HANDCART_FRAME_ID) {
            primary_hv_set_status_ecu_t raw_ts_status = { 0 };
            primary_hv_set_status_ecu_converted_t conv_ts_status = { 0 };

            if (primary_hv_set_status_ecu_unpack(&raw_ts_status, rx_data, PRIMARY_HV_SET_STATUS_ECU_BYTE_SIZE) < 0) {
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_hv_set_status_ecu_raw_to_conversion_struct(&conv_ts_status, &raw_ts_status);

            // Request for TS status change
            switch (conv_ts_status.hv_status_set) {
                case primary_hv_set_status_ecu_hv_status_set_off:
                    set_ts_request.is_new = true;
                    set_ts_request.next_state = STATE_IDLE;
                    break;
                case primary_hv_set_status_ecu_hv_status_set_on:
                    set_ts_request.is_new = true;
                    set_ts_request.next_state = STATE_WAIT_AIRN_CLOSE;
                    break;
            }
        }
        else if (rx_header.StdId == PRIMARY_HV_SET_BALANCING_STATUS_HANDCART_FRAME_ID || rx_header.StdId == PRIMARY_HV_SET_BALANCING_STATUS_STEERING_WHEEL_FRAME_ID) {
            primary_hv_set_balancing_status_handcart_t raw_bal_status = { 0 };
            primary_hv_set_balancing_status_handcart_converted_t conv_bal_status = { 0 };

            if (primary_hv_set_balancing_status_handcart_unpack(&raw_bal_status, rx_data, PRIMARY_HV_SET_BALANCING_STATUS_HANDCART_BYTE_SIZE) < 0) {
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_hv_set_balancing_status_handcart_raw_to_conversion_struct(&conv_bal_status, &raw_bal_status);

            // Send balancing request
            bal_change_status_request(conv_bal_status.set_balancing_status, (voltage_t)conv_bal_status.balancing_threshold * 10);
        }
        else if (rx_header.StdId == PRIMARY_HANDCART_STATUS_FRAME_ID) {
            primary_handcart_status_t raw_handcart_status = { 0 };
            primary_handcart_status_converted_t conv_handcart_status = { 0 };

            if (primary_handcart_status_unpack(&raw_handcart_status, rx_data, PRIMARY_HANDCART_STATUS_BYTE_SIZE) < 0) {
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_handcart_status_raw_to_conversion_struct(&conv_handcart_status, &raw_handcart_status);

            // Reset watchdog
            watchdog_reset(PRIMARY_HANDCART_STATUS_FRAME_ID);

            is_handcart_connected = conv_handcart_status.connected;
        }
        /*
        else if (rx_header.StdId == PRIMARY_HV_CAN_FORWARD_FRAME_ID) {
            bms_state_t fsm_state = fsm_get_state();
            if (fsm_state != STATE_INIT && fsm_state != STATE_IDLE && fsm_state != STATE_FATAL_ERROR) {
                can_forward = 0;
                return;
            }
            
            primary_hv_can_forward_t raw_can_forward = { 0 };
            primary_hv_can_forward_converted_t conv_can_forward = { 0 };

            if (primary_hv_can_forward_unpack(&raw_can_forward, rx_data, PRIMARY_HV_CAN_FORWARD_BYTE_SIZE) < 0) {
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
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
        */
        else if (rx_header.StdId == PRIMARY_HV_SET_FANS_STATUS_FRAME_ID) {
            primary_hv_set_fans_status_t raw_fans = { 0 };
            primary_hv_set_fans_status_converted_t conv_fans = { 0 };

            if (primary_hv_set_fans_status_unpack(&raw_fans, rx_data, PRIMARY_HV_SET_FANS_STATUS_BYTE_SIZE) < 0) {
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_hv_set_fans_status_raw_to_conversion_struct(&conv_fans, &raw_fans);

            // Set fans override and speed
            fans_set_override(conv_fans.fans_override);
            fans_set_speed(conv_fans.fans_speed);
        }
        else if (rx_header.StdId == PRIMARY_HV_JMP_TO_BLT_FRAME_ID) {
            primary_hv_jmp_to_blt_t raw_jmp;
            primary_hv_jmp_to_blt_converted_t conv_jmp;

            if (primary_hv_jmp_to_blt_unpack(&raw_jmp, rx_data, PRIMARY_HV_JMP_TO_BLT_BYTE_SIZE) < 0) {
                error_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
                return;
            }
            primary_hv_jmp_to_blt_raw_to_conversion_struct(&conv_jmp, &raw_jmp);

            bms_state_t state = fsm_get_state();
            if (conv_jmp.forward) {
                flash_cellboard_id = conv_jmp.cellboard_id;
                can_bms_send(BMS_JMP_TO_BLT_FRAME_ID);
            }
            else if ((state == STATE_INIT || state == STATE_IDLE || state == STATE_FATAL_ERROR) && !bal_is_balancing())
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
            ERROR_TOGGLE_IF(HAL_GetTick() - time_since_last_comm[i] >= CELLBOARD_COMM_TIMEOUT, ERROR_GROUP_ERROR_CELLBOARD_COMM, i, HAL_GetTick());
        }
    }
}
