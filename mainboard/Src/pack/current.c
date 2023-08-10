/**
 * @file    current.c
 * @brief   Functions that handle current measurement
 *
 * @date    Sep 24, 2021
 *
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author  Federico Carbone [federico.carbone@studenti.unitn.it]
 */
#include "pack/current.h"

#include "adc.h"
#include "error.h"
#include "main.h"
#include "mainboard_config.h"

#include <math.h>

#define MEASURE_SAMPLE_SIZE 128

#define CURRENT_SENSITIVITY_LOW 40e-3f // V / A
#define CURRENT_SENSITIVITY_HIGH 6.67e-3f // V / A

#define CURRENT_DIVIDER_RATIO_INVERSE (330.f + 169.f) / 330.f

uint16_t adc_50[MEASURE_SAMPLE_SIZE]  = { 0 };
uint16_t adc_300[MEASURE_SAMPLE_SIZE] = { 0 };

current_t current[CURRENT_SENSOR_NUM] = {0.f};

float V0L = 0, V0H = 0;  //voltage offset (Vout(0A))

current_t _current_convert_low(float volt) {
    return (CURRENT_DIVIDER_RATIO_INVERSE / CURRENT_SENSITIVITY_LOW) * (volt - V0L);
}

current_t _current_convert_high(float volt) {
    return (CURRENT_DIVIDER_RATIO_INVERSE / CURRENT_SENSITIVITY_HIGH) * (volt - V0H);
}

current_t _current_convert_shunt(float volt) {
    return (volt - 0.468205124) / (1e-4f * 75);
}

void current_start_measure() {
    HAL_ADC_Start_DMA(&ADC_HALL50, (uint32_t *)adc_50, MEASURE_SAMPLE_SIZE);
    HAL_ADC_Start_DMA(&ADC_HALL300, (uint32_t *)adc_300, MEASURE_SAMPLE_SIZE);
}

float volt_50 = 0;
float volt_300 = 0;
uint32_t current_read(float shunt_adc_val) {
    uint32_t time    = HAL_GetTick();
    uint64_t avg_50  = 0;
    uint64_t avg_300 = 0;
    for (size_t i = 0; i < MEASURE_SAMPLE_SIZE; i++) {
        avg_50 += adc_50[i];
        avg_300 += adc_300[i];
    }

    // Convert Hall-low (50A)
    float volt = avg_50 * (3.3f / 4095 / MEASURE_SAMPLE_SIZE);
    volt_50 = volt;
    current[CURRENT_SENSOR_50] = _current_convert_low(volt);

    // Convert Hall-high (300A)
    volt = avg_300 * (3.3f / 4095 / MEASURE_SAMPLE_SIZE);
    volt_300 = volt;
    current[CURRENT_SENSOR_300] = _current_convert_high(volt);

    // Convert Shunt
    current[CURRENT_SENSOR_SHUNT] = _current_convert_shunt(shunt_adc_val);

    // Check for over-current
    error_toggle_check(fabs(current_get_current()) > PACK_MAX_CURRENT, ERROR_OVER_CURRENT, 0);
    if (volt_300 < CURRENT_SENSOR_DISCONNECTED_THRESHOLD)
        error_set(ERROR_FANS_DISCONNECTED, 1, HAL_GetTick());

    return time;
}
float current_get_volt_300() {
    return volt_300;
}
float current_get_volt_50() {
    return volt_50;
}
void current_zero() {
    uint32_t avg_50  = 0;
    uint32_t avg_300 = 0;
    for (size_t i = 0; i < MEASURE_SAMPLE_SIZE; ++i) {
        avg_50 += adc_50[i];
        avg_300 += adc_300[i];
    }
    V0L = avg_50 * (3.3f / 4095 / MEASURE_SAMPLE_SIZE);
    V0H = avg_300 * (3.3f / 4095 / MEASURE_SAMPLE_SIZE);
}

current_t current_get_current() {
    if (current[CURRENT_SENSOR_SHUNT] < 4 && current[CURRENT_SENSOR_50] < 4)
        return current[CURRENT_SENSOR_SHUNT];
    if (current[CURRENT_SENSOR_50] < 34 && current[CURRENT_SENSOR_300] < 35)
        return current[CURRENT_SENSOR_50];
    return current[CURRENT_SENSOR_300];
}
current_t *current_get_current_sensors() {
    return current;
}

current_t current_get_current_from_sensor(uint8_t sensor) {
    if (sensor >= CURRENT_SENSOR_NUM) {
        return -1;
    }
    return current[sensor];
}

void current_check_errors() {
    current_t hall_50 = fabs(current[CURRENT_SENSOR_50]);
    current_t hall_300 = fabs(current[CURRENT_SENSOR_300]);

    error_toggle_check(hall_300 > PACK_MAX_CURRENT, ERROR_OVER_CURRENT, 0);
    
    // Hall effect sensor disconnected
    // bool is_sensor_disconnected = hall_50 < CURRENT_SENSOR_DISCONNECTED_THRESHOLD &&
    //     hall_300 < CURRENT_SENSOR_DISCONNECTED_THRESHOLD;
    if (volt_300 < CURRENT_SENSOR_DISCONNECTED_THRESHOLD)
        error_set(ERROR_FANS_DISCONNECTED, 1, HAL_GetTick());
}
