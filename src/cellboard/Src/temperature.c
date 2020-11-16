/**
 * @file		temperature.c
 * @brief		This file contains the functions to manage the cellboard
 * temperature
 *
 * @date		Feb 03, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */
#include "temperature.h"

#include "pct2075.h"

void temperature_read_sample(I2C_HandleTypeDef *hi2c[TEMP_BUS_COUNT],
							 uint16_t *buffer) {
	uint8_t count = 0;

	// For each bus, for each strip, read all sensors
	for (uint8_t bus = 0; bus < TEMP_BUS_COUNT; bus++) {
		for (uint8_t strip = 0; strip < TEMP_STRIPS_PER_BUS; strip++) {
			for (uint8_t sens = 0; sens < TEMP_SENSORS_PER_STRIP; sens++) {
				// Sum strip to change LSB of address coding
				buffer[count++] = pct2075_read(
					hi2c[bus], TEMP_SENSOR_ADDRESS_CODING[sens] + strip);
			}
		}
	}
}

void temperature_get_average(
	uint16_t buffer[TEMP_SENSOR_COUNT][TEMP_SAMPLE_COUNT],
	uint8_t temps[TEMP_SENSOR_COUNT]) {
	for (uint8_t sens = 0; sens < TEMP_SENSOR_COUNT; sens++) {
		temps[sens] = 0;

		for (uint8_t sample = 0; sample < TEMP_SAMPLE_COUNT; sample++) {
			temps[sens] += buffer[sens][sample];
			buffer[sens][sample] = 0;
		}
		temps[sens] /= TEMP_SAMPLE_COUNT;
	}
}

void temperature_get_extremes(uint8_t temps[], uint8_t min[2], uint8_t max[2]) {
	max[0] = max[1] = 0;
	min[0] = min[1] = UINT8_MAX;

	for (uint8_t i = 0; i < TEMP_SENSOR_COUNT; i++) {
		if (temps[i] >= max[0]) {
			max[1] = max[0];
			max[0] = temps[i];
		} else if (temps[i] > max[1]) {
			max[1] = temps[i];
		}

		if (temps[i] <= min[0]) {
			min[1] = min[0];
			min[0] = temps[i];
		} else if (temps[i] < min[1]) {
			min[1] = temps[i];
		}
	}
}