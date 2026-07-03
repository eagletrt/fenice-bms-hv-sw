/**
 * @file mainboard_config.c
 * @brief Functions to ensure that all the measurements and error checks
 * happens periodically for a given interval
 *
 * @date Nov 13, 2021
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */
#include "measures.h"

#include "usart.h"
#include "can-primary.h"
#include "energy/soc.h"
#include "error_simple.h"
#include "fans_buzzer.h"
#include "mainboard_config.h"
#include "pack/cell_voltage.h"
#include "pack/current.h"
#include "pack/internal_voltage.h"
#include "pack/temperature.h"
#include "peripherals/can_comm.h"
#include "tim.h"
#include "timer_utils.h"
#include "watchdog.h"

#include <stdbool.h>
#include <stdint.h>

#define MEASURE_CHECK_DELAY               1000                             // ms
#define _MEASURE_CHECK_INTERVAL(interval) (((counter) % (interval)) == 0)  // Check if a given interval is passed

volatile uint32_t counter   = 0;  // Each timer interrupt it increments by 1
volatile uint32_t timestamp = 0;
volatile bool flags_checked = false;

void measures_init() {
    counter       = 0;
    flags_checked = false;

    // Set timer
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_1, TIM_MS_TO_TICKS(&HTIM_MEASURES, MEASURE_BASE_INTERVAL_MS));
    __HAL_TIM_CLEAR_IT(&HTIM_MEASURES, TIM_IT_CC1);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_1);

    timestamp = HAL_GetTick();
}

void measures_check_flags() {
    // Control if flags are already checked
    if (flags_checked)
        return;
    flags_checked = true;

    // 10 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_10MS)) {
    }
    // 50 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_50MS)) {
        // Send info via CAN
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_STATUS);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CURRENT);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_POWER);
        // can_car_send(PRIMARY_HV_SOC_FRAME_ID);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_TS_VOLTAGE);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_VOLTAGES);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_VOLTAGES_INFO);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_ERRORS);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_FEEDBACK_STATUS);

        // Measure SOC
        if (internal_voltage_measure() == HAL_OK)
            current_read(CONVERT_VALUE_TO_INTERNAL_ADC_VOLTAGE(internal_voltage_get_shunt()));
        // soc_sample_energy(HAL_GetTick());

        // Check errors
        if (HAL_GetTick() - timestamp >= MEASURE_CHECK_DELAY)
            cell_voltage_check_errors();
        current_check_errors();
    }
    // 100 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_100MS)) {
        // Send info via CANS
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_TEMPERATURES);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_TEMPERATURES_INFO);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_IMD);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_BALANCING_STATUS);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_FEEDBACK_TS_VOLTAGE);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_FEEDBACK_SD_VOLTAGE);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_FEEDBACK_MISC_VOLTAGE);
        // can_car_send(PRIMARY_HV_ENERGY_FRAME_ID);

        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_VERSION);
        can_car_send(CAN_PRIMARY_MESSAGE_FRAME_ID_HV_BMS_CELLBOARD_VERSION);

        // Check errors
        temperature_check_errors();
        // Check if fans are connected
        if (HAL_GPIO_ReadPin(FANS_DETECT_GPIO_Port, FANS_DETECT_Pin) == GPIO_PIN_RESET) {
            error_simple_set(ERROR_GROUP_ERROR_FANS_DISCONNECTED, 0);
        } else {
            error_simple_reset(ERROR_GROUP_ERROR_FANS_DISCONNECTED, 0);
        }
    }
    // 200 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_200MS)) {
        // Send info via CANS
        // can_car_send(PRIMARY_HV_DEBUG_SIGNALS_FRAME_ID);
        // can_car_send(PRIMARY_DEBUG_SIGNAL_2_FRAME_ID);
    }
    // 500 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_500MS)) {
        // Send info via CAN
        // can_car_send(PRIMARY_HV_CAN_FORWARD_FRAME_ID);
        // can_car_send(PRIMARY_HV_FANS_STATUS_FRAME_ID);

        // Check cellboards connection errors
        if (HAL_GetTick() - timestamp >= MEASURE_CHECK_DELAY)
            can_cellboards_check();
    }
    // 1 s interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_1S)) {

        // Run fans based on temperature
        if (!fans_is_overrided()) {
            float max_temp = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_max());
            fans_set_speed(fans_curve(max_temp));
        }
    }
    // 5 s interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_5S)) {
        // watchdog_routine();
        soc_save_to_eeprom();
    }
}

void _measures_handle_tim_oc_irq(TIM_HandleTypeDef *htim) {
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
