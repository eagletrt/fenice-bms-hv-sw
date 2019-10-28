
#ifndef BAL_H
#define BAL_H

#include <inttypes.h>
#include "fenice_config.h"

#define NULL_INDEX 255

typedef struct bal_conf {
	uint16_t threshold;
	uint8_t slot_time;
} bal_conf;

uint8_t bal_compute_indexes(bal_conf config, uint16_t volts[PACK_MODULE_COUNT],
							uint8_t indexes[]);

#endif