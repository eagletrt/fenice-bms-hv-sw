/**
 * @file		bal.c
 * @brief		This file contains the balancing functions
 *
 * @date		Oct 28, 2019
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include <bal.h>
#include <stdio.h>

/**
 * @brief swap two uint8's
 */
void _swap(uint8_t *val1, uint8_t *val2) {
	uint8_t tmp = *val2;
	*val2 = *val1;
	*val1 = tmp;
}

/**
 * @returns	The index of the maximum value of data
 */
uint8_t _max_index(uint16_t data[], size_t length) {
	uint8_t max = 0;
	for (uint8_t i = 0; i < length; i++) {
		if (data[i] > data[max])
			max = i;
	}

	return max;
}

/**
 * @returns	The index of the minimum value of data
 */
uint8_t _min_index(uint16_t data[], size_t length) {
	uint8_t min_value_index = 0;
	for (uint8_t i = 0; i < length; i++) {
		if (data[i] < data[min_value_index])
			min_value_index = i;
	}

	return min_value_index;
}

/**
 * @brief	Sorts indexes
 * 
 * @param	indexes	Indexes to be sorted
 * @param	values	Values to compare
 * @param	length	Length of both arrays
 */
void _bubble_sort(uint8_t indexes[PACK_CELL_COUNT], uint16_t values[PACK_CELL_COUNT], uint8_t length) {
	// TODO: Do we need to pass length? Can't it just be a local var?
	while (length > 1) {
		uint8_t newn = 0;
		for (uint8_t i = 0; i < length - 1; i++) {
			if (values[indexes[i]] < values[indexes[i + 1]]) {
				_swap(&indexes[i], &indexes[i + 1]);
				newn = i;
			}
		}
		length = newn;
	}
}

void bal_init(bal_handle *bal) {
	bal->enable = false;
	bal->slot_time = BAL_MAX_VOLTAGE_THRESHOLD;
	bal->threshold = 2;
}

uint8_t bal_compute_indexes(uint16_t volts[], uint8_t indexes[], uint16_t threshold) {
	uint8_t indexes_left = PACK_CELL_COUNT;	 // cells to check
	uint8_t min_index = _min_index(volts, PACK_CELL_COUNT);

	for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
		indexes[i] = i;	 // Initialize indexes
	}

	// sort all indexes by voltage
	_bubble_sort(indexes, volts, PACK_CELL_COUNT);

	for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
		if (volts[indexes[i]] > volts[min_index] + threshold) {
			// If current cell needs to be discahrged
			if ((i == 0 || indexes[i - 1] == BAL_NULL_INDEX) && (i == PACK_CELL_COUNT - 1 || indexes[i + 1] != BAL_NULL_INDEX)) {
				// If previous cell is NULL and next cell needs to be discharged, then set next to NULL
				indexes[i + 1] = BAL_NULL_INDEX;
				// We don't decrease indexes_left here because the cell still needs to be discharged.
			}
		} else {
			// No need to balance
			indexes_left--;
			indexes[i] = BAL_NULL_INDEX;
		}
	}

	return indexes_left;
}
