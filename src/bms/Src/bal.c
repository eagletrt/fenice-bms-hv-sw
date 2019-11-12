#include <bal.h>
#include <stdio.h>

inline void _swap(uint8_t *val1, uint8_t *val2) {
	uint8_t tmp = *val2;
	*val2 = *val1;
	*val1 = tmp;
}

uint8_t _max_index(uint16_t data[], size_t length) {
	uint8_t max = 0;
	for (uint8_t i = 0; i < length; i++) {
		if (data[i] > data[max])
			max = i;
	}

	return max;
}

uint8_t _min_index(uint16_t data[], size_t length) {
	uint8_t min_value_index = 0;
	for (uint8_t i = 0; i < length; i++) {
		if (data[i] < data[min_value_index])
			min_value_index = i;
	}

	return min_value_index;
}

void _bubble_sort(uint8_t indexes[PACK_MODULE_COUNT],
				  uint16_t values[PACK_MODULE_COUNT], uint8_t length) {
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

uint8_t bal_compute_indexes(uint16_t volts[], uint8_t indexes[],
							uint16_t threshold) {
	static bool even = true;  // Whether to return even or odd cells

	uint8_t indexes_left = PACK_MODULE_COUNT;  // cells to check
	uint8_t min_index = _min_index(volts, PACK_MODULE_COUNT);

	for (uint8_t i = 0; i < PACK_MODULE_COUNT; i++) {
		indexes[i] = i;  // Initialize indexes
	}

	// sort all indexes by voltage
	_bubble_sort(indexes, volts, PACK_MODULE_COUNT);

	// mark indexes that represent values that are less than
	// (minval + tresh)
	for (uint8_t i = 0; i < PACK_MODULE_COUNT; i++) {
		if (volts[indexes[i]] < volts[min_index] + threshold) {
			indexes_left--;
			indexes[i] = NULL_INDEX;
		}

		if (indexes[i] % 2 == even) {
			indexes[i] = NULL_INDEX;
		}
	}
	even = !even;

	// if all modules are balanced
	if (indexes_left == 0) {
		return 0;
	}

	// return number of indexes to compute
	return indexes_left;
}
