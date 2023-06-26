/**
 * @file		feedback.c
 * @brief		Feedback parsing utilities
 *
 * @date		Mar 16, 2021
 * @author  	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author  	Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#include "feedback.h"

#include <stdbool.h>

#include "error.h"
#include "mainboard_config.h"


#define feedback_analog_threshold_h    9.5f / 4.3f
#define feedback_analog_threshold_l    0.4f / 4.3f
#define feedback_check_mux_threshold_h 3.5f / 4.3f
#define feedback_check_mux_threshold_l 3.1f / 4.3f

uint16_t feedbacks[FEEDBACK_N] = { 0 };
uint8_t fb_index = 0;
uint16_t dma_data = 0;

// TODO: Check SD_IN, SD_OUT, SD_BMS_FB, SD_IMD_FB
// if (index == FEEDBACK_SD_IN_POS || index == FEEDBACK_SD_OUT_POS) {
//     return val < 0.5f;
// }

/**
 * @brief Check if a MUX voltage greater than the high threshold
 * 
 * @param index The index of the multiplexed value
 * @return true If the voltage is greater than the threshold
 * @return false Otherwise
 */
bool _is_adc_value_high(uint8_t index) {
    float val = FEEDBACK_CONVERT_ADC_TO_VOLTAGE(feedbacks[index]);
    return val > feedback_analog_threshold_h;
}
/**
 * @brief Check if MUX voltage is lower than the low threshold
 * 
 * @param index The index of the multiplexed value
 * @return true If the voltage is lower than the threshold
 * @return false Otherwise
 */
bool _is_adc_value_low(uint8_t index) {
    float val = FEEDBACK_CONVERT_ADC_TO_VOLTAGE(feedbacks[index]);
    if (index == FEEDBACK_SD_END_POS)
        return val < 0.5f;
    return val < feedback_analog_threshold_l;
}
/**
 * @brief Check if the MUX voltage is valid
 * 
 * @param index The index of the multiplexed value
 * @return true If the voltage is valid
 * @return false Otherwise
 */
bool _is_adc_value_valid(uint8_t index) {
    return _is_adc_value_high(index) || _is_adc_value_low(index);
}
/**
 * @brief Check if the multiplexer feedback voltage is in the correct voltage range
 * 
 * @return true The MUX voltage is in the correct range
 * @return false Otherwise
 */
bool _is_check_mux_ok() {
    return !_is_adc_value_valid(FEEDBACK_CHECK_MUX_POS);
}


void feedback_init() {
    // Start feedbacks timer
    __HAL_TIM_CLEAR_IT(&HTIM_MUX, TIM_IT_UPDATE);
    __HAL_TIM_SET_AUTORELOAD(&HTIM_MUX, TIM_MS_TO_TICKS(&HTIM_MUX, MUX_INTERVAL_MS));
    HAL_TIM_Base_Start_IT(&HTIM_MUX);
}
void feedback_get_feedback_states(feedback_feed_t out_value[FEEDBACK_N]) {
    for (uint8_t i = 0; i < FEEDBACK_N; ++i) {

        // Get feedback voltage
        out_value[i].voltage = FEEDBACK_CONVERT_ADC_TO_VOLTAGE(feedbacks[i]);

        // Multiplexer feedback
        if (i == FEEDBACK_CHECK_MUX_POS) {
            out_value[i].state = _is_check_mux_ok() ? FEEDBACK_STATE_H : FEEDBACK_STATE_ERROR;
        }
        else {
            // Check feedback state
            if (_is_adc_value_high(i))
                out_value[i].state = FEEDBACK_STATE_H;
            else if (_is_adc_value_low(i))
                out_value[i].state = FEEDBACK_STATE_L;
            else
                out_value[i].state = FEEDBACK_STATE_ERROR;
        }
    }
}
feedback_t feedback_check(feedback_t fb_check_mask, feedback_t fb_value) {
    feedback_t differences = 0;

    for (uint8_t i = 0; i < FEEDBACK_N; ++i) {
        feedback_t fb_i     = 1U << i;
        feedback_t fb_i_val = fb_value & fb_i;

        if (fb_i & fb_check_mask) {
            // Check multiplexer
            if (i == FEEDBACK_CHECK_MUX_POS) {
                if (_is_check_mux_ok())
                    error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
                else {
                    error_set(ERROR_FEEDBACK_CIRCUITRY, i, HAL_GetTick());
                    differences |= fb_i;
                }
            }
            else {
                // Check if feedback voltage is valid
                if (_is_adc_value_valid(i)) {
                    if ((fb_i_val && _is_adc_value_high(i)) || (!fb_i_val && _is_adc_value_low(i)))
                        error_reset(ERROR_FEEDBACK, i);
                    else {
                        error_set(ERROR_FEEDBACK, i, HAL_GetTick());
                        differences |= fb_i;
                    }
                    error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
                }
                else {
                    error_set(ERROR_FEEDBACK_CIRCUITRY, i, HAL_GetTick());
                    differences |= fb_i;
                }
            }
        }
        else
            error_reset(ERROR_FEEDBACK, i);
    }

    return differences;
}

void feedback_set_next_mux_index() {
    fb_index = (fb_index + 1) % FEEDBACK_N;
    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (fb_index & 0b00000001));
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (fb_index & 0b00000010));
    HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (fb_index & 0b00000100));
    HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (fb_index & 0b00001000));
}

void _feedback_handle_tim_elapsed_irq() {
    HAL_ADC_Start_DMA(&ADC_MUX, &dma_data, 1);
}
void _feedback_handle_adc_cnv_cmpl_irq() {
    feedbacks[fb_index] = dma_data;
    feedback_set_next_mux_index();
}