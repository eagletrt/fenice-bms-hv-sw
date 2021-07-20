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

/**
 * @brief Serializes temperature into a CAN-friendly format
 * @details Temperatures are stored as floats locally. Floats are big and inefficient for CAN-bus.
 * This function maps temperatures in the range of 0.0 - 63.8 C to an 8-bit (0-255) integer value.
 * The resolution of the serialized data is 0.25C
 * 
 * @param temp Input temperature to serialize
 * @return uint8_t Serialized output
 */
uint8_t temp_serialize(float temp);