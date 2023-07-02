/**
 * @file		temp.c
 * @brief		Temperature measurement functions
 *
 * @date		Jul 13, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "temp.h"

#include "error.h"
#include "i2c.h"

#include <math.h>
#include <float.h>

#define TEMP_INIT_TIMEOUT 100

temperature_t temperatures[CELLBOARD_TEMP_SENSOR_COUNT] = { 0 };
temperature_t average = 0;
temperature_t max = 0;
temperature_t min = CELL_MAX_TEMPERATURE;

static const uint8_t adc_addresses[6] = {
    ADCTEMP_CELL_1_ADR,
    ADCTEMP_CELL_2_ADR,
    ADCTEMP_CELL_3_ADR,
    ADCTEMP_CELL_4_ADR,
    ADCTEMP_CELL_5_ADR,
    ADCTEMP_CELL_6_ADR};

void temp_init() {
    //initialize all ADCs
    for (uint8_t i = 0; i < TEMP_ADC_COUNT; i++) {
        uint8_t retry = 1;
        uint32_t time = HAL_GetTick();
        while (ADCTEMP_init_ADC(&hi2c1, adc_addresses[i], ADCTEMP_MONITORING_CONTINIOUS) != ADCTEMP_STATE_OK && retry) {
            if (HAL_GetTick() - time >= TEMP_INIT_TIMEOUT) {
                ERROR_SET(ERROR_TEMP_COMM_0 + i);
                retry = 0;
            } else {
                ERROR_UNSET(ERROR_TEMP_COMM_0 + i);
            }
        }
    }
}

void temp_set_limits(temperature_t min, temperature_t max) {
    assert_param(max >= min);

    for (uint8_t i = 0; i < TEMP_ADC_COUNT; i++) {
        for (uint8_t sens = ADCTEMP_INPUT_1_REG; sens <= ADCTEMP_INPUT_6_REG; sens++) {
            ADCTEMP_set_Temperature_Limit(&hi2c1, adc_addresses[i], sens, max, min);
        }
    }
}

void temp_measure(uint8_t adc_index) {
    for (uint8_t sens = ADCTEMP_INPUT_1_REG; sens <= ADCTEMP_INPUT_6_REG; sens++) {
        uint8_t temp_index = (adc_index * TEMP_ADC_SENSOR_COUNT) + sens;

        if (ADCTEMP_read_Temp(&ADC_I2C, adc_addresses[adc_index], sens, &temperatures[temp_index]) !=
            ADCTEMP_STATE_OK) {
            ERROR_SET(ERROR_TEMP_COMM_0 + adc_index);
        } else {
            ERROR_UNSET(ERROR_TEMP_COMM_0 + adc_index);

            max = MAX(temperatures[temp_index], max);
            // Skip temperature below cell minimum temperature
            // FIXME: Broken temperature readings
            // if (temperatures[temp_index] >= CELL_MIN_TEMPERATURE + FLT_EPSILON)
            if (temperatures[temp_index] >= -5.f)
                min = MIN(temperatures[temp_index], min);
        }
    }
}

void temp_measure_all() {
    for (uint8_t adc = 0; adc < TEMP_ADC_COUNT; adc++) {
        temp_measure(adc);
    }

    double sum = 0;
    size_t tot = 0;
    for (uint16_t i = 0; i < CELLBOARD_TEMP_SENSOR_COUNT; i++) {
        // FIXME: Broken temperature readings
        // if (temperatures[i] >= CELL_MIN_TEMPERATURE + FLT_EPSILON) {
        if (temperatures[i] >= -5.f) {
            sum += temperatures[i];
            tot++;
        }
    }
    average = sum / tot;
}

temperature_t temp_get_average() {
    return average;
}

temperature_t temp_get_max() {
    return max;
}

temperature_t temp_get_min() {
    return min;
}