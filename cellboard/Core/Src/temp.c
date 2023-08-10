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

/**
 * @brief Check if a certain temperature sensor value has to be included
 * 
 * @param idx The index of the sensor
 * @return true If the value can be included
 * @return false Otherwise
 */
bool _temp_include_cell(size_t idx) {
    switch (cellboard_index)
    {
        case 0:
            return true;
        case 1:
            return idx != 0 && idx != 4 && idx != 6 && idx != 13;
        case 2:
            return idx != 29;
        case 3:
            return idx != 5 && idx != 25 && idx != 26;
        case 4:
            return idx != 1 && idx != 3 && idx != 13;
        case 5:
            return idx != 7 && idx != 9 && idx != 29;

        default:
            return false;
    }
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

            max = MAX(temperatures[temp_index], max);
            // Skip broken temperature readings
            if (_temp_include_cell(temp_index))
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
        // Skip broken temperature readings
        if (_temp_include_cell(i)) {
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