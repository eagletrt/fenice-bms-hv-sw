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

#include <string.h>
#include <math.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define TEMP_INIT_TIMEOUT 100

temperature_t temperatures[CELLBOARD_TEMP_SENSOR_COUNT];
temperature_t average                                   = 0;
temperature_t max                                       = 0;
temperature_t min                                       = 0;

static const uint8_t adc_addresses[6] = {
    ADCTEMP_CELL_1_ADR,
    ADCTEMP_CELL_2_ADR,
    ADCTEMP_CELL_3_ADR,
    ADCTEMP_CELL_4_ADR,
    ADCTEMP_CELL_5_ADR,
    ADCTEMP_CELL_6_ADR};

void temp_init() {
    for(uint8_t i=0; i<CELLBOARD_TEMP_SENSOR_COUNT; ++i){
        temperatures[i] = -20.f;
    }


    HAL_GPIO_WritePin(ADCTEMP_GPIO_PORT_BANK_0, ADCTEMP_GPIO_PIN_BANK_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ADCTEMP_GPIO_PORT_BANK_1, ADCTEMP_GPIO_PIN_BANK_1, GPIO_PIN_RESET);
    //initialize all ADCs
    for (uint8_t i = 0; i < TEMP_ADC_COUNT; i++) {
        uint8_t retry = 1;
        uint32_t time = HAL_GetTick();
        if(cellboard_index == 3 && i == 5) continue;
        if(cellboard_index == 0 && (i == 0 || i == 1 || i == 2)) continue;
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
    uint8_t is_disconnected = 1;

    for (uint8_t sens = ADCTEMP_INPUT_1_REG; sens <= ADCTEMP_INPUT_6_REG; sens++) {
        uint8_t temp_index = (adc_index * TEMP_ADC_SENSOR_COUNT) + sens;

        if (ADCTEMP_read_Temp(&ADC_I2C, adc_addresses[adc_index], sens, &temperatures[temp_index]) !=
            ADCTEMP_STATE_OK) {
            
            ERROR_SET(ERROR_TEMP_COMM_0 + adc_index);
        } else {
            ERROR_UNSET(ERROR_TEMP_COMM_0 + adc_index);

            if(temperatures[temp_index] > -19.f)
                is_disconnected = 0;

            max = MAX(temperatures[temp_index], max);
            min = MIN(temperatures[temp_index], min);
        }
    }

    if(is_disconnected)
        ERROR_SET(ERROR_TEMP_COMM_0 + adc_index);
}

void temp_measure_all() {
    for (uint8_t adc = 0; adc < TEMP_ADC_COUNT; adc++) {
        if(cellboard_index == 3 && adc == 5) continue;
        if(cellboard_index == 0 && (adc == 0 || adc == 1 || adc == 2)) continue;

        temp_measure(adc);
    }

    double sum = 0;
    for (uint16_t i = 0; i < CELLBOARD_TEMP_SENSOR_COUNT; i++) {
        sum += temperatures[i];
    }
    average = sum / CELLBOARD_TEMP_SENSOR_COUNT;
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

uint8_t temp_serialize(float temp) {
    return MIN(255, MAX(0, round(temp * 4.0)));
}