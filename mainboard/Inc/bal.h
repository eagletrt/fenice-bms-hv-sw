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

#define NULL_INDEX 255

typedef struct bal_conf {
	bool enable;
	uint16_t threshold;
	uint8_t slot_time;
} bal_conf_t;

void _bubble_sort(uint8_t indexes[PACK_CELL_COUNT], uint16_t values[PACK_CELL_COUNT], uint8_t length);
uint8_t bal_compute_indexes(uint16_t volts[PACK_CELL_COUNT], uint8_t indexes[PACK_CELL_COUNT], uint16_t threshold);

#endif