/**
 * @file		temp.h
 * @brief		Temperature measurement functions
 *
 * @date		Jul 13, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef TEMP_H
#define TEMP_H

#include "cellboard_config.h"
#include "peripherals/adctemp.h"

#include <inttypes.h>

#define CONVERT_VALUE_TO_TEMPERATURE(x) ((float)(x) / 2.56 - 20)
#define CONVERT_TEMPERATURE_TO_VALUE(x) (((x) + 20) * 2.56)

extern temperature_t temperatures[CELLBOARD_TEMP_SENSOR_COUNT];

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
void temp_measure(uint8_t adc_index);

/**
 * @brief Measures all ADCs
 * 
 * @param temperatures Output values
 */
void temp_measure_all();

temperature_t temp_get_average();
temperature_t temp_get_max();
temperature_t temp_get_min();

#endif // TEMP_H