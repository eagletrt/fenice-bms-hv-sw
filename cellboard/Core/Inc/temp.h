/**
 * @file		temp.h
 * @brief		Temperature measurement functions
 *
 * @date		Jul 13, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include "cellboard_config.h"
#include "peripherals/adctemp.h"

#include <inttypes.h>

void temp_init();

/**
 * @brief Sets temperature limits
 * 
 * @param min Minimum temperature
 * @param max Maximum temperature
 */
void temp_set_limits(temperature_t min, temperature_t max);

/**
 * @brief Measures a single ADC
 * 
 * @param adc_index Index of ADC to measure
 * @param temperatures Output values
 */
void temp_measure(uint8_t adc_index, temperature_t temperatures[static TEMP_ADC_SENSOR_COUNT]);

/**
 * @brief Measures all ADCs
 * 
 * @param temperatures Output values
 */
void temp_measure_all(temperature_t temperatures[static CELLBOARD_TEMP_SENSOR_COUNT]);