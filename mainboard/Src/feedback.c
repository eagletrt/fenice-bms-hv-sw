/**
 * @file		feedback.c
 * @brief		Feedback parsing utilities
 *
 * @date		Mar 16, 2021
 * @author  	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author  	Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#include "feedback.h"

#include "adc.h"
#include "main.h"
#include "mainboard_config.h"
#include "pack/pack.h"
#include "tim.h"

#define feedback_analog_threshold_h    9.5f / 4.3f
#define feedback_analog_threshold_l    5.f / 4.3f
#define feedback_check_mux_threshold_h 3.7f / 4.3f
#define feedback_check_mux_threshold_l 2.8f / 4.3f

uint16_t feedback_adc_values[FEEDBACK_N] = {0};
uint8_t fb_index                         = FEEDBACK_MUX_N - 1;
uint16_t dma_data[FEEDBACK_N - FEEDBACK_MUX_N + 1];

void feedback_init() {
    __HAL_TIM_CLEAR_IT(&HTIM_MUX, TIM_IT_UPDATE);
    __HAL_TIM_SET_AUTORELOAD(&HTIM_MUX, TIM_MS_TO_TICKS(&HTIM_MUX, MUX_INTERVAL_MS));
    HAL_TIM_Base_Start_IT(&HTIM_MUX);
}

bool _is_adc_value_high(uint8_t index) {
    float val = FEEDBACK_CONVERT_ADC_TO_VOLTAGE(feedback_adc_values[index]);

    return val > feedback_analog_threshold_h;
}

bool _is_adc_value_low(uint8_t index) {
    float val = FEEDBACK_CONVERT_ADC_TO_VOLTAGE(feedback_adc_values[index]);

    if (index == FEEDBACK_SD_IN_POS || index == FEEDBACK_SD_OUT_POS || index == FEEDBACK_SD_END_POS) {
        return val < 0.5f;
    }

    return val < feedback_analog_threshold_l;
}

bool _is_adc_value_valid(uint8_t index) {
    return _is_adc_value_high(index) || _is_adc_value_low(index);
}

bool _is_check_mux_ok() {
    float val = FEEDBACK_CONVERT_ADC_TO_VOLTAGE(feedback_adc_values[FEEDBACK_CHECK_MUX_POS]);
    return val > feedback_check_mux_threshold_l && val < feedback_check_mux_threshold_h;
}

void feedback_get_feedback_states(feedback_feed_t *out_value) {
    for (uint8_t i = 0; i < FEEDBACK_N; ++i) {
        feedback_t fb_i = 1U << i;

        out_value[i].voltage = FEEDBACK_CONVERT_ADC_TO_VOLTAGE(feedback_adc_values[i]);

        if (fb_i == FEEDBACK_CHECK_MUX) {
            if (_is_check_mux_ok()) {
                out_value[i].state = FEEDBACK_STATE_H;
            } else {
                out_value[i].state = FEEDBACK_STATE_ERROR;
            }
            continue;
        }

        if (_is_adc_value_valid(i)) {     //if the adc value is valid
            if (_is_adc_value_high(i)) {  //adc value is H
                out_value[i].state = FEEDBACK_STATE_H;
            } else {  //adc value is L
                out_value[i].state = FEEDBACK_STATE_L;
            }
        } else {  //adc value is not valid, circuitry error
            out_value[i].state = FEEDBACK_STATE_ERROR;
        }
    }
}

feedback_t feedback_check(feedback_t fb_check_mask, feedback_t fb_value) {
    feedback_t differences = 0;

    for (uint8_t i = 0; i < FEEDBACK_N; ++i) {
        feedback_t fb_i     = 1U << i;
        feedback_t fb_i_val = fb_value & fb_i;

        if (fb_i == FEEDBACK_CHECK_MUX && fb_i & fb_check_mask) {
            if (_is_check_mux_ok()) {
                error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
            } else {
                error_set(ERROR_FEEDBACK_CIRCUITRY, i, HAL_GetTick());
                differences |= fb_i;
            }
            continue;
        }

        if (fb_check_mask & fb_i) {        //if the feedback is in the check mask
            if (_is_adc_value_valid(i)) {  //if the adc value is valid
                if ((fb_i_val && _is_adc_value_high(i)) ||
                    (!fb_i_val && _is_adc_value_low(i))) {  //adc value is as expected (H or L)
                    error_reset(ERROR_FEEDBACK, i);
                } else {  //adc value is not as expected
                    error_set(ERROR_FEEDBACK, i, HAL_GetTick());
                    differences |= fb_i;
                }
                error_reset(ERROR_FEEDBACK_CIRCUITRY, i);
            } else {  //adc value is not valid, circuitry error
                error_set(ERROR_FEEDBACK_CIRCUITRY, i, HAL_GetTick());
                differences |= fb_i;
            }
        } else {  //then reset the error
            error_reset(ERROR_FEEDBACK, i);
        }
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
    HAL_ADC_Start_DMA(&ADC_MUX, (uint32_t *)dma_data, FEEDBACK_N - FEEDBACK_MUX_N + 1);
}

void _feedback_handle_adc_cnv_cmpl_irq() {
    feedback_adc_values[fb_index]               = dma_data[0];
    feedback_adc_values[FEEDBACK_RELAY_SD_POS]  = dma_data[1];  //FB_RELAY_SD
    feedback_adc_values[FEEDBACK_IMD_FAULT_POS] = dma_data[2];  //FB_IMD_FAULT
    feedback_adc_values[FEEDBACK_SD_END_POS]    = dma_data[3];
    feedback_set_next_mux_index();
}