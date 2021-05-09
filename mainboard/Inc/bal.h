/**
 * @file		bal.h
 * @brief		This file contains the balancing functions
 *
 * @date		Oct 28, 2019
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef BAL_H
#define BAL_H

#include <inttypes.h>
#include <stdbool.h>

#include "fenice_config.h"

#define BAL_NULL_INDEX UINT8_MAX

/**
 * @brief Balancing configuration handle
 */
typedef struct bal_config {
	uint16_t threshold;
	uint32_t slot_time;
} bal_config;

extern bal_config bal;

/**
 * @brief	Computes the cells' to balance
 * 
 * @param	volts		Array of cell voltages
 * @param	threshold	Balancing tolerance (voltage +/- threshold)
 * @param	indexes		Output array of indexes to be balanced
 * 
 * @returns	Number of cells that are not in indexes but still need to be discharged
 */
uint16_t bal_compute_indexes(uint16_t volts[], uint16_t threshold, uint16_t indexes[]);
#endif