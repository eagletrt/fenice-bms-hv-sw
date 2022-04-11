#include "measures.h"
#include "voltage.h"
#include "adc124s021.h"
#include "spi.h"
#include "can_comm.h"
#include "bms_fsm.h"

measures_flags_t flags;

void measures_init() {
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_1, TIM_MS_TO_TICKS(&HTIM_MEASURES, _20MS_INTERVAL));
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_2, TIM_MS_TO_TICKS(&HTIM_MEASURES, _200MS_INTERVAL));
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_3, TIM_MS_TO_TICKS(&HTIM_MEASURES, _500MS_INTERVAL));
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_2);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_3);
}

void measures_check_flags() {
    if(flags & _20MS_INTERVAL_FLAG) {
        measures_voltage_current_soc();
        can_car_send(ID_HV_VOLTAGE);
        can_car_send(ID_HV_CURRENT);
        can_car_send(ID_TS_STATUS);
        if(error_count > 0) {
            can_car_send(ID_HV_ERRORS);
        }
        flags &= ~_20MS_INTERVAL_FLAG;
    }
    if(flags & _200MS_INTERVAL_FLAG) {
        can_car_send(ID_HV_TEMP);
        flags &= ~_200MS_INTERVAL_FLAG;
    }
    if(flags & _500MS_INTERVAL_FLAG && bms.handcart_connected) {
        can_car_send(ID_HV_CELLS_TEMP);
        can_car_send(ID_HV_CELLS_VOLTAGE);
        can_car_send(ID_HV_CELL_BALANCING_STATUS);
        flags &= ~_500MS_INTERVAL_FLAG;
    }
}

void measures_voltage_current_soc() {
    ADC124S021_CH chs[3] = {ADC124_BUS_CHANNEL, ADC124_INTERNAL_CHANNEL, ADC124_SHUNT_CHANNEL};
    voltage_t adc124_measures[3] = {0};
    if(adc124s021_read_channels(&SPI_ADC124S, chs, 3, adc124_measures)) {
        voltage_measure(adc124_measures);
        current_read((uint16_t)adc124_measures[2]);
    }
    soc_sample_energy(HAL_GetTick());
}

void _measures_handle_tim_oc_irq(TIM_HandleTypeDef *htim) {
    uint32_t cnt = __HAL_TIM_GetCounter(htim);
    switch (htim->Channel) {
        case HAL_TIM_ACTIVE_CHANNEL_1:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (cnt + TIM_MS_TO_TICKS(htim, _20MS_INTERVAL)));
            flags |= _20MS_INTERVAL_FLAG;
            break;
        case HAL_TIM_ACTIVE_CHANNEL_2:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, (cnt + TIM_MS_TO_TICKS(htim, _200MS_INTERVAL)));
            flags |= _200MS_INTERVAL_FLAG;
            break;
        case HAL_TIM_ACTIVE_CHANNEL_3:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, (cnt + TIM_MS_TO_TICKS(htim, _500MS_INTERVAL)));
            flags |= _500MS_INTERVAL_FLAG;
            break;
        default:
            break;
    }
}