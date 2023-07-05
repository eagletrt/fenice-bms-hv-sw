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

#include "error.h"
#include "mainboard_config.h"
#include "bms_fsm.h"
#include "can_comm.h"
#include "error_list_ref.h"
#include "cli_bms.h"

// Multiplexer feedback thresholds
#define FEEDBACK_MUX_ANALOG_THRESHOLD_H 2.2f
#define FEEDBACK_MUX_ANALOG_THRESHOLD_L 0.7f

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
#define FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_H 1.7f // V

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
#define FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(VALUE) ((VALUE) * (FEEDBACK_MUX_VREF / 4095))
#define FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(VALUE) ((VALUE) * (FEEDBACK_SD_VREF / 4095))
#define FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(VALUE) ((VALUE) * 4095 / FEEDBACK_MUX_VREF)
#define FEEDBACK_CONVERT_VOLTAGE_TO_ADC_SD(VALUE) ((VALUE) * 4095 / FEEDBACK_SD_VREF)

#define FEEDBACK_UPDATE_THRESHOLD_MS (MUX_INTERVAL_MS * (FEEDBACK_MUX_N + 2))


struct {
    uint16_t voltages[FEEDBACK_N];
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
        // Save DMA data
        feedbacks.voltages[fb_index] = dma_data[DMA_DATA_MUX_FB];
        feedbacks.voltages[FEEDBACK_SD_IN_POS]  = dma_data[DMA_DATA_SD_IN];
        feedbacks.voltages[FEEDBACK_SD_OUT_POS] = dma_data[DMA_DATA_SD_OUT];
        feedbacks.voltages[FEEDBACK_SD_BMS_POS] = dma_data[DMA_DATA_SD_BMS];
        feedbacks.voltages[FEEDBACK_SD_IMD_POS] = dma_data[DMA_DATA_SD_IMD];

        fb_index = (fb_index + 1) % FEEDBACK_MUX_N;
        feedbacks.has_converted = true;
    }
}

void feedback_init() {
    // Start feedbacks timer
    __HAL_TIM_CLEAR_IT(&HTIM_MUX, TIM_IT_UPDATE);
    __HAL_TIM_SET_AUTORELOAD(&HTIM_MUX, TIM_MS_TO_TICKS(&HTIM_MUX, MUX_INTERVAL_MS));
    HAL_TIM_Base_Start_IT(&HTIM_MUX);

    // Start microseconds timer
    // HAL_TIM_Base_Start(&HTIM_US);

    for (size_t i = 0; i < FEEDBACK_N; i++)
        feedbacks.voltages[i] = 0;
    feedbacks.has_converted = true;
}

// TODO: Check all feedbacks and do not return false immediately
bool feedback_is_ok(feedback_t mask, feedback_t value) {
    bool is_low = false;
    bool is_high = false;

    for (size_t i = 0; i < FEEDBACK_N; i++) {
        feedback_t feedback = 1 << i;

        // Skip feedbacks not in mask
        if ((mask & feedback) == 0)
            continue;

        feedback_t fb_val = feedback & value;

        // Check handcart
        if (is_handcart_connected && i == FEEDBACK_IMD_FAULT_POS) {
            // Check IMD fault for handcart
            is_low = feedbacks.voltages[i] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L);
            is_high = feedbacks.voltages[i] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_H);
        }
        else if (is_handcart_connected && i == FEEDBACK_CHECK_MUX_POS) {
            // Check multiplexer VDC for handcart
            is_low = feedbacks.voltages[i] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_H);
            is_high = feedbacks.voltages[i] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_L);

            // TODO: Check for low state
            if (is_low && is_high)
                error_reset(ERROR_FEEDBACK, i);
            else {
                error_set(ERROR_FEEDBACK, i, HAL_GetTick());
                return false;
            }
        }
        else if (i == FEEDBACK_CHECK_MUX_POS) {
            // Check multiplexer VDC
            is_low = feedbacks.voltages[i] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_H);
            is_high = feedbacks.voltages[i] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_L);

            // TODO: Check for low state
            if (is_low && is_high)
                error_reset(ERROR_FEEDBACK, i);
            else {
                error_set(ERROR_FEEDBACK, i, HAL_GetTick());
                return false;
            }
        }
        else if (i < FEEDBACK_MUX_N) {
            is_low = feedbacks.voltages[i] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_L);
            is_high = feedbacks.voltages[i] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_H);
        }
        else if (i < FEEDBACK_N) {
            is_low = feedbacks.voltages[i] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_SD_ANALOG_THRESHOLD_L);
            is_high = feedbacks.voltages[i] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_SD_ANALOG_THRESHOLD_H);
        }
        else {
            error_set(ERROR_FEEDBACK_CIRCUITRY, i, HAL_GetTick());
            return false;
        }

        // Check for errors
        if ((fb_val && is_high) || (!fb_val && is_low))
            error_reset(ERROR_FEEDBACK, i);
        else {
            error_set(ERROR_FEEDBACK, i, HAL_GetTick());
            return false;
        }

        error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
    }

    return true;
}

feedback_feed_t feedback_get_state(size_t index) {
    feedback_feed_t feed;
    // Set voltage
    feed.voltage = (index < FEEDBACK_MUX_N) ?
        FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks.voltages[index]) :
        FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks.voltages[index]);
    
    // Check handcart
    if (is_handcart_connected && index == FEEDBACK_IMD_FAULT_POS) {
        // Check IMD fault for handcart
        if (feedbacks.voltages[index] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L))
            feed.state = FEEDBACK_STATE_L;
        else if (feedbacks.voltages[index] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_H))
            feed.state = FEEDBACK_STATE_H;
        else
            feed.state = FEEDBACK_STATE_ERROR;
    }
    else if (is_handcart_connected && index == FEEDBACK_CHECK_MUX_POS) {
        // Check multiplexer VDC for handcart
        if (feedbacks.voltages[index] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_H) &&
                feedbacks.voltages[index] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_L))
            feed.state = FEEDBACK_STATE_H;
        else
            feed.state = FEEDBACK_STATE_ERROR;
    }
    else if (index == FEEDBACK_CHECK_MUX_POS) {
        // Check multiplexer VDC
        if (feedbacks.voltages[index] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_H) &&
                feedbacks.voltages[index] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_L))
            feed.state = FEEDBACK_STATE_H;
        else
            feed.state = FEEDBACK_STATE_ERROR;
    }
    else if (index < FEEDBACK_MUX_N) {
        if (feedbacks.voltages[index] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_L))
            feed.state = FEEDBACK_STATE_L;
        else if (feedbacks.voltages[index] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_H))
            feed.state = FEEDBACK_STATE_H;
        else
            feed.state = FEEDBACK_STATE_ERROR;
    }
    else if (index < FEEDBACK_N) {
        if (feedbacks.voltages[index] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_SD_ANALOG_THRESHOLD_L))
            feed.state = FEEDBACK_STATE_L;
        else if (feedbacks.voltages[index] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_SD_ANALOG_THRESHOLD_H))
            feed.state = FEEDBACK_STATE_H;
        else
            feed.state = FEEDBACK_STATE_ERROR;
    }

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