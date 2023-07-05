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
    uint32_t timestamp[FEEDBACK_N];
} feedbacks;

uint16_t dma_data[DMA_DATA_SIZE] = { 0 };
static uint8_t fb_index = 0;

/** @brief Set the multiplexer index */
void _feedback_set_mux_index(uint8_t index) {
    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (index & 0b00000001));
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (index & 0b00000010));
    HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (index & 0b00000100));
    HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (index & 0b00001000));
}
void _feedback_handle_tim_elapsed_irq() {
    _feedback_set_mux_index(fb_index);
    HAL_ADC_Start_DMA(&ADC_MUX, (uint32_t *)dma_data, DMA_DATA_SIZE);
}
void _feedback_handle_adc_cnv_cmpl_irq() {
    // Save DMA data
    feedbacks.voltages[fb_index] = dma_data[DMA_DATA_MUX_FB];
    feedbacks.voltages[FEEDBACK_SD_IN_POS]  = dma_data[DMA_DATA_SD_IN];
    feedbacks.voltages[FEEDBACK_SD_OUT_POS] = dma_data[DMA_DATA_SD_OUT];
    feedbacks.voltages[FEEDBACK_SD_BMS_POS] = dma_data[DMA_DATA_SD_BMS];
    feedbacks.voltages[FEEDBACK_SD_IMD_POS] = dma_data[DMA_DATA_SD_IMD];

    feedbacks.timestamp[fb_index] = HAL_GetTick();
    feedbacks.timestamp[FEEDBACK_SD_IN_POS] = HAL_GetTick();
    feedbacks.timestamp[FEEDBACK_SD_OUT_POS] = HAL_GetTick();
    feedbacks.timestamp[FEEDBACK_SD_BMS_POS] = HAL_GetTick();
    feedbacks.timestamp[FEEDBACK_SD_IMD_POS] = HAL_GetTick();

    fb_index = (fb_index + 1) % FEEDBACK_MUX_N;
}

void feedback_init() {
    // Start feedbacks timer
    __HAL_TIM_CLEAR_IT(&HTIM_MUX, TIM_IT_UPDATE);
    __HAL_TIM_SET_AUTORELOAD(&HTIM_MUX, TIM_MS_TO_TICKS(&HTIM_MUX, MUX_INTERVAL_MS));
    HAL_TIM_Base_Start_IT(&HTIM_MUX);

    for (size_t i = 0; i < FEEDBACK_N; i++) {
        feedbacks.voltages[i] = 0;
        feedbacks.timestamp[i] = 0;
    }
}
bool feedback_need_update() {
    for (size_t i = 0; i < FEEDBACK_N; i++) {
        if (feedbacks.timestamp[i] >= FEEDBACK_UPDATE_THRESHOLD_MS)
            return true;
    }
    return false;
}
bool feedback_check_mux_vdc(bool handcart_connected) {
    bool is_mux_ok;
    if (!handcart_connected)
        is_mux_ok = feedbacks.voltages[FEEDBACK_CHECK_MUX_POS] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_L) &&
            feedbacks.voltages[FEEDBACK_CHECK_MUX_POS] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_THRESHOLD_H);
    else
        is_mux_ok = feedbacks.voltages[FEEDBACK_CHECK_MUX_POS] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_L) &&
            feedbacks.voltages[FEEDBACK_CHECK_MUX_POS] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_H);
    
    error_toggle_check(is_mux_ok, ERROR_FEEDBACK, FEEDBACK_CHECK_MUX_POS);
    return is_mux_ok;
}
feedback_t feedback_check_mux(feedback_t value, bool handcart_connected) {
    feedback_t errors = 0;
    
    for (size_t i = 0; i < FEEDBACK_MUX_N; i++) {
        if (i == FEEDBACK_CHECK_MUX_POS)
            continue;

        feedback_t feedback = 1 << i;
        bool fb_val = (value & feedback) != 0;
        bool is_low = false;
        bool is_high = false;

        // Check feedbacks
        if (handcart_connected && i == FEEDBACK_IMD_FAULT_POS) {
            // Check IMD fault
            is_low = feedbacks.voltages[i] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L);
            is_high = feedbacks.voltages[i] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_H);
        }
        else {
            // Check other feedbacks in the multiplexer
            is_low = feedbacks.voltages[i] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_L);
            is_high = feedbacks.voltages[i] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_H);
        }
        if ((is_low && !fb_val) || (is_high && fb_val))
            error_reset(ERROR_FEEDBACK, i);
        else {
            errors |= feedback;
            error_set(ERROR_FEEDBACK, i, HAL_GetTick());
        }
        error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
    }

    return errors;
}
feedback_t feedback_check_sd(feedback_t value) {
    feedback_t errors = 0;
    
    for (size_t i = FEEDBACK_MUX_N; i < FEEDBACK_N; i++) {
        feedback_t feedback = 1 << i;
        bool fb_val = (value & feedback) != 0;        
        
        // Check external shutdown feedbacks
        bool is_low = feedbacks.voltages[i] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_SD(FEEDBACK_MUX_ANALOG_THRESHOLD_L);
        bool is_high = feedbacks.voltages[i] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_SD(FEEDBACK_MUX_ANALOG_THRESHOLD_H);
        if ((is_low && !fb_val) || (is_high && fb_val))
            error_reset(ERROR_FEEDBACK, i);
        else {
            errors |= feedback;
            error_set(ERROR_FEEDBACK, i, HAL_GetTick());
        }
        error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
    }

    return errors;
}
feedback_feed_t feedback_get_state(size_t index, bool handcart_connected) {
    feedback_feed_t feed;
    // Set voltage
    feed.voltage = (index < FEEDBACK_MUX_N) ?
        FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks.voltages[index]) :
        FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks.voltages[index]);
    
    // Set status
    if (index == FEEDBACK_CHECK_MUX_POS)
        feed.state = feedback_check_mux_vdc(handcart_connected) ? FEEDBACK_STATE_H : FEEDBACK_STATE_ERROR; // Mulitplexer VDC status
    else {
        // Handcart IMD fault status
        if (handcart_connected && index == FEEDBACK_IMD_FAULT_POS) {
            if (feedbacks.voltages[index] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L))
                feed.state = FEEDBACK_STATE_L;
            else if (feedbacks.voltages[index] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_H))
                feed.state = FEEDBACK_STATE_H;
            else
                feed.state = FEEDBACK_STATE_ERROR;
        }
        else {
            if (index < FEEDBACK_MUX_N) {
                // Multiplexer feedbacks status
                if (feedbacks.voltages[index] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_L))
                    feed.state = FEEDBACK_STATE_L;
                else if (feedbacks.voltages[index] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_MUX(FEEDBACK_MUX_ANALOG_THRESHOLD_H))
                    feed.state = FEEDBACK_STATE_H;
                else
                    feed.state = FEEDBACK_STATE_ERROR;
            }
            else {
                // Shutdown feedbacks status
                if (feedbacks.voltages[index] <= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_SD(FEEDBACK_SD_ANALOG_THRESHOLD_L))
                    feed.state = FEEDBACK_STATE_L;
                else if (feedbacks.voltages[index] >= FEEDBACK_CONVERT_VOLTAGE_TO_ADC_SD(FEEDBACK_SD_ANALOG_THRESHOLD_H))
                    feed.state = FEEDBACK_STATE_H;
                else
                    feed.state = FEEDBACK_STATE_ERROR;
            }
        }
    }

    return feed;
}
void feedback_get_all_states(feedback_feed_t out_value[FEEDBACK_N], bool handcart_connected) {
    for (size_t i = 0; i < FEEDBACK_N; i++)
        out_value[i] = feedback_get_state(i, handcart_connected);
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