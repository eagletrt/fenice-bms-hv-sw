/**
 * @file error.c
 * @brief Implementation of the error handler required functions
 *
 * @date Mar 21, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "error/error.h"

#include <stdint.h>

#include "mainboard_config.h"
#include "timer_utils.h"
#include "error/error-handler.h"

static uint32_t primask;

void error_cs_enter(void) {
    primask = __get_PRIMASK();
    __disable_irq();
}

void error_cs_exit(void) {
    if (!primask)
        __enable_irq();
}

void error_update_timer_callback(uint32_t timestamp, uint16_t timeout) {
    HAL_TIM_Base_Stop_IT(&HTIM_ERR);
    int32_t t = HAL_GetTick();
    int32_t dt = ((int32_t)timestamp - t) + (int32_t)timeout;
    if (dt < 0)
        dt = 0;
    __HAL_TIM_SET_COUNTER(&HTIM_ERR, 0);
    __HAL_TIM_SET_AUTORELOAD(&HTIM_ERR, TIM_MS_TO_TICKS(&HTIM_ERR, dt));
    __HAL_TIM_CLEAR_FLAG(&HTIM_ERR, TIM_IT_CC1);
    HAL_TIM_Base_Start_IT(&HTIM_ERR);
}

void error_stop_timer_callback() {
    HAL_TIM_Base_Stop_IT(&HTIM_ERR);
}

void error_elapsed() {
    HAL_TIM_Base_Stop_IT(&HTIM_ERR);
    error_expire();
}

