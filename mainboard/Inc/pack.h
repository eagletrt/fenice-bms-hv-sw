/**
 * @file	pack.h
 * @brief	This file contains the functions to manage the battery pack
 *
 * @date 	Apr 11, 2019
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <inttypes.h>

#include "bal.h"
#include "error/error.h"
#include "fenice_config.h"
#include "peripherals/ltc6813_utils.h"

void pack_init();

/**
 * @brief   Polls all the LTCs and the SI8900 ADC for voltages
 * @details	This function should take 10~11ms to fully execute
 *
 * @param	hspi    The SPI configuration structure
 * @param	huart   The UART configuration structure
 * @returns The index of the last updated cell
 */
void pack_update_voltages(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart);

/**
 * @brief	Gets quick temperature stats from the cellboards
 *
 * @param	hspi    The SPI configuration structure
 */
void pack_update_temperatures(SPI_HandleTypeDef *hspi);
void pack_update_temperatures_all(SPI_HandleTypeDef *hspi);

/**
 * @brief   Calculates the current exiting/entering the pack
 */
void pack_update_current();

/**
 * @brief   Updates the pack's voltage stats
 * @details It updates *_voltage variables with the data of the pack
 */
void pack_update_voltage_stats();

/**
 * @brief	Updates the pack's temperature stats
 * @details It updates *_temperature variables with the data of the pack
 */
void pack_update_temperature_stats();
bool pack_balance_cells(SPI_HandleTypeDef *hspi);

bool pack_set_ts_off();
bool pack_set_pc_start();
bool pack_set_precharge_end();

voltage_t *pack_get_voltages();
voltage_t pack_get_max_voltage();
voltage_t pack_get_min_voltage();
voltage_t pack_get_bus_voltage();
voltage_t pack_get_int_voltage();
temperature_t *pack_get_temperatures();
temperature_t pack_get_max_temperature();
temperature_t pack_get_min_temperature();
temperature_t pack_get_mean_temperature();
current_t pack_get_current();
int16_t pack_get_power();
bal_handle pack_get_balancing();
