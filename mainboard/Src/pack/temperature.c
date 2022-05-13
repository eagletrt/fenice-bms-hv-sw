/**
 * @file    temperature.c
 * @brief   Functions to manage all cell temperatures
 *
 * @date    Apr 11, 2019
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "pack/temperature.h"

#include "bms_fsm.h"
#include "error/error.h"
#include "main.h"
#include "mainboard_config.h"

#include <inttypes.h>
#include <math.h>
#include <string.h>

temperature_t temperatures[PACK_TEMP_COUNT];

void temperature_init() {
    memset(temperatures, 0, sizeof(temperatures));
}

void temperature_check_errors() {
    for (size_t i = 0; i < PACK_TEMP_COUNT; i++) {
        error_toggle_check(temperatures[i] > CELL_MAX_TEMPERATURE, ERROR_CELL_OVER_TEMPERATURE, i);
    }
}

temperature_t *temperature_get_all() {
    return temperatures;
}
temperature_t temperature_get_max() {
    temperature_t max_temp = 0;
    for (size_t i = 0; i < PACK_TEMP_COUNT; i++) {
        max_temp = MAX(max_temp, temperatures[i]);
    }
    return max_temp;
}

temperature_t temperature_get_min() {
    temperature_t min_temp = 0;
    for (size_t i = 0; i < PACK_TEMP_COUNT; i++) {
        min_temp = MIN(min_temp, temperatures[i]);
    }
    return min_temp;
}

temperature_t temperature_get_average() {
    size_t average = 0;
    for (size_t i = 0; i < PACK_TEMP_COUNT; i++) {
        average += temperatures[i];
    }
    return (temperature_t)round(average / PACK_TEMP_COUNT);
}

void temperature_set_cells(
    uint8_t index,
    temperature_t t1,
    temperature_t t2,
    temperature_t t3,
    temperature_t t4,
    temperature_t t5,
    temperature_t t6) {
    temperatures[index]     = t1;
    temperatures[index + 1] = t2;
    temperatures[index + 2] = t3;
    temperatures[index + 3] = t4;
    temperatures[index + 4] = t5;
    temperatures[index + 5] = t6;
}

uint8_t temperature_get_cellboard_offset(uint8_t cellboard_index) {
    uint8_t index = 0;
    while (bms_get_cellboard_distribution()[index] != cellboard_index)
        ++index;
    return index * TEMP_SENSOR_COUNT;
}