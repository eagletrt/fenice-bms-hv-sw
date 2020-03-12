/**
 * @file		data.h
 * @brief		
 *
 * @date		Mar 9, 2020
 * 
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef DATA_H
#define DATA_H

#include <inttypes.h>
#include <stdbool.h>
#include "../../fenice_config.h"

uint16_t voltages[PACK_CELL_COUNT]; /*!< [mV * 10] Cell voltages */

uint8_t temperatures[PACK_TEMP_COUNT]; /*!< [°C] */

uint32_t total_voltage; /*!< [mV * 10] Total pack voltage */
uint16_t max_voltage;	/*!< [mV * 10] Maximum cell voltage */
uint16_t min_voltage;	/*!< [mV * 10] Minimum cell voltage */

uint16_t avg_temperature; /*!< [°C * 100] Average pack temperature */
uint16_t max_temperature; /*!< [°C * 100] Maximum temperature */
uint16_t min_temperature; /*!< [°C * 100] Mimimum temperature */

int16_t current; /*!< [A * 10] Instant current draw. */

#endif