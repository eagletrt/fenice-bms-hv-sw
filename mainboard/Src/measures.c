#include "measures.h"

#include "bms_fsm.h"
#include "can_comm.h"
#include "pwm.h"
#include "soc.h"
#include "spi.h"
#include "temperature.h"
#include "internal_voltage.h"
#include "cell_voltage.h"

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
}

void measures_check_flags() {
    if (flags & _50MS_INTERVAL_FLAG) {
        measures_voltage_current_soc();
        cell_voltage_check_errors();
        current_check_errors();
        can_car_send(PRIMARY_HV_CURRENT_FRAME_ID);
        can_car_send(PRIMARY_TS_STATUS_FRAME_ID);
        if (error_count() > 0)
            can_car_send(PRIMARY_HV_ERRORS_FRAME_ID);
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

        // Check if the fans are connected
        error_toggle_check(HAL_GPIO_ReadPin(FANS_DETECT_GPIO_Port, FANS_DETECT_Pin) == GPIO_PIN_RESET, ERROR_FANS_DISCONNECTED, 0);

        flags &= ~_500MS_INTERVAL_FLAG;
    }
    if (flags & _5S_INTERVAL_FLAG) {
        soc_save_to_eeprom();
        flags &= ~_5S_INTERVAL_FLAG;
    }
}

void measures_voltage_current_soc() {
    if (internal_voltage_measure() == HAL_OK)
        current_read(internal_voltage_get_shunt());
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