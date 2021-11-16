#include "measures.h"

void measures_init() {
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_2);
}

void measures_current() {
    //voltage_measure(&SI8900_UART);
    current_read();
    soc_sample_energy(HAL_GetTick());
}