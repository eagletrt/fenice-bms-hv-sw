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
#include "bootloader.h"
#include "can-bms-api.h"
#include "can-bms.h"
#include "can-primary-api.h"
#include "can-primary.h"
#include "can-version.h"
#include "cell_voltage.h"
#include "cli_bms.h"
#include "error_simple.h"
#include "fans_buzzer.h"
#include "feedback.h"
#include "imd.h"
#include "internal_voltage.h"
#include "mainboard_config.h"
#include "pack/current.h"
#include "pack/pack.h"
#include "soc.h"
#include "temperature.h"
#include "usart.h"
#include "watchdog.h"

#include <math.h>
#include <string.h>
#include <time.h>

#ifdef TEMP_GROUP_ERROR_ENABLE
uint16_t temp_errors[CELLBOARD_COUNT];
#endif  // TEMP_GROUP_ERROR_ENABLE

uint32_t time_since_last_comm[CELLBOARD_COUNT];
bool can_forward;
uint8_t flash_cellboard_id;
float debug_signal;

static time_t build_epoch;

static struct CanPrimaryHvBmsCellboardVersion cellboard_version[CELLBOARD_COUNT];
static struct CanPrimaryHvBmsBalancingStatus balancing_status[CELLBOARD_COUNT];

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

bool can_is_forwarding() {
    return can_forward;
}
void can_bms_init() {
    struct tm tm;
    if (strptime(__DATE__ " "__TIME__, "%b %d %Y %H:%M:%S", &tm) != NULL)
        build_epoch = mktime(&tm);

    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of:
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    CAN_FilterTypeDef filter = {
        .FilterActivation     = CAN_FILTER_ENABLE,
        .FilterBank           = 14,
        .FilterFIFOAssignment = CAN_FILTER_FIFO0,
        .FilterIdHigh         = ((1U << 11) - 1) << 5,  // Take all ids to 2^11 - 1
        .FilterIdLow          = 0,                      // Take all ids from 0
        .FilterMaskIdHigh     = 0,
        .FilterMaskIdLow      = 0,
        .FilterMode           = CAN_FILTERMODE_IDMASK,
        .FilterScale          = CAN_FILTERSCALE_16BIT,
        .SlaveStartFilterBank = CAN_SLAVE_START_FILTER_BANK};

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
        .FilterActivation     = CAN_FILTER_ENABLE,
        .FilterBank           = 0,
        .FilterFIFOAssignment = CAN_FILTER_FIFO1,
        .FilterIdHigh         = ((1U << 11) - 1) << 5,  // Take all ids to 2^11 - 1
        .FilterIdLow          = 0,                      // Take all ids from 0
        .FilterMaskIdHigh     = 0,
        .FilterMaskIdLow      = 0,
        .FilterMode           = CAN_FILTERMODE_IDMASK,
        .FilterScale          = CAN_FILTERSCALE_16BIT,
        .SlaveStartFilterBank = CAN_SLAVE_START_FILTER_BANK};

    // Enable filters and start CAN
    HAL_CAN_ConfigFilter(&CAR_CAN, &filter);
    HAL_CAN_ActivateNotification(&CAR_CAN, CAN_IT_ERROR | CAN_IT_RX_FIFO1_MSG_PENDING);
    HAL_CAN_Start(&CAR_CAN);
}

HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header) {
    // Wait for free mailboxes
    if (_can_wait(hcan, 3) != HAL_OK) {
        return HAL_TIMEOUT;
    }

    uint32_t mailbox = 0;
    // if (hcan == &CAR_CAN && header->StdId == 0)
    //     return HAL_OK;

    // Add message to a free mailbox
    HAL_StatusTypeDef status = HAL_CAN_AddTxMessage(hcan, header, buffer, &mailbox);
    if (status != HAL_OK) {
        error_simple_set(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);
    } else {
        error_simple_reset(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);
    }
    return status;
}

HAL_StatusTypeDef can_car_send(uint16_t id) {
    // Return if busy
    if (can_forward)  // && id != PRIMARY_HV_CAN_FORWARD_STATUS_FRAME_ID)
        return HAL_BUSY;

    CAN_TxHeaderTypeDef tx_header = {
        .DLC = 0, .ExtId = 0, .IDE = CAN_ID_STD, .RTR = CAN_RTR_DATA, .StdId = id, .TransmitGlobalTime = DISABLE};
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH] = {0};

    union CanPrimaryMessages message = {0};
    if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_TS_VOLTAGE) {
        message.hv_bms_ts_voltage.bus_v     = CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp());
        message.hv_bms_ts_voltage.pack_v    = CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_bat());
        message.hv_bms_ts_voltage.cellsum_v = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_sum());
    } else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_VOLTAGES_INFO) {
        float v_min = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_min());
        float v_max = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_max());

        message.hv_bms_cellboard_voltages_info.max_v     = v_max;
        message.hv_bms_cellboard_voltages_info.min_v     = v_min;
        message.hv_bms_cellboard_voltages_info.average_v = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_avg());
        message.hv_bms_cellboard_voltages_info.delta_v   = v_max - v_min;
    } else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CURRENT) {
        message.hv_bms_current.current_a = current_get_current();
    } else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_POWER) {
        message.hv_bms_power.power_kw =
            (current_get_current() * CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp())) / 1000.f;
    }
    /*
    else if (id == PRIMARY_HV_ENERGY_FRAME_ID) {
        primary_hv_energy_t raw_energy            = {0};
        primary_hv_energy_converted_t conv_energy = {0};

        // TODO: Add energy? (old code)
        conv_energy.energy = 0;

        primary_hv_energy_conversion_to_raw_struct(&raw_energy, &conv_energy);

        int data_len = primary_hv_energy_pack(buffer, &raw_energy, PRIMARY_HV_ENERGY_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    else if (id == PRIMARY_HV_SOC_FRAME_ID) {
        primary_hv_soc_t raw_soc            = {0};
        primary_hv_soc_converted_t conv_soc = {0};

        // TODO: Add soc
        conv_soc.soc = 0;

        primary_hv_soc_conversion_to_raw_struct(&raw_soc, &conv_soc);

        int data_len = primary_hv_soc_pack(buffer, &raw_soc, PRIMARY_HV_SOC_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    */
    else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_STATUS) {
        switch (fsm_get_state()) {
            case STATE_INIT:
                message.hv_bms_status.name = CAN_PRIMARY_HV_BMS_STATUS_NAME_INIT;
                break;
            case STATE_IDLE:
                message.hv_bms_status.name = CAN_PRIMARY_HV_BMS_STATUS_NAME_IDLE;
                break;
            case STATE_WAIT_AIRN_CLOSE:
                // message.hv_bms_status.name = primary_hv_status_status_airn_close;
                message.hv_bms_status.name = CAN_PRIMARY_HV_BMS_STATUS_NAME_WAIT_AIRN_CLOSE;
                break;
            case STATE_WAIT_TS_PRECHARGE:
                // message.hv_bms_status.name = primary_hv_status_status_precharge;
                message.hv_bms_status.name = CAN_PRIMARY_HV_BMS_STATUS_NAME_WAIT_PRECHARGE;
                break;
            case STATE_WAIT_AIRP_CLOSE:
                message.hv_bms_status.name = CAN_PRIMARY_HV_BMS_STATUS_NAME_WAIT_AIRP_CLOSE;
                break;
            case STATE_TS_ON:
                message.hv_bms_status.name = CAN_PRIMARY_HV_BMS_STATUS_NAME_TS_ON;
                break;
            case STATE_FATAL_ERROR:
                message.hv_bms_status.name = CAN_PRIMARY_HV_BMS_STATUS_NAME_FATAL;
                break;
            default:
                message.hv_bms_status.name = CAN_PRIMARY_HV_BMS_STATUS_NAME_IDLE;
                break;
        }
    } else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_TEMPERATURES_INFO) {
        message.hv_bms_cellboard_temperatures_info.average_c = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_average());
        message.hv_bms_cellboard_temperatures_info.min_c     = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_min());
        message.hv_bms_cellboard_temperatures_info.max_c     = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_max());
    } else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_ERRORS) {
        size_t n_expired_errors = get_expired_errors();

        for (size_t i = 0; i < n_expired_errors; ++i) {
            switch (error_simple_dump[i].group) {
                case ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE:
                    message.hv_bms_errors.cellundervoltage = 1;
                    break;
                case ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE:
                    message.hv_bms_errors.cellovervoltage = 1;
                    break;
                case ERROR_GROUP_ERROR_CELL_UNDER_TEMPERATURE:
                    // TODO: Add under temperature to canlib
                    // message.hv_bms_errors.cellundertemperature = 1;
                    break;
                case ERROR_GROUP_ERROR_CELL_OVER_TEMPERATURE:
                    message.hv_bms_errors.cellovertemperature = 1;
                    break;
                case ERROR_GROUP_ERROR_OVER_CURRENT:
                    message.hv_bms_errors.overcurrent = 1;
                    break;
                case ERROR_GROUP_ERROR_CAN:
                    message.hv_bms_errors.cancommunication = 1;
                    break;
                case ERROR_GROUP_ERROR_INT_VOLTAGE_MISMATCH:
                    message.hv_bms_errors.tsvoltagemismatch = 1;
                    break;
                case ERROR_GROUP_ERROR_CELLBOARD_COMM:
                    message.hv_bms_errors.cellboardcommunication = 1;
                    break;
                case ERROR_GROUP_ERROR_CELLBOARD_INTERNAL:
                    message.hv_bms_errors.cellboardinternal = 1;
                    break;
                case ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED:
                    message.hv_bms_errors.connectordisconnected = 1;
                    break;
                case ERROR_GROUP_ERROR_FANS_DISCONNECTED:
                    message.hv_bms_errors.fansdisconnected = 1;
                    break;
                case ERROR_GROUP_ERROR_FEEDBACK:
                    message.hv_bms_errors.feedback = 1;
                    break;
                case ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY:
                    message.hv_bms_errors.feedbackcircuitry = 1;
                    break;
                case ERROR_GROUP_ERROR_EEPROM_COMM:
                    message.hv_bms_errors.eepromcommunication = 1;
                    break;
                case ERROR_GROUP_ERROR_EEPROM_WRITE:
                    message.hv_bms_errors.eepromwrite = 1;
                    break;

                default:
                    break;
            }
        }
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
    else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_VERSION) {
        message.hv_bms_version.canlibbuildtime_s = can_generation_time;
        message.hv_bms_version.buildtime_s       = build_epoch;
    }
    else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_VERSION) {
        for (uint8_t cellboard_id = 0; cellboard_id < CELLBOARD_COUNT; ++cellboard_id) {
            message.hv_bms_cellboard_version.id = cellboard_id;
            message.hv_bms_cellboard_version.buildtime_s = cellboard_version[cellboard_id].buildtime_s;
            message.hv_bms_cellboard_version.canlibbuildtime_s = cellboard_version[cellboard_id].canlibbuildtime_s;

            int serialize_byte_count = can_primary_api_serialize_from_id(tx_header.StdId, &message, buffer);
            if (serialize_byte_count < 0) {
                return HAL_ERROR;
            }
            tx_header.DLC = serialize_byte_count;
            can_send(&CAR_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        return HAL_OK;
    }
    else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_FEEDBACK_STATUS) {
        // Get feedbacks status
        feedback_feed_t fbs[FEEDBACK_N] = {0};
        feedback_get_all_states(fbs);

        // TODO: Set feedbacks status (is_circuitry)
        for (size_t i = 0; i < FEEDBACK_N; i++) {
            switch (i) {
                case FEEDBACK_IMPLAUSIBILITY_DETECTED_POS:
                    message.hv_bms_feedback_status.implausibility =
                        (fbs[i].real_state == FEEDBACK_STATE_H)
                            ? CAN_PRIMARY_HV_BMS_FEEDBACK_STATUS_IMPLAUSIBILITY_LOW
                            : ((fbs[i].real_state == FEEDBACK_STATE_L)
                                   ? CAN_PRIMARY_HV_BMS_FEEDBACK_STATUS_IMPLAUSIBILITY_HIGH
                                   : CAN_PRIMARY_HV_BMS_FEEDBACK_STATUS_IMPLAUSIBILITY_ERROR);
                    break;
                case FEEDBACK_IMD_COCKPIT_POS:
                    message.hv_bms_feedback_status.imdcockpit = fbs[i].real_state;
                    break;
                case FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS:
                    message.hv_bms_feedback_status.tsalgreenfaultlatched = fbs[i].real_state;
                    break;
                case FEEDBACK_BMS_COCKPIT_POS:
                    message.hv_bms_feedback_status.bmscockpit = fbs[i].real_state;
                    break;
                case FEEDBACK_EXT_LATCHED_POS:
                    message.hv_bms_feedback_status.extlatched = fbs[i].real_state;
                    break;
                case FEEDBACK_TSAL_GREEN_POS:
                    message.hv_bms_feedback_status.tsalgreen = fbs[i].real_state;
                    break;
                case FEEDBACK_TS_OVER_60V_STATUS_POS:
                    message.hv_bms_feedback_status.tsover60v =
                        (fbs[i].real_state == FEEDBACK_STATE_H)
                            ? CAN_PRIMARY_HV_BMS_FEEDBACK_STATUS_TSOVER60V_LOW
                            : ((fbs[i].real_state == FEEDBACK_STATE_L)
                                   ? CAN_PRIMARY_HV_BMS_FEEDBACK_STATUS_TSOVER60V_HIGH
                                   : CAN_PRIMARY_HV_BMS_FEEDBACK_STATUS_TSOVER60V_ERROR);
                    break;
                case FEEDBACK_AIRN_STATUS_POS:
                    message.hv_bms_feedback_status.airnstatus = fbs[i].real_state;
                    break;
                case FEEDBACK_AIRP_STATUS_POS:
                    message.hv_bms_feedback_status.airpstatus = fbs[i].real_state;
                    break;
                case FEEDBACK_AIRP_GATE_POS:
                    message.hv_bms_feedback_status.airpgate = fbs[i].real_state;
                    break;
                case FEEDBACK_AIRN_GATE_POS:
                    message.hv_bms_feedback_status.airngate = fbs[i].real_state;
                    break;
                case FEEDBACK_PRECHARGE_STATUS_POS:
                    message.hv_bms_feedback_status.prechargestatus = fbs[i].real_state;
                    break;
                case FEEDBACK_TSP_OVER_60V_STATUS_POS:
                    message.hv_bms_feedback_status.tspover60v = fbs[i].real_state;
                    break;
                case FEEDBACK_IMD_FAULT_POS:
                    message.hv_bms_feedback_status.imdfault =
                        (fbs[i].real_state == FEEDBACK_STATE_H)
                            ? CAN_PRIMARY_HV_BMS_FEEDBACK_STATUS_IMDFAULT_LOW
                            : ((fbs[i].real_state == FEEDBACK_STATE_L)
                                   ? CAN_PRIMARY_HV_BMS_FEEDBACK_STATUS_IMDFAULT_HIGH
                                   : CAN_PRIMARY_HV_BMS_FEEDBACK_STATUS_IMDFAULT_ERROR);
                    break;
                case FEEDBACK_CHECK_MUX_POS:
                    message.hv_bms_feedback_status.checkmux = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_END_POS:
                    message.hv_bms_feedback_status.sdend = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_OUT_POS:
                    message.hv_bms_feedback_status.sdout = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_IN_POS:
                    message.hv_bms_feedback_status.sdin = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_BMS_POS:
                    message.hv_bms_feedback_status.sdbms = fbs[i].real_state;
                    break;
                case FEEDBACK_SD_IMD_POS:
                    message.hv_bms_feedback_status.sdimd = fbs[i].real_state;
                    break;
            }
        }
    }
    else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_BALANCING_STATUS) {
        for (uint8_t cellboard_id = 0; cellboard_id < CELLBOARD_COUNT; ++cellboard_id) {
            message.hv_bms_balancing_status.cellboardid    = cellboard_id;
            message.hv_bms_balancing_status.balancing_bool = balancing_status[cellboard_id].balancing_bool;

            message.hv_bms_balancing_status.errorcancomm   = balancing_status[cellboard_id].errorcancomm;
            message.hv_bms_balancing_status.errorltccomm   = balancing_status[cellboard_id].errorltccomm;
            message.hv_bms_balancing_status.erroropenwire  = balancing_status[cellboard_id].erroropenwire;
            message.hv_bms_balancing_status.errortempcomm0 = balancing_status[cellboard_id].errortempcomm0;
            message.hv_bms_balancing_status.errortempcomm1 = balancing_status[cellboard_id].errortempcomm1;
            message.hv_bms_balancing_status.errortempcomm2 = balancing_status[cellboard_id].errortempcomm2;
            message.hv_bms_balancing_status.errortempcomm3 = balancing_status[cellboard_id].errortempcomm3;
            message.hv_bms_balancing_status.errortempcomm4 = balancing_status[cellboard_id].errortempcomm4;
            message.hv_bms_balancing_status.errortempcomm5 = balancing_status[cellboard_id].errortempcomm5;

            message.hv_bms_balancing_status.balancingcell0  = balancing_status[cellboard_id].balancingcell0;
            message.hv_bms_balancing_status.balancingcell1  = balancing_status[cellboard_id].balancingcell1;
            message.hv_bms_balancing_status.balancingcell2  = balancing_status[cellboard_id].balancingcell2;
            message.hv_bms_balancing_status.balancingcell3  = balancing_status[cellboard_id].balancingcell3;
            message.hv_bms_balancing_status.balancingcell4  = balancing_status[cellboard_id].balancingcell4;
            message.hv_bms_balancing_status.balancingcell5  = balancing_status[cellboard_id].balancingcell5;
            message.hv_bms_balancing_status.balancingcell6  = balancing_status[cellboard_id].balancingcell6;
            message.hv_bms_balancing_status.balancingcell7  = balancing_status[cellboard_id].balancingcell7;
            message.hv_bms_balancing_status.balancingcell8  = balancing_status[cellboard_id].balancingcell8;
            message.hv_bms_balancing_status.balancingcell9  = balancing_status[cellboard_id].balancingcell9;
            message.hv_bms_balancing_status.balancingcell10 = balancing_status[cellboard_id].balancingcell10;
            message.hv_bms_balancing_status.balancingcell11 = balancing_status[cellboard_id].balancingcell11;
            message.hv_bms_balancing_status.balancingcell12 = balancing_status[cellboard_id].balancingcell12;
            message.hv_bms_balancing_status.balancingcell13 = balancing_status[cellboard_id].balancingcell13;
            message.hv_bms_balancing_status.balancingcell14 = balancing_status[cellboard_id].balancingcell14;
            message.hv_bms_balancing_status.balancingcell15 = balancing_status[cellboard_id].balancingcell15;
            message.hv_bms_balancing_status.balancingcell16 = balancing_status[cellboard_id].balancingcell16;
            message.hv_bms_balancing_status.balancingcell17 = balancing_status[cellboard_id].balancingcell17;

            int serialize_byte_count = can_primary_api_serialize_from_id(tx_header.StdId, &message, buffer);
            if (serialize_byte_count < 0) {
                return HAL_ERROR;
            }
            tx_header.DLC = serialize_byte_count;
            can_send(&CAR_CAN, buffer, &tx_header);
            HAL_Delay(1);
        }
        return HAL_OK;
    }
    /*
    else if (id == PRIMARY_HV_FANS_STATUS_FRAME_ID) {
        primary_hv_fans_status_t raw_fans            = {0};
        primary_hv_fans_status_converted_t conv_fans = {0};

        conv_fans.fans_override = fans_is_overrided();
        conv_fans.fans_speed    = fans_get_speed();

        primary_hv_fans_status_conversion_to_raw_struct(&raw_fans, &conv_fans);

        int data_len = primary_hv_fans_status_pack(buffer, &raw_fans, PRIMARY_HV_FANS_STATUS_BYTE_SIZE);
        if (data_len < 0)
            return HAL_ERROR;
        tx_header.DLC = data_len;
    }
    */
    else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_IMD) {
        message.hv_bms_imd.details       = imd_get_details();
        message.hv_bms_imd.dutycycle_pct = imd_get_duty_cycle_percentage();
        message.hv_bms_imd.fault_bool    = imd_is_fault();
        message.hv_bms_imd.frequency_hz  = imd_get_freq();
        message.hv_bms_imd.period_ms     = imd_get_period();
        message.hv_bms_imd.status        = imd_get_state();
    } else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_FEEDBACK_TS_VOLTAGE) {
        message.hv_bms_feedback_ts_voltage.airnstatus_v      = feedback_get_voltage(FEEDBACK_AIRN_STATUS_POS);
        message.hv_bms_feedback_ts_voltage.airngate_v        = feedback_get_voltage(FEEDBACK_AIRN_GATE_POS);
        message.hv_bms_feedback_ts_voltage.prechargestatus_v = feedback_get_voltage(FEEDBACK_PRECHARGE_STATUS_POS);
        message.hv_bms_feedback_ts_voltage.airpstatus_v      = feedback_get_voltage(FEEDBACK_AIRN_STATUS_POS);
        message.hv_bms_feedback_ts_voltage.airpgate_v        = feedback_get_voltage(FEEDBACK_AIRP_GATE_POS);
        message.hv_bms_feedback_ts_voltage.tsover60v_v       = feedback_get_voltage(FEEDBACK_TS_OVER_60V_STATUS_POS);
        message.hv_bms_feedback_ts_voltage.tspover60v_v      = feedback_get_voltage(FEEDBACK_TSP_OVER_60V_STATUS_POS);
    } else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_FEEDBACK_SD_VOLTAGE) {
        message.hv_bms_feedback_sd_voltage.sdbms_v = feedback_get_voltage(FEEDBACK_SD_BMS_POS);
        message.hv_bms_feedback_sd_voltage.sdend_v = feedback_get_voltage(FEEDBACK_SD_END_POS);
        message.hv_bms_feedback_sd_voltage.sdimd_v = feedback_get_voltage(FEEDBACK_SD_IMD_POS);
        message.hv_bms_feedback_sd_voltage.sdin_v  = feedback_get_voltage(FEEDBACK_SD_IN_POS);
        message.hv_bms_feedback_sd_voltage.sdout_v = feedback_get_voltage(FEEDBACK_SD_OUT_POS);
    } else if (id == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_FEEDBACK_MISC_VOLTAGE) {
        message.hv_bms_feedback_misc_voltage.implausibility_v =
            feedback_get_voltage(FEEDBACK_IMPLAUSIBILITY_DETECTED_POS);
        message.hv_bms_feedback_misc_voltage.bmscockpit_v = feedback_get_voltage(FEEDBACK_BMS_COCKPIT_POS);
        message.hv_bms_feedback_misc_voltage.imdcockpit_v = feedback_get_voltage(FEEDBACK_IMD_COCKPIT_POS);
        message.hv_bms_feedback_misc_voltage.tsalgreenfaultlatched_v =
            feedback_get_voltage(FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS);
        message.hv_bms_feedback_misc_voltage.extlatched_v = feedback_get_voltage(FEEDBACK_EXT_LATCHED_POS);
        message.hv_bms_feedback_misc_voltage.tsalgreen_v  = feedback_get_voltage(FEEDBACK_TSAL_GREEN_POS);
        message.hv_bms_feedback_misc_voltage.imdfault_v   = feedback_get_voltage(FEEDBACK_IMD_FAULT_POS);
        message.hv_bms_feedback_misc_voltage.checkmux_v   = feedback_get_voltage(FEEDBACK_CHECK_MUX_POS);
    } else {
        return HAL_ERROR;
    }

    int serialize_byte_count = can_primary_api_serialize_from_id(tx_header.StdId, &message, buffer);
    if (serialize_byte_count < 0) {
        return HAL_ERROR;
    }
    tx_header.DLC = serialize_byte_count;
    return can_send(&CAR_CAN, buffer, &tx_header);
}

HAL_StatusTypeDef can_bms_send(uint16_t id) {
    // Return if busy
    if (can_forward && id != CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_FLASH)
        return HAL_BUSY;

    CAN_TxHeaderTypeDef tx_header = {
        .DLC = 0, .ExtId = 0, .IDE = CAN_ID_STD, .RTR = CAN_RTR_DATA, .StdId = id, .TransmitGlobalTime = DISABLE};
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH] = {0};

    union CanBmsMessages message = {0};

    if (id == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_SET_BALANCING_STATUS) {
        BalRequest request = bal_get_request();

        message.cellboard_set_balancing_status.balancingstatus = request.status;
        message.cellboard_set_balancing_status.target          = MAX(CELL_MIN_VOLTAGE, cell_voltage_get_min());
        message.cellboard_set_balancing_status.threshold       = request.threshold;
    } else if (id == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_FLASH) {
        message.cellboard_flash.id = flash_cellboard_id;
    } else {
        error_simple_set(ERROR_GROUP_ERROR_CAN, 0);
        return HAL_ERROR;
    }

    int serialize_byte_count = can_bms_api_serialize_from_id(id, &message, buffer);
    if (serialize_byte_count < 0) {
        return HAL_ERROR;
    }
    tx_header.DLC = serialize_byte_count;
    return can_send(&BMS_CAN, buffer, &tx_header);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rx_header           = {0};
    uint8_t rx_data[CAN_MAX_PAYLOAD_LENGTH] = {0};

    // Check for communication errors
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
        error_simple_set(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == BMS_CAN.Instance) {
        // Reset can errors
        error_simple_reset(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);

        // Forward data to the cellboards
        // if (rx_header.StdId >= BMS_FLASH_CELLBOARD_0_TX_FRAME_ID && rx_header.StdId <= BMS_FLASH_CELLBOARD_5_RX_FRAME_ID) {
        if (rx_header.StdId >= CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_OPENBLT_TX_0 &&
            rx_header.StdId <= CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_OPENBLT_RX_5) {
            CAN_TxHeaderTypeDef tx_header = {
                .DLC                = rx_header.DLC,
                .ExtId              = 0,
                .IDE                = CAN_ID_STD,
                .RTR                = CAN_RTR_DATA,
                .StdId              = rx_header.StdId,
                .TransmitGlobalTime = DISABLE};
            can_send(&CAR_CAN, rx_data, &tx_header);
            return;
        }

        union CanBmsMessages message = {0};

        int deserialize_result = can_bms_api_deserialize_from_id(rx_header.StdId, rx_data, &message);
        if (deserialize_result < 0) {
            error_simple_set(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);
            return;
        }

        if (rx_header.StdId == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_VOLTAGES) {
            // Reset time since last communication
            time_since_last_comm[message.cellboard_voltages.id] = HAL_GetTick();

            // Forward data
            CAN_TxHeaderTypeDef tx_header = {
                .DLC                = 0,
                .ExtId              = 0,
                .IDE                = CAN_ID_STD,
                .RTR                = CAN_RTR_DATA,
                .StdId              = CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_VOLTAGES,
                .TransmitGlobalTime = DISABLE};

            uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH]   = {0};
            union CanPrimaryMessages forward_message = {0};

            // Set start_index of the received cells between all cells of the pack
            forward_message.hv_bms_cellboard_voltages.cellboardid = message.cellboard_voltages.id;
            forward_message.hv_bms_cellboard_voltages.startindex  = message.cellboard_voltages.startindex;
            forward_message.hv_bms_cellboard_voltages.first_v     = message.cellboard_voltages.first_v;
            forward_message.hv_bms_cellboard_voltages.second_v    = message.cellboard_voltages.second_v;
            forward_message.hv_bms_cellboard_voltages.third_v     = message.cellboard_voltages.third_v;

            int serialize_byte_count = can_primary_api_serialize_from_id(tx_header.StdId, &forward_message, buffer);
            if (serialize_byte_count < 0) {
                return;
            }
            tx_header.DLC = serialize_byte_count;
            can_send(&CAR_CAN, buffer, &tx_header);
        } else if (rx_header.StdId == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_VOLTAGES_INFO) {
            uint8_t cellboard_id = message.cellboard_voltages_info.id;
            // Reset time since last communication
            time_since_last_comm[cellboard_id] = HAL_GetTick();

            cell_voltage_set_cells(
                cellboard_id,
                CONVERT_VOLTAGE_TO_VALUE(message.cellboard_voltages_info.min_v),
                CONVERT_VOLTAGE_TO_VALUE(message.cellboard_voltages_info.max_v),
                CONVERT_VOLTAGE_TO_VALUE(message.cellboard_voltages_info.average_v));
        } else if (rx_header.StdId == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_TEMPERATURES) {
            // Reset time since last communication
            time_since_last_comm[message.cellboard_temperatures.id] = HAL_GetTick();

            // TODO: Test
#if defined(TEMP_GROUP_ERROR_ENABLE) && defined(TEMP_ERROR_ENABLE)
            // Add error bit to the temp group
            size_t index = message.cellboard_temperatures.start_index / 4;
            size_t bit   = (index < 2) ? index : (index + ((index - 2) / 3 + 1));
            if (index % 3 == 1) {
                if (message.cellboard_temperatures.temp0 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    message.cellboard_temperatures.temp1 <= CELL_MIN_TEMPERATURE + 0.01f)
                    temp_errors[message.cellboard_temperatures.cellboard_id] |= 1 << bit;
                else
                    temp_errors[message.cellboard_temperatures.cellboard_id] &= ~(1 << bit);

                ++bit;
                if (message.cellboard_temperatures.temp2 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    message.cellboard_temperatures.temp3 <= CELL_MIN_TEMPERATURE + 0.01f)
                    temp_errors[message.cellboard_temperatures.cellboard_id] |= 1 << bit;
                else
                    temp_errors[message.cellboard_temperatures.cellboard_id] &= ~(1 << bit);
            } else {
                if (message.cellboard_temperatures.temp0 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    message.cellboard_temperatures.temp1 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    message.cellboard_temperatures.temp2 <= CELL_MIN_TEMPERATURE + 0.01f &&
                    message.cellboard_temperatures.temp3 <= CELL_MIN_TEMPERATURE + 0.01f)
                    temp_errors[message.cellboard_temperatures.cellboard_id] |= 1 << bit;
                else
                    temp_errors[message.cellboard_temperatures.cellboard_id] &= ~(1 << bit);
            }
            // Toggle temperatures connectors error
            bool is_error_set = false;
            /*
            for (size_t board_id = 0; board_id < CELLBOARD_COUNT && !is_error_set; board_id++) {
                for (size_t temp_bit = 0; temp_bit < TEMP_STRIPS_PER_BUS * 2; temp_bit += 2) {
                    if ((temp_errors[board_id] & (1 << temp_bit)) && (temp_errors[board_id] & (1 << (temp_bit + 1)))) {
                        error_simple_set(ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED, 2);
                        is_error_set = true;
                        break;
                    }
                }
            }
            */
            if (!is_error_set)
                error_simple_reset(ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED, 2);
#endif  // TEMP_GROUP_ERROR_ENABLE

            // Forward data
            CAN_TxHeaderTypeDef tx_header = {
                .DLC                = 0,
                .ExtId              = 0,
                .IDE                = CAN_ID_STD,
                .RTR                = CAN_RTR_DATA,
                .StdId              = CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_TEMPERATURES,
                .TransmitGlobalTime = DISABLE};

            uint8_t buffer[8]                        = {0};
            union CanPrimaryMessages forward_message = {0};

            // Set start_index of the received cells between all cells of the pack
            forward_message.hv_bms_cellboard_temperatures.cellboardid = message.cellboard_temperatures.id;
            forward_message.hv_bms_cellboard_temperatures.startindex  = message.cellboard_temperatures.startindex;
            forward_message.hv_bms_cellboard_temperatures.first_c     = message.cellboard_temperatures.first_c;
            forward_message.hv_bms_cellboard_temperatures.second_c    = message.cellboard_temperatures.second_c;
            forward_message.hv_bms_cellboard_temperatures.third_c     = message.cellboard_temperatures.third_c;
            forward_message.hv_bms_cellboard_temperatures.fourth_c    = message.cellboard_temperatures.fourth_c;

            int serialize_byte_count = can_primary_api_serialize_from_id(tx_header.StdId, &forward_message, buffer);
            if (serialize_byte_count < 0) {
                return;
            }
            tx_header.DLC = serialize_byte_count;
            can_send(&CAR_CAN, buffer, &tx_header);
        } else if (rx_header.StdId == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_TEMPERATURES_INFO) {
            // Reset time since last communication
            time_since_last_comm[message.cellboard_temperatures_info.id] = HAL_GetTick();

            temperature_set_cells(
                message.cellboard_temperatures_info.id,
                CONVERT_TEMPERATURE_TO_VALUE(message.cellboard_temperatures_info.min_c),
                CONVERT_TEMPERATURE_TO_VALUE(message.cellboard_temperatures_info.max_c),
                CONVERT_TEMPERATURE_TO_VALUE(message.cellboard_temperatures_info.average_c));
        } else if (rx_header.StdId == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_BOARD_STATUS) {
            // Reset the watchdog timer
            // watchdog_reset(rx_header.StdId);
            uint8_t cellboard_id = message.cellboard_board_status.id;

            // Reset time since last communication
            time_since_last_comm[cellboard_id] = HAL_GetTick();

            bal_update_status(cellboard_id, message.cellboard_board_status.balancing);

            // Check cellboard errors
            uint32_t error_status = 0;
            error_status |= message.cellboard_board_status.errorcancomm;
            error_status |= message.cellboard_board_status.errorltccomm;
            error_status |= message.cellboard_board_status.erroropenwire;
            // error_status |= message.cellboard_board_status.errortempcomm0;
            // error_status |= message.cellboard_board_status.errortempcomm1;
            // error_status |= message.cellboard_board_status.errortempcomm2;
            // error_status |= message.cellboard_board_status.errortempcomm3;
            // error_status |= message.cellboard_board_status.errortempcomm4;
            // error_status |= message.cellboard_board_status.errortempcomm5;

            if (error_status != 0) {
                error_simple_set(ERROR_GROUP_ERROR_CELLBOARD_INTERNAL, cellboard_id);
            } else {
                error_simple_reset(ERROR_GROUP_ERROR_CELLBOARD_INTERNAL, cellboard_id);
            }

            balancing_status[cellboard_id].cellboardid    = message.cellboard_board_status.id;
            balancing_status[cellboard_id].balancing_bool = message.cellboard_board_status.balancing;

            balancing_status[cellboard_id].errorcancomm   = message.cellboard_board_status.errorcancomm;
            balancing_status[cellboard_id].errorltccomm   = message.cellboard_board_status.errorltccomm;
            balancing_status[cellboard_id].erroropenwire  = message.cellboard_board_status.erroropenwire;
            balancing_status[cellboard_id].errortempcomm0 = message.cellboard_board_status.errortempcomm0;
            balancing_status[cellboard_id].errortempcomm1 = message.cellboard_board_status.errortempcomm1;
            balancing_status[cellboard_id].errortempcomm2 = message.cellboard_board_status.errortempcomm2;
            balancing_status[cellboard_id].errortempcomm3 = message.cellboard_board_status.errortempcomm3;
            balancing_status[cellboard_id].errortempcomm4 = message.cellboard_board_status.errortempcomm4;
            balancing_status[cellboard_id].errortempcomm5 = message.cellboard_board_status.errortempcomm5;

            balancing_status[cellboard_id].balancingcell0  = message.cellboard_board_status.balancingcell0;
            balancing_status[cellboard_id].balancingcell1  = message.cellboard_board_status.balancingcell1;
            balancing_status[cellboard_id].balancingcell2  = message.cellboard_board_status.balancingcell2;
            balancing_status[cellboard_id].balancingcell3  = message.cellboard_board_status.balancingcell3;
            balancing_status[cellboard_id].balancingcell4  = message.cellboard_board_status.balancingcell4;
            balancing_status[cellboard_id].balancingcell5  = message.cellboard_board_status.balancingcell5;
            balancing_status[cellboard_id].balancingcell6  = message.cellboard_board_status.balancingcell6;
            balancing_status[cellboard_id].balancingcell7  = message.cellboard_board_status.balancingcell7;
            balancing_status[cellboard_id].balancingcell8  = message.cellboard_board_status.balancingcell8;
            balancing_status[cellboard_id].balancingcell9  = message.cellboard_board_status.balancingcell9;
            balancing_status[cellboard_id].balancingcell10 = message.cellboard_board_status.balancingcell10;
            balancing_status[cellboard_id].balancingcell11 = message.cellboard_board_status.balancingcell11;
            balancing_status[cellboard_id].balancingcell12 = message.cellboard_board_status.balancingcell12;
            balancing_status[cellboard_id].balancingcell13 = message.cellboard_board_status.balancingcell13;
            balancing_status[cellboard_id].balancingcell14 = message.cellboard_board_status.balancingcell14;
            balancing_status[cellboard_id].balancingcell15 = message.cellboard_board_status.balancingcell15;
            balancing_status[cellboard_id].balancingcell16 = message.cellboard_board_status.balancingcell16;
            balancing_status[cellboard_id].balancingcell17 = message.cellboard_board_status.balancingcell17;
        } else if (rx_header.StdId == CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_VERSION) {
            // Reset time since last communication
            time_since_last_comm[message.cellboard_version.id] = HAL_GetTick();

            uint8_t cellboard_id = message.cellboard_version.id;
            cellboard_version[cellboard_id].id = cellboard_id;
            cellboard_version[cellboard_id].buildtime_s = message.cellboard_version.buildtime_s;
            cellboard_version[cellboard_id].canlibbuildtime_s = message.cellboard_version.canlibbuildtime_s;
        }
    }
}
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef rx_header           = {0};
    uint8_t rx_data[CAN_MAX_PAYLOAD_LENGTH] = {0};

    // Check for communication errors
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &rx_header, rx_data) != HAL_OK) {
        error_simple_set(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);
        cli_bms_debug("CAN: Error receiving message", 29);
        return;
    }

    if (hcan->Instance == CAR_CAN.Instance) {
        error_simple_reset(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);

        // if (rx_header.StdId >= BMS_FLASH_CELLBOARD_0_TX_FRAME_ID && rx_header.StdId <= BMS_FLASH_CELLBOARD_5_RX_FRAME_ID) {
        if (rx_header.StdId >= CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_OPENBLT_TX_0 &&
            rx_header.StdId <= CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_OPENBLT_RX_5) {
            CAN_TxHeaderTypeDef tx_header = {
                .DLC                = rx_header.DLC,
                .ExtId              = 0,
                .IDE                = CAN_ID_STD,
                .RTR                = CAN_RTR_DATA,
                .StdId              = rx_header.StdId,
                .TransmitGlobalTime = DISABLE};
            can_send(&BMS_CAN, rx_data, &tx_header);
            return;
        }
        // else if (rx_header.StdId == CAN_PRIMARY_MESSAGE_FRAME_ID_ECU_STATUS) {
        //     // Reset the watchdog timer
        //     watchdog_reset(rx_header.StdId);
        // }

        // Deserialize message
        union CanPrimaryMessages message = {0};
        int deserialize_result           = can_primary_api_deserialize_from_id(rx_header.StdId, rx_data, &message);
        if (deserialize_result < 0) {
            error_simple_set(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);
            return;
        }

        if (rx_header.StdId == CAN_PRIMARY_MESSAGE_FRAME_ID_ECU_SET_HV_BMS_STATUS) {
            // Request for TS status change
            set_ts_request.is_new     = true;
            set_ts_request.next_state = (message.ecu_set_hv_bms_status.targetstatus) ? STATE_WAIT_AIRN_CLOSE
                                                                                     : STATE_IDLE;
        } else if (rx_header.StdId == CAN_PRIMARY_MESSAGE_FRAME_ID_STEERING_WHEEL_SET_HV_BMS_BALANCING_STATUS) {
            // Send balancing request
            bal_change_status_request(
                message.steering_wheel_set_hv_bms_balancing_status.targetstatus,
                (voltage_t)message.steering_wheel_set_hv_bms_balancing_status.threshold * 10);
        }
        /*
        else if (rx_header.StdId == PRIMARY_HANDCART_STATUS_FRAME_ID) {
            primary_handcart_status_t raw_handcart_status            = {0};
            primary_handcart_status_converted_t conv_handcart_status = {0};

            if (primary_handcart_status_unpack(&raw_handcart_status, rx_data, PRIMARY_HANDCART_STATUS_BYTE_SIZE) < 0) {
                error_simple_set(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);
                return;
            }
            primary_handcart_status_raw_to_conversion_struct(&conv_handcart_status, &raw_handcart_status);

            // Reset watchdog
            watchdog_reset(PRIMARY_HANDCART_STATUS_FRAME_ID);

            is_handcart_connected = conv_handcart_status.connected;
        }
        */
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
                error_simple_set(ERROR_GROUP_ERROR_CAN, 1, HAL_GetTick());
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
        /*
        else if (rx_header.StdId == PRIMARY_HV_SET_FANS_STATUS_FRAME_ID) {
            primary_hv_set_fans_status_t raw_fans            = {0};
            primary_hv_set_fans_status_converted_t conv_fans = {0};

            if (primary_hv_set_fans_status_unpack(&raw_fans, rx_data, PRIMARY_HV_SET_FANS_STATUS_BYTE_SIZE) < 0) {
                error_simple_set(ERROR_GROUP_ERROR_CAN, hcan->Instance != BMS_CAN.Instance);
                return;
            }
            primary_hv_set_fans_status_raw_to_conversion_struct(&conv_fans, &raw_fans);

            // Set fans override and speed
            fans_set_override(conv_fans.fans_override);
            fans_set_speed(conv_fans.fans_speed);
        }
        */
        else if (rx_header.StdId == CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_FLASH) {
            bms_state_t state = fsm_get_state();
            if (message.hv_bms_flash.forward_bool) {
                flash_cellboard_id = message.hv_bms_flash.cellboardid;
                can_bms_send(CAN_BMS_MESSAGE_FRAME_ID_CELLBOARD_FLASH);
            } else if (
                (state == STATE_INIT || state == STATE_IDLE || state == STATE_FATAL_ERROR) && !bal_is_balancing())
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
            if (HAL_GetTick() - time_since_last_comm[i] >= CELLBOARD_COMM_TIMEOUT) {
                error_simple_set(ERROR_GROUP_ERROR_CELLBOARD_COMM, i);
            } else {
                error_simple_reset(ERROR_GROUP_ERROR_CELLBOARD_COMM, i);
            }
        }
    }
}
