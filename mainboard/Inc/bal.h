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
typedef struct bal_handle {
	bool enable;
	uint16_t threshold;
	uint8_t slot_time;
} bal_handle;

/**
 * @brief	Computes the cells' to balance
 * 
 * @param	volts		Array of cell voltages
 * @param	indexes		Output array of indexes to be balanced
 * @param	threshold	Balancing tolerance (voltage +/- threshold)
 * 
 * @returns	Number of cells that are not in indexes but still need to be discharged
 */
uint8_t bal_compute_indexes(uint16_t volts[PACK_CELL_COUNT], uint8_t indexes[PACK_CELL_COUNT], uint16_t threshold);

#endif