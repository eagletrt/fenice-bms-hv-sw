/**
 * @file feedback.c
 * @brief Feedback parsing utilities
 *
 * @date Mar 16, 2021
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "feedback.h"

#include <string.h>

#include "error.h"
#include "mainboard_config.h"
#include "bms_fsm.h"
#include "can_comm.h"
#include "cli_bms.h"
#include "timer_utils.h"
#include "error/error-handler.h"

// Multiplexer feedback thresholds
#define FEEDBACK_MUX_ANALOG_THRESHOLD_H 1.9f // 2.2f
#define FEEDBACK_MUX_ANALOG_THRESHOLD_L 0.7f // TODO: Check AIRN_STATUS and AIRP_STATUS slightly above threshold

// Shutdown feedbacks threshold
#define FEEDBACK_SD_THRESHOLD_H 10.0f // V
#define FEEDBACK_SD_THRESHOLD_L 1.0f  // V
#define FEEDBACK_SD_DIVIDER_RATIO 0.233f // V
#define FEEDBACK_SD_ANALOG_THRESHOLD_L (FEEDBACK_SD_DIVIDER_RATIO * FEEDBACK_SD_THRESHOLD_L)
#define FEEDBACK_SD_ANALOG_THRESHOLD_H (FEEDBACK_SD_DIVIDER_RATIO * FEEDBACK_SD_THRESHOLD_H)

// VDC feedback threshold
#define FEEDBACK_CHECK_MUX_THRESHOLD_L 2.4f // V
#define FEEDBACK_CHECK_MUX_THRESHOLD_H 3.4f // V

// Handcart feedback thresholds
#define FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L 0.7f // V
#define FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_H 1.5f // V
#define FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_L 1.3f // V
#define FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_H 1.8f // 1.7f // V

#define MUX_INTERVAL_MS 0.1 // ms
#define DMA_DATA_SIZE 5

// Dma data indices
#define DMA_DATA_MUX_FB 3
#define DMA_DATA_SD_OUT 4
#define DMA_DATA_SD_IN  2
#define DMA_DATA_SD_BMS 1
#define DMA_DATA_SD_IMD 0

#define FEEDBACK_MUX_VREF 3.3f // V
#define FEEDBACK_SD_VREF 13.f // V
#define FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(VALUE) ((VALUE) * (FEEDBACK_MUX_VREF / 4095.f))
#define FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(VALUE) ((VALUE) * (FEEDBACK_SD_VREF / 4095.f))
#define FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(VALUE) ((uint16_t)((VALUE) * 4095 / FEEDBACK_MUX_VREF))
#define FEEDBACK_CONVERT_VOLTAGE_TO_ADC_SD(VALUE) ((uint16_t)((VALUE) * 4095 / FEEDBACK_SD_VREF))

#define FEEDBACK_UPDATE_THRESHOLD_MS (MUX_INTERVAL_MS * (FEEDBACK_MUX_N + 2))

#define FEEDBACK_QUEUE_SIZE 5

struct {
    uint16_t voltages[FEEDBACK_N][FEEDBACK_QUEUE_SIZE];
    uint8_t index[FEEDBACK_N];
    bool has_converted;
} feedbacks;

uint8_t fb_index;
static uint16_t dma_data[DMA_DATA_SIZE] = { 0 };

/** @brief Set the multiplexer index */
void _feedback_set_mux_index(uint8_t index) {
    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (index & 0b00000001));
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (index & 0b00000010));
    HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (index & 0b00000100));
    HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (index & 0b00001000));
}
void _feedback_handle_tim_elapsed_irq() {
    if (feedbacks.has_converted) {
        _feedback_set_mux_index(fb_index);
        feedbacks.has_converted = false;
        HAL_ADC_Start_DMA(&ADC_MUX, (uint32_t *)dma_data, DMA_DATA_SIZE);
    }
}
void _feedback_handle_adc_cnv_cmpl_irq() {
    if (!feedbacks.has_converted) {
        size_t j = feedbacks.index[fb_index];

        // Save DMA data
        feedbacks.voltages[fb_index][j] = dma_data[DMA_DATA_MUX_FB];
        feedbacks.voltages[FEEDBACK_SD_IN_POS][feedbacks.index[FEEDBACK_SD_IN_POS]]   = dma_data[DMA_DATA_SD_IN];
        feedbacks.voltages[FEEDBACK_SD_OUT_POS][feedbacks.index[FEEDBACK_SD_OUT_POS]] = dma_data[DMA_DATA_SD_OUT];
        feedbacks.voltages[FEEDBACK_SD_BMS_POS][feedbacks.index[FEEDBACK_SD_BMS_POS]] = dma_data[DMA_DATA_SD_BMS];
        feedbacks.voltages[FEEDBACK_SD_IMD_POS][feedbacks.index[FEEDBACK_SD_IMD_POS]] = dma_data[DMA_DATA_SD_IMD];

        // Update queue indices
        feedbacks.index[fb_index] = (feedbacks.index[fb_index] + 1) % FEEDBACK_QUEUE_SIZE;
        feedbacks.index[FEEDBACK_SD_IN_POS]  = (feedbacks.index[FEEDBACK_SD_IN_POS]  + 1) % FEEDBACK_QUEUE_SIZE;
        feedbacks.index[FEEDBACK_SD_OUT_POS] = (feedbacks.index[FEEDBACK_SD_OUT_POS] + 1) % FEEDBACK_QUEUE_SIZE;
        feedbacks.index[FEEDBACK_SD_BMS_POS] = (feedbacks.index[FEEDBACK_SD_BMS_POS] + 1) % FEEDBACK_QUEUE_SIZE;
        feedbacks.index[FEEDBACK_SD_IMD_POS] = (feedbacks.index[FEEDBACK_SD_IMD_POS] + 1) % FEEDBACK_QUEUE_SIZE;

        fb_index = (fb_index + 1) % FEEDBACK_MUX_N;
        feedbacks.has_converted = true;
    }
}

FEEDBACK_STATE _feedback_get_majority(size_t low, size_t high, size_t error) {
    if (low > high && low > error)
        return FEEDBACK_STATE_L;
    if (high > low && high > error)
        return FEEDBACK_STATE_H;
    return FEEDBACK_STATE_ERROR;
}

void feedback_init() {
    // Start feedbacks timer
    __HAL_TIM_CLEAR_IT(&HTIM_MUX, TIM_IT_UPDATE);
    __HAL_TIM_SET_AUTORELOAD(&HTIM_MUX, TIM_MS_TO_TICKS(&HTIM_MUX, MUX_INTERVAL_MS));
    HAL_TIM_Base_Start_IT(&HTIM_MUX);

    // Start microseconds timer
    // HAL_TIM_Base_Start(&HTIM_US);

    for (size_t i = 0; i < FEEDBACK_N; i++) {
        memset(feedbacks.voltages[i], 0, FEEDBACK_QUEUE_SIZE * sizeof(uint16_t));
        feedbacks.index[i] = 0;
    }
    feedbacks.has_converted = true;
}

float feedback_get_voltage(size_t index) {
    if (index < FEEDBACK_MUX_N)
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks.voltages[index][feedbacks.index[index]]);
    else if (index < FEEDBACK_N)
        return FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks.voltages[index][feedbacks.index[index]]);
    return 0.f;
}

// TODO: Check all feedbacks and do not return false immediately
bool feedback_is_ok(feedback_t mask, feedback_t value) {
    for (size_t i = 0; i < FEEDBACK_N; i++) {
        size_t low = 0, high = 0, error = 0;
        feedback_t feedback = 1 << i;

        // Skip feedbacks not in mask
        if ((mask & feedback) == 0)
            continue;

        feedback_t fb_val = feedback & value;

        // Check handcart
        if (is_handcart_connected && i == FEEDBACK_IMD_FAULT_POS) {
            for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
                // Check IMD fault for handcart
                if (feedbacks.voltages[i][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L))
                    low++;
                else if (feedbacks.voltages[i][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_H))
                    high++;
                else
                    error++;
            }
        }
        else if (is_handcart_connected && i == FEEDBACK_CHECK_MUX_POS) {
            for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
                // Check multiplexer VDC for handcart
                if (feedbacks.voltages[i][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_H))
                    error++;
                else if (feedbacks.voltages[i][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_L))
                    error++;
                else
                    high++;
            }

            FEEDBACK_STATE state = _feedback_get_majority(low, high, error);

            // TODO: Check for low state
            if (state == FEEDBACK_STATE_H)
                error_reset(ERROR_FEEDBACK, i);
            else {
                error_set(ERROR_FEEDBACK, i, HAL_GetTick());
                conv_debug.debug_signals_feedback_check_mux = 1;
                return false;
            }
            continue;
        }
        else if (i == FEEDBACK_CHECK_MUX_POS) {
            for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
                // Check multiplexer VDC
                if (feedbacks.voltages[i][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_H))
                    error++;
                else if(feedbacks.voltages[i][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_L))
                    error++;
                else
                    high++;
            }

            FEEDBACK_STATE state = _feedback_get_majority(low, high, error);

            // TODO: Check for low state
            if (state == FEEDBACK_STATE_H)
                error_reset(ERROR_FEEDBACK, i);
            else {
                error_set(ERROR_FEEDBACK, i, HAL_GetTick());
                conv_debug.debug_signals_feedback_check_mux = 1;
                return false;
            }
            continue;
        }
        else if (i < FEEDBACK_MUX_N) {
            for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
                if (feedbacks.voltages[i][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_L))
                    low++;
                else if (feedbacks.voltages[i][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_H))
                    high++;
                else
                    error++;
            }
        }
        else if (i < FEEDBACK_N) {
            for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
                if (feedbacks.voltages[i][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_SD_ANALOG_THRESHOLD_L))
                    low++;
                else if(feedbacks.voltages[i][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_SD_ANALOG_THRESHOLD_H))
                    high++;
                else
                    error++;
            }
        }
        else {
            error_set(ERROR_FEEDBACK_CIRCUITRY, i, HAL_GetTick());
            return false;
        }

        FEEDBACK_STATE state = _feedback_get_majority(low, high, error);

        // Check for errors
        if (state == FEEDBACK_STATE_ERROR || (fb_val && state == FEEDBACK_STATE_L) || (!fb_val && state == FEEDBACK_STATE_H)) {
            error_set(ERROR_FEEDBACK, i, HAL_GetTick());
            if (fsm_get_state() == STATE_TS_ON) {
                switch (i) {
                    case FEEDBACK_IMPLAUSIBILITY_DETECTED:
                        conv_debug.debug_signals_feedback_implausibility_detected = 1;
                        break;
                    case FEEDBACK_IMD_COCKPIT:
                        conv_debug.debug_signals_feedback_imd_cockpit = 1;
                        break;
                    case FEEDBACK_TSAL_GREEN_FAULT_LATCHED:
                        conv_debug.debug_signals_feedback_tsal_green_fault_latched = 1;
                        break;
                    case FEEDBACK_BMS_COCKPIT:
                        conv_debug.debug_signals_feedback_bms_cockpit = 1;
                        break;
                    case FEEDBACK_EXT_LATCHED:
                        conv_debug.debug_signals_feedback_ext_latched = 1;
                        break;
                    case FEEDBACK_TSAL_GREEN:
                        conv_debug.debug_signals_feedback_tsal_green = 1;
                        break;
                    case FEEDBACK_TS_OVER_60V_STATUS:
                        conv_debug.debug_signals_feedback_ts_over_60v_status = 1;
                        break;
                    case FEEDBACK_AIRN_STATUS:
                        conv_debug.debug_signals_feedback_airn_status = 1;
                        break;
                    case FEEDBACK_AIRP_STATUS:
                        conv_debug.debug_signals_feedback_airp_status = 1;
                        break;
                    case FEEDBACK_AIRP_GATE:
                        conv_debug.debug_signals_feedback_airp_gate = 1;
                        break;
                    case FEEDBACK_AIRN_GATE:
                        conv_debug.debug_signals_feedback_airn_gate = 1;
                        break;
                    case FEEDBACK_PRECHARGE_STATUS:
                        conv_debug.debug_signals_feedback_precharge_status = 1;
                        break;
                    case FEEDBACK_TSP_OVER_60V_STATUS:
                        conv_debug.debug_signals_feedback_tsp_over_60v_status = 1;
                        break;
                    case FEEDBACK_IMD_FAULT:
                        conv_debug.debug_signals_feedback_imd_fault = 1;
                        break;
                    case FEEDBACK_CHECK_MUX:
                        conv_debug.debug_signals_feedback_check_mux = 1;
                        break;
                    case FEEDBACK_SD_END:
                        conv_debug.debug_signals_feedback_sd_end = 1;
                        break;
                    case FEEDBACK_SD_OUT:
                        conv_debug.debug_signals_feedback_sd_out = 1;
                        break;
                    case FEEDBACK_SD_IN:
                        conv_debug.debug_signals_feedback_sd_in = 1;
                        break;
                    case FEEDBACK_SD_BMS:
                        conv_debug.debug_signals_feedback_sd_bms = 1;
                        break;
                    case FEEDBACK_SD_IMD:
                        conv_debug.debug_signals_feedback_sd_imd = 1;
                        break;

                    default:
                        break;
                }
            }
            return false;
        }
        else
            error_reset(ERROR_FEEDBACK, i);
        error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
    }

    return true;
}

feedback_feed_t feedback_get_state(size_t index) {
    size_t queue_index = feedbacks.index[index];
    size_t low = 0, high = 0, error = 0;

    feedback_feed_t feed = { 0 };
    // Set voltage
    feed.voltage = (index < FEEDBACK_MUX_N) ?
        FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks.voltages[index][queue_index]) :
        FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks.voltages[index][queue_index]);
    
    // Check handcart
    if (is_handcart_connected && index == FEEDBACK_IMD_FAULT_POS) {
        for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
            // Check IMD fault for handcart
            if (feedbacks.voltages[index][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L)) {
                low++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_L;
            }
            else if (feedbacks.voltages[index][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_H)) {
                high++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_H;
            }
            else {
                error++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_ERROR;
            }
        }
    }
    else if (is_handcart_connected && index == FEEDBACK_CHECK_MUX_POS) {
        for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
            // Check multiplexer VDC for handcart
            if (feedbacks.voltages[index][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_H)) {
                error++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_ERROR;
            }
            else if (feedbacks.voltages[index][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_L)) {
                error++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_ERROR;
            }
            else {
                high++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_H;
            }
        }

        feed.real_state = _feedback_get_majority(low, high, error);
        return feed;
    }
    else if (index == FEEDBACK_CHECK_MUX_POS) {
        for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
            // Check multiplexer VDC
            if (feedbacks.voltages[index][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_H)) {
                error++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_ERROR;
            }
            else if(feedbacks.voltages[index][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_L)) {
                error++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_ERROR;
            }
            else {
                high++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_H;
            }
        }
        
        feed.real_state = _feedback_get_majority(low, high, error);
        return feed;
    }
    else if (index < FEEDBACK_MUX_N) {
        for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
            if (feedbacks.voltages[index][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_L)) {
                low++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_L;
            }
            else if (feedbacks.voltages[index][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_H)) {
                high++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_H;
            }
            else {
                error++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_ERROR;
            }
        }
    }
    else if (index < FEEDBACK_N) {
        for (size_t j = 0; j < FEEDBACK_QUEUE_SIZE; j++) {
            if (feedbacks.voltages[index][j] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_SD_ANALOG_THRESHOLD_L)) {
                low++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_L;
            }
            else if(feedbacks.voltages[index][j] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_SD_ANALOG_THRESHOLD_H)) {
                high++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_H;
            }
            else {
                error++;
                if (j == queue_index)
                    feed.cur_state = FEEDBACK_STATE_ERROR;
            }
        }
    }

    feed.real_state = _feedback_get_majority(low, high, error);
    return feed;
}
void feedback_get_all_states(feedback_feed_t out_value[FEEDBACK_N]) {
    for (size_t i = 0; i < FEEDBACK_N; i++)
        out_value[i] = feedback_get_state(i);
}

// bool _is_adc_value_low(uint8_t index) {
//     if (index == FEEDBACK_CHECK_MUX_POS) {
//         if (is_handcart_connected)
//             return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_L;
//         return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_CHECK_MUX_THRESHOLD_L;
//     }
//     if (index == FEEDBACK_IMD_FAULT_POS && is_handcart_connected)
//         return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L;
//     if (index == FEEDBACK_SD_END_POS)
//         return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < 0.5f;
//     if (index == FEEDBACK_SD_IN_POS || index == FEEDBACK_SD_OUT_POS)
//         return FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[index]) < 0.5f;
// 
//     if (index < FEEDBACK_MUX_N)
//         return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_MUX_ANALOG_THRESHOLD_L;
//     return FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_SD_ANALOG_THRESHOLD_L;
// }
