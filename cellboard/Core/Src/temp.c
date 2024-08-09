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
#include <stdbool.h>

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
    ADCTEMP_CELL_6_ADR
};

static const bool excluded_temps[CELLBOARD_COUNT][CELLBOARD_TEMP_SENSOR_COUNT] = {
    [0] = { [7] = true, [26] = true },
    [1] = { [0] = true, [4] = true, [6] = true, [8] = true, [13] = true, [35] = true },
    [2] = { [15] = true, [21] = true, [26] = true, [29] = true },
    [3] = { [5] = true, [6] = true, [25] = true, [26] = true },
    [4] = { [1] = true, [3] = true, [13] = true, [14] = true, [23] = true, [35] = true },
    [5] = { [3] = true, [7] = true, [9] = true, [14] = true, [18] = true, [29] = true }
};

/**
 * @brief Check if a certain temperature sensor value has to be included
 * 
 * @param idx The index of the sensor
 * @return true If the value can be included
 * @return false Otherwise
 */
bool _temp_include_cell(size_t idx) {
    if (cellboard_index >= CELLBOARD_COUNT)
        return false;
    if (idx >= CELLBOARD_TEMP_SENSOR_COUNT)
        return false;
    return !excluded_temps[cellboard_index][idx];
}

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

            // Change excluded temperatures
            if (!_temp_include_cell(temp_index)) {
                size_t cnt = 0U;
                temperature_t temp = 0U;

                size_t base = (temp_index / 6U) * 6U;
                for (size_t i = base; i < base + 6U; ++i) {
                    if (_temp_include_cell(i)) {
                        temp += temperatures[i];
                        ++cnt;
                    }
                }
                // Average the temperatures
                temperatures[temp_index] = (cnt > 0) ? (temp / cnt) : average;
            }

            max = MAX(temperatures[temp_index], max);
            min = MIN(temperatures[temp_index], min);                
        }
    }
}

void temp_measure_all() {
    max = 0;
    min = CELL_MAX_TEMPERATURE;
    for (uint8_t adc = 0; adc < TEMP_ADC_COUNT; adc++) {
        temp_measure(adc);
    }

    double sum = 0;
    size_t tot = 0;
    for (uint16_t i = 0; i < CELLBOARD_TEMP_SENSOR_COUNT; i++) {
        sum += temperatures[i];
        tot++;
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