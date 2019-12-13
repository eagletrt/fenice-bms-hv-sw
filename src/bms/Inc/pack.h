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
#include "bal.h"
#include "comm/ltc6813_utils.h"
#include "error.h"
#include "fenice_config.h"

/** @brief Battery pack basic info */
typedef struct {
	uint16_t voltages[PACK_MODULE_COUNT]; /*!< [mV * 10] Cell voltages */
	ERROR_STATUS_T voltage_errors[PACK_MODULE_COUNT];

	uint8_t temperatures[LTC6813_TEMP_COUNT * LTC6813_COUNT]; /*!< [°C] */
	ERROR_STATUS_T temperature_errors[PACK_MODULE_COUNT];

	uint32_t total_voltage; /*!< [mV * 10] Total pack voltage */
	uint16_t max_voltage;   /*!< [mV * 10] Maximum cell voltage */
	uint16_t min_voltage;   /*!< [mV * 10] Minimum cell voltage */

	uint16_t avg_temperature; /*!< [°C * 100] Average pack temperature */
	uint16_t max_temperature; /*!< [°C * 100] Maximum temperature */
	uint16_t min_temperature; /*!< [°C * 100] Mimimum temperature */

	ER_INT16_T current; /*!< [A * 10] Instant current draw. */
} PACK_T;

void pack_init(PACK_T *pack);
uint8_t pack_update_voltages(SPI_HandleTypeDef *spi, PACK_T *pack,
							 warning_t *warning, error_t *error);
void pack_update_temperatures(SPI_HandleTypeDef *spi, PACK_T *pack);
void pack_update_temperatures_all(SPI_HandleTypeDef *spi, uint8_t *temps);

void pack_update_current(ER_INT16_T *current, error_t *error);
void pack_update_voltage_stats(PACK_T *pack);
void pack_update_temperature_stats(PACK_T *pack);
bool pack_balance_cells(SPI_HandleTypeDef *spi, PACK_T *pack, bal_conf_t *conf,
						error_t *error);
uint8_t pack_check_errors(PACK_T *pack, error_t *error);
uint8_t pack_check_voltage_drops(PACK_T *pack,
								 uint8_t cells[PACK_MODULE_COUNT]);

#endif /* PACK_H_ */
