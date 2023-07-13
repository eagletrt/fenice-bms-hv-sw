#include "measures.h"

#include <stdbool.h>

#include "bms_fsm.h"
#include "can_comm.h"
#include "pwm.h"
#include "soc.h"
#include "spi.h"
#include "temperature.h"
#include "internal_voltage.h"
#include "cell_voltage.h"
#include "watchdog.h"
#include "fans_buzzer.h"
#include "bal.h"

#define _MEASURE_CHECK_INTERVAL(interval) (((counter) % (interval)) == 0)

uint32_t counter = 0; // Each timer interrupt it increments by 1
bool flags_checked = false;

void measures_init() {
    counter = 0;
    flags_checked = false;

    // Set timer
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_1, TIM_MS_TO_TICKS(&HTIM_MEASURES, MEASURE_BASE_INTERVAL_MS));
    __HAL_TIM_CLEAR_IT(&HTIM_MEASURES, TIM_IT_CC1);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_1);
}

void measures_check_flags() {
    // Control if flags are already checked
    if (flags_checked)
        return;
    flags_checked = true;
    
    // 10 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_10MS)) {
        can_car_send(PRIMARY_TS_STATUS_FRAME_ID);
    }
    // 50 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_50MS)) {
        // Send info via CAN
        can_car_send(PRIMARY_HV_CURRENT_FRAME_ID);
        can_car_send(PRIMARY_HV_VOLTAGE_FRAME_ID);
        can_car_send(PRIMARY_HV_ERRORS_FRAME_ID);

        // Measure SOC
        if (internal_voltage_measure() == HAL_OK)
            current_read(CONVERT_VALUE_TO_INTERNAL_ADC_VOLTAGE(internal_voltage_get_shunt()));
        soc_sample_energy(HAL_GetTick());

        // Check errors
        // cell_voltage_check_errors();
        current_check_errors();
    }
    // 100 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_100MS)) {
        // Send info via CANS
        can_car_send(PRIMARY_HV_TEMP_FRAME_ID);
        can_car_send(PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID);
        can_car_send(PRIMARY_HV_CELL_BALANCING_STATUS_FRAME_ID);

        // Check errors
        temperature_check_errors();
    }
    // 500 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_500MS)) {
        // Send info via CAN
        can_car_send(PRIMARY_HV_CAN_FORWARD_FRAME_ID);
        can_car_send(PRIMARY_HV_VERSION_FRAME_ID);
        
        // Check cellboards connection errors
        // can_cellboards_check();
        // Check if fans are connected
        error_toggle_check(HAL_GPIO_ReadPin(FANS_DETECT_GPIO_Port, FANS_DETECT_Pin) == GPIO_PIN_RESET, ERROR_FANS_DISCONNECTED, 0);
    }
    // 1 s interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_1S)) {
        // Run fans based on temperature
        if (!fans_is_overrided()) {
            float max_temp = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_max());
            if (max_temp >= FANS_START_TEMP)
                fans_set_speed(fans_curve(max_temp));
        }
    }
    // 5 s interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_5S)) {
        watchdog_routine();
        soc_save_to_eeprom();
    }
}

void _measures_handle_tim_oc_irq(TIM_HandleTypeDef * htim) {
    uint32_t cnt = __HAL_TIM_GetCounter(htim);

    switch (htim->Channel) {
        case HAL_TIM_ACTIVE_CHANNEL_1:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (cnt + TIM_MS_TO_TICKS(htim, MEASURE_BASE_INTERVAL_MS)));
            if (counter == UINT32_MAX)
                counter = 0;
            ++counter;
            flags_checked = false;
            break;
        default:
            break;
    }
}