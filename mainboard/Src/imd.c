#include "imd.h"

uint32_t periodTicks = 0, dutyCycleTicks = 0;
uint32_t icValRising = 0, icValFalling = 0;
uint8_t isRisingEdge = 1;

void imd_init() {
    __HAL_TIM_CLEAR_FLAG(&HTIM_BMS, TIM_IT_CC4); //clears existing interrupts on channel 4
    HAL_TIM_IC_Start_IT(&HTIM_IMD, TIM_CHANNEL_4);
}

float imd_get_duty_cycle_percentage() {
    return periodTicks == 0 ? 0 : (100.0 * dutyCycleTicks / periodTicks);
}

float imd_get_freq() {
    return periodTicks == 0 ? 0 : (1000.0 / TIM_TICKS_TO_MS(&HTIM_IMD, periodTicks));
}

uint32_t imd_get_period() {
    return TIM_TICKS_TO_MS(&HTIM_IMD, periodTicks);
}

IMD_STATE imd_get_state() {
    uint32_t freq = imd_get_freq();

    if(freq == 0)   return IMD_SC;
    if(freq == 10)  return IMD_NORMAL;
    if(freq == 20)  return IMD_UNDER_VOLTAGE;
    if(freq == 30)  return IMD_START_MEASURE;
    if(freq == 40)  return IMD_DEVICE_ERROR;
    if(freq == 50)  return IMD_HEARTH_FAULT;

    return -1;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if(htim->Instance == HTIM_IMD.Instance){
        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
            if(isRisingEdge) {
                uint32_t icValue = HAL_TIM_ReadCapturedValue(&HTIM_IMD, TIM_CHANNEL_4);
                isRisingEdge = 0;

                //if it's the first rising edge just save the icValRising and exits
                if(icValFalling != 0) {
                    if ( icValue > icValRising) {
                        periodTicks = icValue - icValRising;
                    } else {
                        periodTicks = (__HAL_TIM_GET_AUTORELOAD(&HTIM_IMD) - icValRising) + icValue;
                    }
                }

                icValRising = icValue;
            } else {
                icValFalling = HAL_TIM_ReadCapturedValue(&HTIM_IMD, TIM_CHANNEL_4);
                isRisingEdge = 1;

                if ( icValFalling > icValRising) {
                    dutyCycleTicks = icValFalling - icValRising;
                } else {
                    dutyCycleTicks = (__HAL_TIM_GET_AUTORELOAD(&HTIM_IMD) - icValRising) + icValFalling;
                }
            }
        }
    }
}