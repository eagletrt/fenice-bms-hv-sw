/**
 * @file		ltc6813_utils.h
 * @brief		This file contains utilities for improving LTC6813
 * 				communications
 *
 * @date		Nov 16, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */
#ifndef LTC6813_UTILS_H
#define LTC6813_UTILS_H

#include <inttypes.h>
#include <stm32f4xx_hal.h>
#include "error.h"
#include "fenice_config.h"
#include "ltc6813.h"

uint8_t ltc6813_read_voltages(SPI_HandleTypeDef *spi, LTC6813_T *ltc,
							  uint16_t volts[], ERROR_STATUS_T volts_error[],
							  warning_t *warning, error_t *error);
void ltc6813_read_temperatures(SPI_HandleTypeDef *hspi, uint8_t max[2],
							   uint8_t min[2]);
void ltc6813_read_all_temps(SPI_HandleTypeDef *hspi, uint8_t *temps);

void ltc6813_check_voltage(uint16_t volt, ERROR_STATUS_T *volt_error,
						   warning_t *warning, error_t *error);
void ltc6813_check_temperature(uint16_t temp, ERROR_STATUS_T *temp_error,
							   error_t *error);

void ltc6813_set_dcc(uint8_t indexes[], uint8_t cfgar[8], uint8_t cfgbr[8]);
void ltc6813_set_balancing(SPI_HandleTypeDef *hspi, uint8_t *indexes, int dcto);

uint16_t ltc6813_convert_voltage(uint8_t v_data[]);
uint8_t ltc6813_convert_temp(uint8_t temps);

#endif