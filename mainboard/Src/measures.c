#include "measures.h"
#include "voltage.h"
#include "adc124s021.h"
#include "spi.h"
#include "can_comm.h"

void measures_init() {
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_2);
}

void measures_voltage_current() {
    ADC124S021_CH chs[3] = {ADC124_BUS_CHANNEL, ADC124_INTERNAL_CHANNEL, ADC124_SHUNT_CHANNEL};
    voltage_t adc124_measures[3] = {0};
    if(adc124s021_read_channels(&SPI_ADC124S, chs, 3, adc124_measures)) {
        voltage_measure(adc124_measures);
        current_read((uint16_t)adc124_measures[2]);
    }
    soc_sample_energy(HAL_GetTick());
}

void _measures_handle_tim_oc_irq(TIM_HandleTypeDef *htim) {
    uint32_t pulse = __HAL_TIM_GetCounter(htim);
    switch (htim->Channel) {
        case HAL_TIM_ACTIVE_CHANNEL_1:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (pulse + TIM_MS_TO_TICKS(htim, VOLTS_READ_INTERVAL)));

            measures_voltage_current();
            can_car_send(ID_HV_CURRENT);
            break;
        case HAL_TIM_ACTIVE_CHANNEL_2:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, (pulse + TIM_MS_TO_TICKS(htim, TEMPS_READ_INTERVAL)));

            can_car_send(ID_HV_TEMP);
            break;
        default:
            break;
    }
}