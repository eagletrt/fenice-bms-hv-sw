/**
 * @file		temperature.h
 * @brief		This file contains the functions to manage the cellboard
 * temperature
 *
 * @date		Feb 03, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "cellboard_config.h"
#include "stm32f4xx_hal.h"

typedef struct temperature {
	uint8_t values[TEMP_SENSOR_COUNT];

	uint8_t min[2];
	uint8_t max[2];
} temperature_t;

void temperature_read_sample(I2C_HandleTypeDef *hi2c[TEMP_BUS_COUNT],
							 uint16_t *buffer);

void temperature_get_average(
	uint16_t buffer[TEMP_SENSOR_COUNT][TEMP_SAMPLE_COUNT],
	uint8_t temps[TEMP_SENSOR_COUNT]);

/**
 * @brief Computes the two highest and lowest temperatures
 *
 * @param		temps		The input array
 * @param		min			The lowest temperature output array
 * @param		max			The highest temperature output array
 */
void temperature_get_extremes(uint8_t temps[], uint8_t min[2], uint8_t max[2]);
#endif