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


#define FEEDBACK_MUX_ANALOG_THRESHOLD_H 2.2f
#define FEEDBACK_MUX_ANALOG_THRESHOLD_L 0.7f

#define FEEDBACK_SD_THRESHOLD_H 10.0f // V
#define FEEDBACK_SD_THRESHOLD_L 1.0f  // V
#define FEEDBACK_SD_DIVIDER_RATIO 0.233f
#define FEEDBACK_SD_ANALOG_THRESHOLD_L (FEEDBACK_SD_DIVIDER_RATIO * FEEDBACK_SD_THRESHOLD_L)
#define FEEDBACK_SD_ANALOG_THRESHOLD_H (FEEDBACK_SD_DIVIDER_RATIO * FEEDBACK_SD_THRESHOLD_H)

#define FEEDBACK_CHECK_MUX_THRESHOLD_L 2.4f // V
#define FEEDBACK_CHECK_MUX_THRESHOLD_H 3.4f // V

#define FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L 0.7f // V
#define FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_H 1.5f // V
#define FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_L 1.3f // V
#define FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_H 1.7f // V

#define MUX_INTERVAL_MS 0.2
#define DMA_DATA_SIZE 5

// Dma data indices
#define DMA_DATA_MUX_FB 3
#define DMA_DATA_SD_OUT 4
#define DMA_DATA_SD_IN  2
#define DMA_DATA_SD_BMS 1
#define DMA_DATA_SD_IMD 0

#define FEEDBACK_MUX_VREF 3.3f
#define FEEDBACK_SD_VREF 13.f
#define FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(VALUE) ((VALUE) * (FEEDBACK_MUX_VREF / 4095))
#define FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(VALUE) ((VALUE) * (FEEDBACK_SD_VREF / 4095))


uint16_t feedbacks[FEEDBACK_N] = { 0 };
uint16_t dma_data[DMA_DATA_SIZE] = { 0 };
uint8_t fb_index = 0;
size_t mux_update_count = 0;


/**
 * @brief Check if a feedback voltage is greater than the high threshold
 * 
 * @param index The index of the feedback value
 * @return true If the voltage is greater than the threshold
 * @return false Otherwise
 */
bool _is_adc_value_high(uint8_t index) {
    if (index == FEEDBACK_CHECK_MUX_POS) {
        if (is_handcart_connected)
            return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) > FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_H;
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) > FEEDBACK_CHECK_MUX_THRESHOLD_H;
    }
    if (index == FEEDBACK_IMD_FAULT_POS && is_handcart_connected)
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) > FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_H;
    if (index < FEEDBACK_MUX_N)
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) > FEEDBACK_MUX_ANALOG_THRESHOLD_H;
    return FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[index]) > FEEDBACK_SD_ANALOG_THRESHOLD_H;
}
/**
 * @brief Check if a feedback voltage is lower than the low threshold
 * 
 * @param index The index of the feedback value
 * @return true If the voltage is lower than the threshold
 * @return false Otherwise
 */
bool _is_adc_value_low(uint8_t index) {
    if (index == FEEDBACK_CHECK_MUX_POS) {
        if (is_handcart_connected)
            return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_CHECK_MUX_HANDCART_THRESHOLD_L;
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_CHECK_MUX_THRESHOLD_L;
    }
    if (index == FEEDBACK_IMD_FAULT_POS && is_handcart_connected)
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_IMD_FAULT_HANDCART_THRESHOLD_L;
    if (index == FEEDBACK_SD_END_POS)
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < 0.5f;
    if (index == FEEDBACK_SD_IN_POS || index == FEEDBACK_SD_OUT_POS)
        return FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[index]) < 0.5f;

    if (index < FEEDBACK_MUX_N)
        return FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_MUX_ANALOG_THRESHOLD_L;
    return FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[index]) < FEEDBACK_SD_ANALOG_THRESHOLD_L;
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
/** @brief Set the multiplexer index */
void _feedback_set_mux_index() {
    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (fb_index & 0b00000001));
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (fb_index & 0b00000010));
    HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (fb_index & 0b00000100));
    HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (fb_index & 0b00001000));
}


void feedback_init() {
    // Start feedbacks timer
    __HAL_TIM_CLEAR_IT(&HTIM_MUX, TIM_IT_UPDATE);
    __HAL_TIM_SET_AUTORELOAD(&HTIM_MUX, TIM_MS_TO_TICKS(&HTIM_MUX, MUX_INTERVAL_MS));
    HAL_TIM_Base_Start_IT(&HTIM_MUX);
}
bool feedback_is_mux_ok() {
    return !_is_adc_value_valid(FEEDBACK_CHECK_MUX_POS);
}
bool feedback_is_updated() {
    return mux_update_count >= FEEDBACK_MUX_N;
}
feedback_feed_t feedback_get_feedback_state(size_t index) {
    feedback_feed_t feedback;
    feedback.voltage = (index < FEEDBACK_MUX_N) ?
        FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(feedbacks[index]) :
        FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(feedbacks[index]);
    
    // Set status
    if (index == FEEDBACK_CHECK_MUX_POS)
        feedback.state = feedback_is_mux_ok() ? FEEDBACK_STATE_H : FEEDBACK_STATE_ERROR;
    else {
        if (_is_adc_value_high(index))
            feedback.state = FEEDBACK_STATE_H;
        else if (_is_adc_value_low(index))
            feedback.state = FEEDBACK_STATE_L;
        else
            feedback.state = FEEDBACK_STATE_ERROR;
    }
    
    return feedback;
}
void feedback_get_feedback_states(feedback_feed_t out_value[FEEDBACK_N]) {
    for (size_t i = 0; i < FEEDBACK_N; i++)
        out_value[i] = feedback_get_feedback_state(i);
}
feedback_t feedback_check(feedback_t value) {
    // Set or reset connection error
    error_toggle_check(HAL_GPIO_ReadPin(CONNS_DETECTION_GPIO_Port, CONNS_DETECTION_Pin) == GPIO_PIN_RESET, ERROR_CONNECTOR_DISCONNECTED, 0);

    feedback_t diff = 0;
    for (size_t i = 0; i < FEEDBACK_N; i++) {
        feedback_t feedback = 1 << i;
        
        if (i == FEEDBACK_CHECK_MUX_POS) {
            if (feedback_is_mux_ok())
                error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
            else {
                error_set(ERROR_FEEDBACK_CIRCUITRY, i, HAL_GetTick());
                diff |= feedback;
            }
        }
        else {
            feedback_t fb_val = value & feedback;
            // Check feedback voltages
            if (_is_adc_value_valid(i)) {
                if ((fb_val && _is_adc_value_high(i)) || (!fb_val && _is_adc_value_low(i)))
                    error_reset(ERROR_FEEDBACK, i);
                else {
                    error_set(ERROR_FEEDBACK, i, HAL_GetTick());
                    diff |= feedback;
                }
                error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
            }
            else {
                error_set(ERROR_FEEDBACK_CIRCUITRY, i, HAL_GetTick());
                diff |= feedback;
            }
        }
    }

    return diff;
}
void feedback_request_update() {
    fb_index = 0;
    mux_update_count = 0;
}

void _feedback_handle_tim_elapsed_irq() {
    if (fb_index == 0)
        mux_update_count = 0;
    _feedback_set_mux_index();
    HAL_ADC_Start_DMA(&ADC_MUX, (uint32_t *)dma_data, DMA_DATA_SIZE);
}
void _feedback_handle_adc_cnv_cmpl_irq() {
    // Save DMA data
    feedbacks[fb_index] = dma_data[DMA_DATA_MUX_FB];
    feedbacks[FEEDBACK_SD_IN_POS]  = dma_data[DMA_DATA_SD_IN];
    feedbacks[FEEDBACK_SD_OUT_POS] = dma_data[DMA_DATA_SD_OUT];
    feedbacks[FEEDBACK_SD_BMS_POS] = dma_data[DMA_DATA_SD_BMS];
    feedbacks[FEEDBACK_SD_IMD_POS] = dma_data[DMA_DATA_SD_IMD];

    ++mux_update_count;
    fb_index = (fb_index + 1) % FEEDBACK_MUX_N;
}