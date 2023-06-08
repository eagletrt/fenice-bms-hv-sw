#include "measures.h"

#include "max22530.h"
#include "bms_fsm.h"
#include "can_comm.h"
#include "pwm.h"
#include "soc.h"
#include "spi.h"
#include "temperature.h"
#include "voltage.h"

MAX22530_HandleTypeDef adc_handler;
measures_flags_t flags;

void measures_init() {
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_1, TIM_MS_TO_TICKS(&HTIM_MEASURES, _50MS_INTERVAL));
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_2, TIM_MS_TO_TICKS(&HTIM_MEASURES, _200MS_INTERVAL));
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_3, TIM_MS_TO_TICKS(&HTIM_MEASURES, _500MS_INTERVAL));
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_4, TIM_MS_TO_TICKS(&HTIM_MEASURES, _5S_INTERVAL));
    __HAL_TIM_CLEAR_IT(&HTIM_MEASURES, TIM_IT_CC1);
    __HAL_TIM_CLEAR_IT(&HTIM_MEASURES, TIM_IT_CC2);
    __HAL_TIM_CLEAR_IT(&HTIM_MEASURES, TIM_IT_CC3);
    __HAL_TIM_CLEAR_IT(&HTIM_MEASURES, TIM_IT_CC4);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_2);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_3);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_4);
    flags = 0;

    // Init ADC
    max22530_init(&adc_handler, &SPI_ADC, ADC_CS_GPIO_Port, ADC_CS_Pin);
}

void measures_check_flags() {
    if (flags & _50MS_INTERVAL_FLAG) {
        measures_voltage_current_soc();
        voltage_check_errors();
        current_check_errors();
        can_car_send(PRIMARY_HV_VOLTAGE_FRAME_ID);
        can_car_send(PRIMARY_HV_CURRENT_FRAME_ID);
        can_car_send(PRIMARY_TS_STATUS_FRAME_ID);
        if (error_count() > 0) {
            can_car_send(PRIMARY_HV_ERRORS_FRAME_ID);
        }
        flags &= ~_50MS_INTERVAL_FLAG;
    }
    if (flags & _200MS_INTERVAL_FLAG) {
        can_cellboards_check();
        temperature_check_errors();
        can_car_send(PRIMARY_HV_TEMP_FRAME_ID);
        flags &= ~_200MS_INTERVAL_FLAG;
    }
    if (flags & _500MS_INTERVAL_FLAG) {
        if (bms.handcart_connected) {
            can_car_send(PRIMARY_HV_CELLS_TEMP_FRAME_ID);
            can_car_send(PRIMARY_HV_CELLS_VOLTAGE_FRAME_ID);
            can_car_send(PRIMARY_HV_CELL_BALANCING_STATUS_FRAME_ID);
        }
        can_car_send(PRIMARY_HV_CAN_FORWARD_FRAME_ID);
        can_car_send(PRIMARY_HV_VERSION_FRAME_ID);
        flags &= ~_500MS_INTERVAL_FLAG;
    }
    if (flags & _5S_INTERVAL_FLAG) {
        soc_save_to_eeprom();
        flags &= ~_5S_INTERVAL_FLAG;
    }
}

void measures_voltage_current_soc() {
    float volts[MAX22530_CHANNEL_COUNT] = { 0 };
    if (max22530_read_all_channels(&adc_handler, volts) == HAL_OK) {
        voltage_measure(volts);
        current_read(volts[2]);
    }
    soc_sample_energy(HAL_GetTick());
}

void _measures_handle_tim_oc_irq(TIM_HandleTypeDef *htim) {
    uint32_t cnt = __HAL_TIM_GetCounter(htim);
    switch (htim->Channel) {
        case HAL_TIM_ACTIVE_CHANNEL_1:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (cnt + TIM_MS_TO_TICKS(htim, _50MS_INTERVAL)));
            flags |= _50MS_INTERVAL_FLAG;
            break;
        case HAL_TIM_ACTIVE_CHANNEL_2:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_2, (cnt + TIM_MS_TO_TICKS(htim, _200MS_INTERVAL)));
            flags |= _200MS_INTERVAL_FLAG;
            break;
        case HAL_TIM_ACTIVE_CHANNEL_3:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, (cnt + TIM_MS_TO_TICKS(htim, _500MS_INTERVAL)));
            flags |= _500MS_INTERVAL_FLAG;
            break;
        case HAL_TIM_ACTIVE_CHANNEL_4:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_4, (cnt + TIM_MS_TO_TICKS(htim, _5S_INTERVAL)));
            flags |= _5S_INTERVAL_FLAG;
            break;
        default:
            break;
    }
}