/**
 * @file		feedback.c
 * @brief		Feedback parsing utilities
 *
 * @date		Mar 16, 2021
 * @author  	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "feedback.h"

#include "main.h"
#include "mainboard_config.h"
#include "adc.h"
#include "cli_bms.h"
#include "tim.h"

const float feedback_analog_threshold_h = 10.5f / 4.3f;
const float feedback_analog_threshold_l = 1.5f / 4.3f;
feedback_t feedback;
uint16_t adc_values[FEEDBACK_MUX_N] = {0};
uint8_t fb_index = FEEDBACK_MUX_N;

void feedback_init() {
    feedback = 0;
    __HAL_TIM_SET_AUTORELOAD(&HTIM_MUX, TIM_MS_TO_TICKS(&HTIM_MUX, MUX_INTERVAL_MS));
    __HAL_TIM_SET_COMPARE(&HTIM_MUX, TIM_CHANNEL_1, MUX_DELAY_MS);
    __HAL_TIM_CLEAR_IT(&HTIM_MUX, TIM_IT_UPDATE);
    __HAL_TIM_CLEAR_FLAG(&HTIM_MUX, TIM_FLAG_CC1);
    HAL_TIM_Base_Start_IT(&HTIM_MUX);
    HAL_TIM_OC_Start_IT(&HTIM_MUX, TIM_CHANNEL_1);
}

void feedback_read(feedback_t fb_mask) {
    //initialize the feedback value to 0 on the mask bits;
    feedback &= (~fb_mask);
    for (uint8_t i = 0; i < FEEDBACK_N; ++i) {
        if ((1U << i) & fb_mask) {
            HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (i & 0b00000001));
            HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (i & 0b00000010));
            HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (i & 0b00000100));
            HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (i & 0b00001000));

            feedback |= (HAL_GPIO_ReadPin(MUX_IN_GPIO_Port, MUX_IN_Pin) << i);
        }
    }
}

bool _is_adc_value_high(uint8_t index) {
    if(index == FEEDBACK_CHECK_MUX_POS) return (CONVERT_ADC_TO_VOLTAGE(adc_values[index]) * 4.3F) > feedback_analog_threshold_h;
    return CONVERT_ADC_TO_VOLTAGE(adc_values[index]) > feedback_analog_threshold_h;
}

bool _is_adc_value_low(uint8_t index) {
    if(index == FEEDBACK_CHECK_MUX_POS) return (CONVERT_ADC_TO_VOLTAGE(adc_values[index]) * 4.3F) < feedback_analog_threshold_l;
    return CONVERT_ADC_TO_VOLTAGE(adc_values[index]) < feedback_analog_threshold_l;
}

bool feedback_check(feedback_t fb_check_mask, feedback_t fb_value, error_id error_id) {
    feedback_t difference = 0;

    return true;
    
    for (uint8_t i=0; i<FEEDBACK_N; ++i) {
        feedback_t fb = fb_value & (1U << i);
        if((fb_check_mask & (1U << i))) {
            if(i<FEEDBACK_MUX_N) {
                if(((fb && _is_adc_value_high(i)) || (!fb && _is_adc_value_low(i)))) {
                    error_reset(error_id, i);
                } else {
                    error_set(error_id, i, HAL_GetTick());
                    difference |= 1U << i;
                }
            } else {
                error_toggle_check(((feedback & fb_check_mask) ^ fb_value) & (1U << i), error_id, i);
            }
        } else {
            error_reset(error_id, i);
        }
    }

    return !difference;
}

// //this check is performed during ON state and from PRECHARGE TO ON
// bool pack_feedback_check_on() {
// 	uint16_t difference = pd_feedback ^ FEEDBACK_ON;

// 	for (uint8_t i = 0; i < FEEDBACK_N; i++) {
// 		error_toggle_check(difference & (1 << i), ERROR_FEEDBACK_HARD, i);
// 	}

// 	return pd_feedback == FEEDBACK_ON;
// }

// bool pack_feedback_check_charge() {
// 	uint16_t difference = pd_feedback ^ FEEDBACK_CHARGE;

// 	for (uint8_t i = 0; i < FEEDBACK_N; i++) {
// 		error_toggle_check(difference & (1 << i), ERROR_FEEDBACK_HARD, i);
// 	}

// 	return pd_feedback == FEEDBACK_CHARGE;
// }

void feedback_set_next_mux_index() {
    fb_index = (fb_index+1) % FEEDBACK_MUX_N;
    HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (fb_index & 0b00000001));
    HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (fb_index & 0b00000010));
    HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (fb_index & 0b00000100));
    HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (fb_index & 0b00001000));
    //for(uint8_t i=0; i<100; ++i);
}

void feedback_save_value(uint32_t adc_value, uint8_t index) {
    adc_values[index] = adc_value;
}

uint8_t feedback_get_adc_index() {
    return fb_index;
}

void feedback_incr_adc_index() {
    ++fb_index;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    uint8_t feedback_index = FEEDBACK_MUX_N;
    GPIO_TypeDef *GPIO_Port;
    switch(GPIO_Pin) {
        case FB_IMD_FAULT_Pin:
            GPIO_Port = FB_IMD_FAULT_GPIO_Port;
            feedback_index += FEEDBACK_IMD_FAULT_POS;
            break;
        case FB_RELAY_SD_Pin:
            GPIO_Port = FB_RELAY_SD_GPIO_Port;
            feedback_index += FEEDBACK_RELAY_SD_POS;
            break;
        case FB_FROM_TSMS_Pin:
            GPIO_Port = FB_FROM_TSMS_GPIO_Port;
            feedback_index += FEEDBACK_FROM_TSMS_POS;
            break;
        default: return;
    }

    if (HAL_GPIO_ReadPin(GPIO_Port, GPIO_Pin)) {
        feedback |= (1U << feedback_index);
    } else {
        feedback &= ~(1U << feedback_index);
    }
}