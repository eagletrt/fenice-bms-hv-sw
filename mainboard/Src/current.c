/**
 * @file		current.c
 * @brief		Functions that handle current measurement
 *
 * @date		Sep 24, 2021
 *
 * @author  	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */
#include "current.h"

#include "adc.h"
#include "error.h"
#include "main.h"

#define ROLLING_AVERAGE_FACTOR 0.75f

current_t current[CURRENT_SENSOR_NUM] = {0.f};
current_t zero[CURRENT_SENSOR_NUM]    = {0.f};

current_t _current_convert_low(float volt) {
    return ((1 / 40.0e-3f) * (volt / (330.f / (330 + 169)) - 2.5f) - zero[CURRENT_SENSOR_50]);
}

current_t _current_convert_high(float volt) {
    return ((1 / 2.5e-3f) * (volt / (330.f / (330 + 169)) - 2.5f) - zero[CURRENT_SENSOR_300]);
}

current_t _current_convert_shunt() {
    return 0.f;
}

uint32_t current_measure() {
    // Start ADC measurements for every sensor
    HAL_ADC_Start(&hadc1);
    HAL_ADC_Start(&hadc2);
    //TODO: measure shunt

    HAL_ADC_PollForConversion(&hadc1, 1);
    HAL_ADC_PollForConversion(&hadc2, 1);
    uint32_t time = HAL_GetTick();

    // Convert Hall-low (50A) and save rolling average
    uint16_t adcval            = HAL_ADC_GetValue(&hadc1);
    float volt                 = (adcval * 3.3f) / 4095;
    current[CURRENT_SENSOR_50] = ROLLING_AVERAGE_FACTOR * _current_convert_low(volt) +
                                 (1 - ROLLING_AVERAGE_FACTOR) * current[CURRENT_SENSOR_50];

    // Convert Hall-high (300A) and save rolling average
    adcval                      = HAL_ADC_GetValue(&hadc2);
    volt                        = (adcval * 3.3f) / 4095;
    current[CURRENT_SENSOR_300] = ROLLING_AVERAGE_FACTOR * _current_convert_high(volt) +
                                  (1 - ROLLING_AVERAGE_FACTOR) * current[CURRENT_SENSOR_300];

    // Check for over-current
    error_toggle_check(current_get_current() > PACK_MAX_CURRENT, ERROR_OVER_CURRENT, 0);

    return time;
}

void current_zero() {
    zero[CURRENT_SENSOR_50] += current[CURRENT_SENSOR_50];
    zero[CURRENT_SENSOR_300] += current[CURRENT_SENSOR_300];
}

current_t current_get_current() {
    // TODO: change this
    return current[CURRENT_SENSOR_50];
}
current_t *current_get_current_sensors() {
    return current;
}
current_t *current_get_zero() {
    return zero;
}
