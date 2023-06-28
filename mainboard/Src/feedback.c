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

#include <stdbool.h>

#include "error.h"
#include "mainboard_config.h"


#define feedback_mux_analog_threshold_h 9.5f / 4.3f
#define feedback_mux_analog_threshold_l 0.4f / 4.3f
#define feedback_check_mux_threshold_h  3.5f / 4.3f
#define feedback_check_mux_threshold_l  3.1f / 4.3f

// TODO: Check SD feedbacks thresholds
#define feedback_sd_analog_threshold_h  9.5f / 4.3f
#define feedback_sd_analog_threshold_l  0.4f / 4.3f

#define DMA_DATA_SIZE 5

// Dma data indices
#define DMA_DATA_MUX_FB 3
#define DMA_DATA_SD_OUT 4
#define DMA_DATA_SD_IN  2
#define DMA_DATA_SD_BMS 0
#define DMA_DATA_SD_IMD 1

uint16_t feedbacks[FEEDBACK_N] = { 0 };
uint8_t fb_index = 0;
uint16_t dma_data[DMA_DATA_SIZE] = { 0 };

/**
 * @brief Check if a feedback voltage is greater than the high threshold
 * 
 * @param index The index of the feedback value
 * @return true If the voltage is greater than the threshold
 * @return false Otherwise
 */
bool _is_adc_value_high(uint8_t index) {
    if (index < FEEDBACK_MUX_N)
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) > feedback_mux_analog_threshold_h;
    return FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[index]) > feedback_sd_analog_threshold_h;
}
/**
 * @brief Check if a feedback voltage is lower than the low threshold
 * 
 * @param index The index of the feedback value
 * @return true If the voltage is lower than the threshold
 * @return false Otherwise
 */
bool _is_adc_value_low(uint8_t index) {
    if (index == FEEDBACK_SD_END_POS)
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < 0.5f;
    if (index == FEEDBACK_SD_IN_POS || index == FEEDBACK_SD_OUT_POS)
        return FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[index]) < 0.5f;

    if (index < FEEDBACK_MUX_N)
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < feedback_mux_analog_threshold_l;
    return FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[index]) < feedback_sd_analog_threshold_l;
}
/**
 * @brief Check if the feedback voltage is valid
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
    for (size_t i = 0; i < FEEDBACK_N; ++i) {
        // Set voltage
        out_value[i].voltage = (i < FEEDBACK_MUX_N) ?
            FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[i]) :
            FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[i]);

        // Set status
        if (i == FEEDBACK_CHECK_MUX_POS)
            out_value[i].state = _is_check_mux_ok() ? FEEDBACK_STATE_H : FEEDBACK_STATE_ERROR;
        else {
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
    // Set or reset connection error
    error_toggle_check(HAL_GPIO_ReadPin(CONNS_DETECTION_GPIO_Port, CONNS_DETECTION_Pin) == GPIO_PIN_RESET, ERROR_CONNECTION, 0);

    feedback_t differences = 0;

    for (size_t i = 0; i < FEEDBACK_N; ++i) {
        feedback_t fb_i = 1U << i;
        feedback_t fb_i_val = fb_value & fb_i;

        if (fb_i & fb_check_mask) {
            // Check MUX status
            if (i == FEEDBACK_CHECK_MUX_POS) {
                if (_is_check_mux_ok())
                    error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
                else {
                    error_set(ERROR_FEEDBACK_CIRCUITRY, i, HAL_GetTick());
                    differences |= fb_i;
                }
            }
            else {
                // Check feedback voltages
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
    fb_index = (fb_index + 1) % FEEDBACK_MUX_N;
    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (fb_index & 0b00000001));
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (fb_index & 0b00000010));
    HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (fb_index & 0b00000100));
    HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (fb_index & 0b00001000));
}

void _feedback_handle_tim_elapsed_irq() {
    HAL_ADC_Start_DMA(&ADC_MUX, (uint32_t *)dma_data, DMA_DATA_SIZE);
}
void _feedback_handle_adc_cnv_cmpl_irq() {
    // Save DMA data
    feedbacks[fb_index] = dma_data[DMA_DATA_MUX_FB];
    feedbacks[FEEDBACK_SD_IN_POS]  = dma_data[DMA_DATA_SD_IN];
    feedbacks[FEEDBACK_SD_OUT_POS] = dma_data[DMA_DATA_SD_OUT];
    feedbacks[FEEDBACK_SD_BMS_POS] = dma_data[DMA_DATA_SD_BMS];
    feedbacks[FEEDBACK_SD_IMD_POS] = dma_data[DMA_DATA_SD_IMD];

    feedback_set_next_mux_index();
}