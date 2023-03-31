#include "test_volt_data.h"

#include "mainboard_config.h"

#include <munit.h>

uint16_t volt_max(uint16_t volts[]) {
    uint16_t max = 0;
    for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
        if (volts[i] > max) {
            max = volts[i];
        }
    }

    return max;
}

uint16_t volt_min(uint16_t volts[]) {
    uint16_t min = UINT16_MAX;
    for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
        if (volts[i] < min) {
            min = volts[i];
        }
    }
    return min;
}

void volt_gen_random(uint16_t volts[], uint16_t max_delta, uint16_t center) {
    for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
        volts[i] = center + munit_rand_int_range(-max_delta / 2, max_delta / 2);
    }
}