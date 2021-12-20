#include "measures.h"

bool measure_voltage_current_flag=false, measure_temp_flag=false;

void measures_init() {
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_2);
}

void measures_voltage_current() {
    voltage_measure(&SI8900_UART);
    current_read();
    soc_sample_energy(HAL_GetTick());
}