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
#include "cli_bms.h"
#include "error.h"
#include "main.h"
#include "string.h"

#define ROLLING_AVERAGE_FACTOR 0.75f
#define MEASURE_SAMPLE_SIZE    128

uint16_t adc_50[MEASURE_SAMPLE_SIZE]  = {0};
uint16_t adc_300[MEASURE_SAMPLE_SIZE] = {0};

current_t current[CURRENT_SENSOR_NUM] = {0.f};
current_t zero[CURRENT_SENSOR_NUM]    = {0.f};

current_t _current_convert_low(float volt) {
    return (1 / 40.0e-3f) * (volt / (330.f / (330 + 169)) - 2.5f);
}

current_t _current_convert_high(float volt) {
    return (1 / 2.5e-3f) * (volt / (330.f / (330 + 169)) - 2.5f);
}

current_t _current_convert_shunt() {
    return 0.f;
}

void current_start_measure() {
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *)adc_50, MEASURE_SAMPLE_SIZE);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc_300, MEASURE_SAMPLE_SIZE);
}

uint32_t current_read() {
    //TODO: measure shunt

    uint32_t time    = HAL_GetTick();
    uint32_t avg_50  = 0;
    uint32_t avg_300 = 0;
    for (size_t i = 0; i < MEASURE_SAMPLE_SIZE; i++) {
        avg_50 += adc_50[i];
        avg_300 += adc_300[i];
    }
    avg_50 /= MEASURE_SAMPLE_SIZE;
    avg_300 /= MEASURE_SAMPLE_SIZE;

    // Convert Hall-low (50A) and save rolling average
    float volt                 = (avg_50 * 3.3f) / 4095;
    current[CURRENT_SENSOR_50] = (ROLLING_AVERAGE_FACTOR * _current_convert_low(volt) +
                                  (1 - ROLLING_AVERAGE_FACTOR) * current[CURRENT_SENSOR_50]) -
                                 zero[CURRENT_SENSOR_50];

    // Convert Hall-high (300A) and save rolling average
    volt                        = (avg_300 * 3.3f) / 4095;
    current[CURRENT_SENSOR_300] = (ROLLING_AVERAGE_FACTOR * _current_convert_high(volt) +
                                   (1 - ROLLING_AVERAGE_FACTOR) * current[CURRENT_SENSOR_300]) -
                                  zero[CURRENT_SENSOR_300];

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
