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
#include "adc124s021.h"
#include "spi.h"
#include "cli_bms.h"
#include "bms_fsm.h"

#include <stdio.h>
#include <math.h>

#define MEASURE_SAMPLE_SIZE 128

uint16_t adc_50[MEASURE_SAMPLE_SIZE]  = {0};
uint16_t adc_300[MEASURE_SAMPLE_SIZE] = {0};

current_t current[CURRENT_SENSOR_NUM] = {0.f};

float V0L = 0, V0H = 0;  //voltage offset (Vout(0A))
float shunt_offset;

current_t _current_convert_low(float volt) {
    return (499.f / 300.f / 40e-3f) * (volt - V0L);
}

current_t _current_convert_high(float volt) {
    return (499.f / 300.f / 6.67e-3f) * (volt - V0H);
}

current_t _current_convert_shunt(float volt) {
    return (volt - shunt_offset) / (1e-4f * 500);
}

void current_start_measure() {
    HAL_ADC_Start_DMA(&ADC_HALL50, (uint32_t *)adc_50, MEASURE_SAMPLE_SIZE);
    HAL_ADC_Start_DMA(&ADC_HALL300, (uint32_t *)adc_300, MEASURE_SAMPLE_SIZE);
}

uint32_t current_read(float shunt_adc_val) {
    uint32_t time    = HAL_GetTick();
    uint32_t avg_50  = 0;
    uint32_t avg_300 = 0;
    for (size_t i = 0; i < MEASURE_SAMPLE_SIZE; i++) {
        avg_50 += adc_50[i];
        avg_300 += adc_300[i];
    }

    // Convert Hall-low (50A)
    float volt        = avg_50 * (3.3f / 4095 / MEASURE_SAMPLE_SIZE);
    current[CURRENT_SENSOR_50] = _current_convert_low(volt);

    error_toggle_check(volt < 0.3, ERROR_CONNECTOR_DETACH, 2);

    // Convert Hall-high (300A)
    volt                        = avg_300 * (3.3f / 4095 / MEASURE_SAMPLE_SIZE);
    current[CURRENT_SENSOR_300] = _current_convert_high(volt);

    // Convert Shunt
    current[CURRENT_SENSOR_SHUNT] = _current_convert_shunt(shunt_adc_val);

    // Check for over-current
    error_toggle_check(current_get_current() > PACK_MAX_CURRENT, ERROR_OVER_CURRENT, 0);

    return time;
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

    if(V0L < 0.3){
        error_set(ERROR_CONNECTOR_DETACH, 2, HAL_GetTick());
        cli_bms_debug("sensor detached");
    }

    shunt_offset = adc124S021_read_channel(&SPI_ADC124S, ADC124_SHUNT_CHANNEL);
}

current_t current_get_current() {
    if (fabs(current[CURRENT_SENSOR_SHUNT]) < 4 && fabs(current[CURRENT_SENSOR_50]) < 5)
        return current[CURRENT_SENSOR_SHUNT];
    if (fabs(current[CURRENT_SENSOR_50]) < 34 && fabs(current[CURRENT_SENSOR_300]) < 35)
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
    error_toggle_check(current[CURRENT_SENSOR_300] > PACK_MAX_CURRENT, ERROR_OVER_CURRENT, 0);
}
