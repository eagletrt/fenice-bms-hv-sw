#include "imd.h"

uint32_t period, duty_cycle;
uint32_t icValRising, icValFalling = 0;
uint8_t isRisingEdge = 0;

void imd_init() {
    HAL_TIM_IC_Start_IT(&HTIM_IMD, TIM_CHANNEL_4);
}

uint8_t imd_get_duty_cycle_percentage() {
    return duty_cycle / period * 100;
}

uint32_t imd_get_freq() {
    return 1/period;
}

uint32_t imd_get_period() {
    return period;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if(htim->Instance == HTIM_IMD.Instance){
        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
            if(isRisingEdge) {
                uint32_t icValue = HAL_TIM_ReadCapturedValue(&HTIM_IMD, TIM_CHANNEL_4);
                isRisingEdge = 1;

                //if it's the first rising edge just save the icValRising and exits
                if(icValFalling != 0) {
                    if ( icValue > icValRising) {
                        period = TIM_TICKS_TO_MS(&HTIM_IMD, icValue - icValRising);
                    } else {
                        period = TIM_TICKS_TO_MS(&HTIM_IMD, (HTIM_IMD.Instance->ARR - icValRising) + icValue);
                    }
                }

                icValRising = icValue;
            } else {
                icValFalling = HAL_TIM_ReadCapturedValue(&HTIM_IMD, TIM_CHANNEL_4);
                isRisingEdge = 0;

                if ( icValFalling > icValRising) {
                    duty_cycle = TIM_TICKS_TO_MS(&HTIM_IMD, icValFalling - icValRising);
                } else {
                    duty_cycle = TIM_TICKS_TO_MS(&HTIM_IMD, (0xffffffff - icValRising) + icValFalling);
                }
            }
        }
    }
}