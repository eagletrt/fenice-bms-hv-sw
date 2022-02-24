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

void _measures_handle_tim_oc_irq(TIM_HandleTypeDef *htim) {
    uint32_t pulse = __HAL_TIM_GetCounter(htim);
    switch (htim->Channel) {
        case HAL_TIM_ACTIVE_CHANNEL_1:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (pulse + TIM_MS_TO_TICKS(htim, VOLTS_READ_INTERVAL)));

            measure_voltage_current_flag = true;
            break;
        case HAL_TIM_ACTIVE_CHANNEL_2:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, (pulse + TIM_MS_TO_TICKS(htim, TEMPS_READ_INTERVAL)));

            measure_temp_flag = true;
            break;
        default:
            break;
    }
}