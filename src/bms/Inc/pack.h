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
#include "comm/ltc6813_utils.h"
#include "error/error.h"

/** @brief Battery pack basic info */
typedef struct {
	uint16_t voltages[PACK_CELL_COUNT]; /*!< [mV * 10] Cell voltages */
	error_status_t voltage_errors[PACK_CELL_COUNT];

	uint8_t temperatures[PACK_TEMP_COUNT]; /*!< [°C] */
	error_status_t temperature_errors[PACK_CELL_COUNT];

	uint32_t total_voltage; /*!< [mV * 10] Total pack voltage */
	uint16_t max_voltage;	/*!< [mV * 10] Maximum cell voltage */
	uint16_t min_voltage;	/*!< [mV * 10] Minimum cell voltage */

	uint16_t avg_temperature; /*!< [°C * 100] Average pack temperature */
	uint16_t max_temperature; /*!< [°C * 100] Maximum temperature */
	uint16_t min_temperature; /*!< [°C * 100] Mimimum temperature */

	int16_t current; /*!< [A * 10] Instant current draw. */
} PACK_T;

void pack_init(PACK_T *pack);

void pack_update_voltages(SPI_HandleTypeDef *spi, PACK_T *pack);
void pack_update_temperatures(SPI_HandleTypeDef *spi, PACK_T *pack);
void pack_update_temperatures_all(SPI_HandleTypeDef *spi, uint8_t *temps);

void pack_update_current(int16_t *current);
void pack_update_voltage_stats(PACK_T *pack);
void pack_update_temperature_stats(PACK_T *pack);
bool pack_balance_cells(SPI_HandleTypeDef *spi, PACK_T *pack, bal_conf_t *conf);
uint8_t pack_check_voltage_drops(PACK_T *pack,
								 uint8_t cells[PACK_CELL_COUNT]);

#endif /* PACK_H_ */
