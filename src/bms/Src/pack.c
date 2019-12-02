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

LTC6813_T ltc[LTC6813_COUNT];

/**
 * @brief	Initializes the pack
 *
 * @param	adc		The configuration structure for the ADC
 * @param	pack	The PACK_T struct to initialize
 */
void pack_init(PACK_T *pack) {
	pack->total_voltage = 0;
	pack->max_voltage = 0;
	pack->min_voltage = 0;
	pack->avg_temperature = 0;
	pack->max_temperature = 0;
	pack->min_temperature = 0;

	pack->current.value = 0;
	error_init(&pack->current.error);

	uint8_t i;
	for (i = 0; i < LTC6813_COUNT; i++) {
		ltc[i].address = (uint8_t)8 * i;
		// cell_distribution is not duplicated through the array of ltcs

		error_init(&ltc[i].error);
	}

	for (i = 0; i < PACK_MODULE_COUNT; i++) {
		pack->voltages[i] = 0;
		error_init(&pack->voltage_errors[i]);

		// Split this cycle if temperatures has a different size than voltage

		pack->temperatures[i] = 0;
		error_init(&pack->temperature_errors[i]);
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
uint8_t pack_update_voltages(SPI_HandleTypeDef *spi, PACK_T *pack,
							 warning_t *warning, error_t *error) {
	_ltc6813_adcv(spi, 0);

	uint8_t cell;
	uint8_t ltc_i;
	for (ltc_i = 0; ltc_i < LTC6813_COUNT; ltc_i++) {
		cell = ltc6813_read_voltages(spi, &ltc[ltc_i], pack->voltages,
									 pack->voltage_errors, warning, error);
		ER_CHK(error);
	}

End:;
	pack_update_voltage_stats(pack);

	if (*error == ERROR_LTC_PEC_ERROR) {
		return ltc_i;
	}
	return cell;
}

/**
 * @brief		Polls the LTCs for temperatures
 * @details	Temperature measurements with the current hardware architecture
 * 					lasts 600ms: too slow for the BMS. To avoid stopping for
 * 					that long, everytime this function is called only two LTCs
 * 					are polled, and only even or odd cells are measured from
 * 					them. When all LTCs have been polled, we start from the
 * 					beginning and we flip the odd/even bit to read the remaining
 * 					cells. This decreases the update frequency on a single cell,
 * 					but greatly improves (~10x) the blocking time of temperature
 * 					measurements. The time to update a single cell can be
 * 					calculated as following: 2*CELL_COUNT*(52ms+CALL_INTERVAL)
 *
 * @param		spi			The SPI configuration structure
 * @param		pack		The PACK_T struct to update
 * @param		error		The error return value
 *
 * @returns	The index of the last updated cell
 */
uint8_t pack_update_temperatures(SPI_HandleTypeDef *spi, PACK_T *pack,
								 error_t *error) {
	uint8_t send[8] = {0};
	ltc6813_wrcomm_i2c_w(spi, 69, 0x41);

	if (!ltc6813_rdcomm_i2c(spi, send)) {
		cli_print("pecw\r\n", 8);
	}
	ltc6813_stcomm_i2c(spi, 3);
	// HAL_Delay(1);
	///// read
	uint8_t recv[8] = {0};
	/*ltc6813_wrcomm_i2c_r(spi, 69);
	ltc6813_stcomm_i2c(spi, 4);

	// HAL_Delay(1);

	if (!ltc6813_rdcomm_i2c(spi, recv)) {
		cli_print("pecr\r\n", 8);
	}*/

	// uint8_t icom0 = recv[0] >> 4;
	uint8_t d0 = (recv[0] << 4) | (recv[1] >> 4);
	// uint8_t fcom0 = recv[1] & 0x0F;
	uint8_t d1 = (recv[2] << 4) | (recv[3] >> 4);
	uint8_t d2 = (recv[4] << 4) | (recv[5] >> 4);

	/*sprintf(kek, "\r\nicom0: %d\td0: %d\tfcom0: %d\td1: %d\r\n", icom0, d0,
		fcom0, d1);*/

	char kek[250] = {0};
	sprintf(
		kek,
		"SEND\tRECV\r\n%x\t%x\r\n%x\t%x\r\n%x\t%x\r\n%x\t%x\r\n%x\t%x\r\n%x\t%x"
		"\r\n%x\t%x\t%x\r\n\n",
		send[0], recv[0], send[1], recv[1], send[2], recv[2], send[3], recv[3],
		send[4], recv[4], send[5], recv[5], d0, d1, d2);

	cli_print(kek, strlen(kek));

	return 0;
}

/**
 * @brief		Calculates the current exiting/entering the pack
 *
 * @param		current	The current value to update
 * @param		error		The error return value
 */
void pack_update_current(ER_INT16_T *current, error_t *error) {
	int32_t tmp = 0;
	uint16_t i;
	for (i = 0; i < CURRENT_ARRAY_LENGTH; i++) {
		tmp += adc_current[i];
	}
	tmp /= CURRENT_ARRAY_LENGTH;

	// We calculate the input voltage
	float in_volt = (((float)tmp * 3.3) / 4096);

	// Check the current sensor datasheet for the correct formula
	current->value = (int16_t)(-round((((in_volt - 2.048) * 200 / 1.25)) * 10));
	current->value += 100;

	if (current->value > PACK_MAX_CURRENT) {
		// error_add()
		error_set(ERROR_OVER_CURRENT, &current->error, HAL_GetTick());
	} else {
		error_unset(ERROR_OVER_CURRENT, &current->error);
	}

	*error = error_check_fatal(&current->error, HAL_GetTick());
	ER_CHK(error);

End:;
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

	uint8_t i;
	for (i = 0; i < PACK_MODULE_COUNT; i++) {
		tot_voltage += (uint32_t)pack->voltages[i];

		if (!pack->voltage_errors[i].active) {
			max_voltage = fmax(max_voltage, pack->voltages[i]);
			min_voltage = fmin(min_voltage, pack->voltages[i]);
		}
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
	for (int i = 0; i < PACK_MODULE_COUNT; i++) {
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

bool pack_balance_cells(SPI_HandleTypeDef *spi, PACK_T *pack, bal_conf_t *conf,
						error_t *error) {
	uint8_t indexes[PACK_MODULE_COUNT];
	size_t len = bal_compute_indexes(pack->voltages, indexes, conf->threshold);

	if (len > 0) {
		char out[BUF_SIZE];
		sprintf(out, "\r\nBalancing cells\r\n");

		for (uint8_t i = 0; i < PACK_MODULE_COUNT; i++) {
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

uint8_t pack_check_errors(PACK_T *pack, error_t *error) {
	*error = ERROR_OK;
	warning_t warning;

	uint8_t i;
	for (i = 0; i < PACK_MODULE_COUNT; i++) {
		ltc6813_check_voltage(pack->voltages[i], &pack->voltage_errors[i],
							  &warning, error);
		ER_CHK(error);
		ltc6813_check_temperature(pack->temperatures[i],
								  &pack->temperature_errors[i], error);
		ER_CHK(error);
	}

End:
	return i == PACK_MODULE_COUNT ? 255 : i;
}
