/**
 * @file		pack.h
 * @brief		This file contains the functions to manage the battery pack
 *
 * @date		Apr 11, 2019
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <inttypes.h>

#include "bal.h"
#include "error/error.h"
#include "fenice_config.h"
#include "peripherals/ltc6813_utils.h"

void pack_init();

void pack_update_voltages(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart);
void pack_update_temperatures(SPI_HandleTypeDef *hspi);
void pack_update_temperatures_all(SPI_HandleTypeDef *hspi);
void pack_update_current();
void pack_update_voltage_stats();
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
bal_handle pack_get_balancing();
