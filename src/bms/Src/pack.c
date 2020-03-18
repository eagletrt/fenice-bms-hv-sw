/**
 * @file		pack.c
 * @brief		This file contains the functions to manage the battery pack
 *
 * @date		Apr 11, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "pack.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stm32f4xx_hal.h>
#include <string.h>

#include "bal.h"
#include "cli.h"

#define CURRENT_ARRAY_LENGTH 512

uint32_t adc_current[CURRENT_ARRAY_LENGTH];

/**
 * @brief	Initializes the pack
 *
 * @param	adc		The configuration structure for the ADC
 * @param	pack	The PACK_T struct to initialize
 */
void pack_init(PACK_T *pack) {
	pack->adc_voltage = 0;
	pack->ext_voltage = 0;
	pack->total_voltage = 0;
	pack->max_voltage = 0;
	pack->min_voltage = 0;
	pack->avg_temperature = 0;
	pack->max_temperature = 0;
	pack->min_temperature = 0;

	pack->current = 0;
	// error_init(&pack->current.error);

	for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
		pack->voltages[i] = 0;
		//error_init(&pack->voltage_errors[i]);
	}

	for (uint8_t i = 0; i < TEMP_SENSOR_COUNT * LTC6813_COUNT; i++) {
		pack->temperatures[i] = 0;
		//error_init(&pack->temperature_errors[i]);
	}

	// LTC6813 GPIO configuration
	GPIO_CONFIG = GPIO_I2C_MODE;
}

/**
 * @brief		Polls all the LTCs for voltages
 * @details	This function should take 10~11ms to fully execute
 *
 * @param		spi				The SPI configuration structure
 * @param		pack			The PACK_T struct to update
 * @param		warning		The warning return value
 * @param		error			The error return value
 *
 * @returns	The index of the last updated cell
 */
void pack_update_voltages(SPI_HandleTypeDef *spi, PACK_T *pack) {
	_ltc6813_adcv(spi, 0);

	ltc6813_read_voltages(spi, pack->voltages);

	pack_update_voltage_stats(pack);
}

/**
 * @brief		Gets quick temperature stats from the cellboards
 * @details	
 *
 * @param		spi			The SPI configuration structure
 * @param		pack		The PACK_T struct to update
 */
void pack_update_temperatures(SPI_HandleTypeDef *spi, PACK_T *pack) {
	uint8_t max[LTC6813_COUNT * 2];
	uint8_t min[LTC6813_COUNT * 2];

	pack->avg_temperature = 0;
	pack->max_temperature = 0;
	pack->min_temperature = UINT8_MAX;
	ltc6813_read_temperatures(spi, max, min);

	for (uint8_t i = 0; i < LTC6813_COUNT; i++) {
		pack->avg_temperature += max[i * 2] + max[i * 2 + 1];
		pack->avg_temperature += min[i * 2] + min[i * 2 + 1];

		pack->max_temperature = fmax(max[i * 2], pack->max_temperature);
		pack->min_temperature = fmin(min[i * 2], pack->min_temperature);
	}

	pack->avg_temperature =
		((float)pack->avg_temperature / (LTC6813_COUNT * 4)) * 10;
}

void pack_update_temperatures_all(SPI_HandleTypeDef *spi, uint8_t *temps) {
	ltc6813_read_all_temps(spi, temps);
}

/**
 * @brief		Calculates the current exiting/entering the pack
 *
 * @param		current	The current value to update
 * @param		error		The error return value
 */
void pack_update_current(int16_t *current) {
	int32_t tmp = 0;
	uint16_t i;
	for (i = 0; i < CURRENT_ARRAY_LENGTH; i++) {
		tmp += adc_current[i];
	}
	tmp /= CURRENT_ARRAY_LENGTH;

	// We calculate the input voltage
	float in_volt = (((float)tmp * 3.3) / 4096);

	// Check the current sensor datasheet for the correct formula
	*current = (int16_t)(-round((((in_volt - 2.048) * 200 / 1.25)) * 10));
	*current += 100;

	if (*current > PACK_MAX_CURRENT) {
		error_set(ERROR_OVER_CURRENT, 0, HAL_GetTick());
	} else {
		error_unset(ERROR_OVER_CURRENT, 0);
	}
}

/**
 * @brief		Updates the pack's voltage stats
 * @details	It updates *_voltage variables with the data of the pack
 *
 * @param		pack	The struct to save the data to
 */
void pack_update_voltage_stats(PACK_T *pack) {
	uint32_t tot_voltage = 0;
	uint16_t max_voltage = pack->voltages[0];
	uint16_t min_voltage = UINT16_MAX;

	for (uint16_t i = 0; i < PACK_CELL_COUNT; i++) {
		tot_voltage += (uint32_t)pack->voltages[i];

		// TODO: Check for errors
		//if (!pack->voltage_errors[i].active) {
		max_voltage = fmax(max_voltage, pack->voltages[i]);
		min_voltage = fmin(min_voltage, pack->voltages[i]);
		//}
	}

	pack->total_voltage = tot_voltage;
	pack->max_voltage = max_voltage;
	pack->min_voltage = fmin(min_voltage, max_voltage);
}

/**
 * @brief		Updates the pack's temperature stats
 * @details	It updates *_temperature variables with the data of the pack
 *
 * @param		pack	The struct to save the data to
 */
void pack_update_temperature_stats(PACK_T *pack) {
	uint32_t avg_temperature = 0;
	uint16_t max_temperature = 0;
	uint16_t min_temperature = UINT16_MAX;

	uint8_t temp_count = 0;
	for (uint16_t i = 0; i < TEMP_SENSOR_COUNT; i++) {
		if (pack->temperatures[i] > 0) {
			avg_temperature += (uint32_t)pack->temperatures[i];

			max_temperature = fmax(max_temperature, pack->temperatures[i]);
			min_temperature = fmin(min_temperature, pack->temperatures[i]);
			temp_count++;
		}
	}

	pack->avg_temperature = (uint16_t)(avg_temperature / temp_count);
	pack->max_temperature = max_temperature;
	pack->min_temperature = fmin(min_temperature, max_temperature);
}

bool pack_balance_cells(SPI_HandleTypeDef *spi, PACK_T *pack, bal_conf_t *conf) {
	uint8_t indexes[PACK_CELL_COUNT];
	size_t len = bal_compute_indexes(pack->voltages, indexes, conf->threshold);

	if (len > 0) {
		char out[BUF_SIZE];
		sprintf(out, "\r\nBalancing cells\r\n");

		for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
			if (indexes[i] < NULL_INDEX) {
				sprintf(out + strlen(out), "%d ", indexes[i]);
			}
		}
		sprintf(out + strlen(out), "\r\n");
		cli_print(out, strlen(out));

		ltc6813_set_balancing(spi, indexes, conf->slot_time);
		return true;
	}
	cli_print("\r\nnothing to balance\r\n", 22);
	return false;
}
