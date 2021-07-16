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

#define TEMP_INIT_TIMEOUT 100

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
        uint32_t time = HAL_GetTick();
        while (ADCTEMP_init_ADC(&hi2c1, adc_addresses[i], ADCTEMP_MONITORING_CONTINIOUS) != ADCTEMP_STATE_OK) {
            if (HAL_GetTick() - time >= TEMP_INIT_TIMEOUT) {
                ERROR_SET(ERROR_TEMP_COMM_0 + i);
                return;
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

void temp_measure(uint8_t adc_index, temperature_t temperatures[static TEMP_ADC_SENSOR_COUNT]) {
    for (uint8_t sens = ADCTEMP_INPUT_1_REG; sens <= ADCTEMP_INPUT_6_REG; sens++) {
        if (ADCTEMP_read_Temp(&ADC_I2C, adc_addresses[adc_index], sens, &temperatures[sens]) != ADCTEMP_STATE_OK) {
            ERROR_SET(ERROR_TEMP_COMM_0 + sens);
        } else {
            ERROR_UNSET(ERROR_TEMP_COMM_0 + sens);
        }
    }
}
void temp_measure_all(temperature_t temperatures[static CELLBOARD_TEMP_SENSOR_COUNT]) {
    for (uint8_t adc = 0; adc < TEMP_ADC_COUNT; adc++) {
        temp_measure(adc, &temperatures[adc * TEMP_ADC_SENSOR_COUNT]);
    }
}