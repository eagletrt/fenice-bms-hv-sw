
#ifndef BAL_H
#define BAL_H

#include <inttypes.h>
#include "fenice_config.h"

#define NULL_INDEX 255

typedef struct bal_conf_t {
	bool enable;
	uint16_t threshold;
	uint8_t slot_time;
} bal_conf_t;

void _bubble_sort(uint8_t indexes[PACK_MODULE_COUNT],
				  uint16_t values[PACK_MODULE_COUNT], uint8_t length);
uint8_t bal_compute_indexes(uint16_t volts[PACK_MODULE_COUNT],
							uint8_t indexes[PACK_MODULE_COUNT],
							uint16_t threshold);

#endif