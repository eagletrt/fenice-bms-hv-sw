/**
 * @file		pack.h
 * @brief		This file contains the functions to manage the battery pack
 *
 * @date		Apr 11, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef PACK_H_
#define PACK_H_

#include <inttypes.h>

#include "../../fenice_config.h"
#include "bal.h"
#include "error/error.h"
#include "pack_data.h"
#include "peripherals/ltc6813_utils.h"

extern bal_conf_t balancing;  // TODO: Remove bal_conf_t struct (remove enable and the rest has to be managed by the state machine with the eeprom too)

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

bool pack_feedback_check(FEEDBACK_T fb_check_mask, FEEDBACK_T fb_value, error_id error_id);
bool pack_feedback_check_charge();
bool pack_feedback_check_precharge();
bool pack_feedback_check_on();

#endif
