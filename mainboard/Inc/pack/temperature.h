/**
 * @file    temperature.h
 * @brief   Functions to manage all cell temperatures
 *
 * @date    Apr 11, 2019
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <inttypes.h>
#include <mainboard_config.h>

typedef uint8_t temperature_t;

typedef enum { TEMP_CELLBOARD_0_OFFSET = 0, 
    TEMP_CELLBOARD_1_OFFSET = TEMP_SENSOR_COUNT*1, 
    TEMP_CELLBOARD_2_OFFSET = TEMP_SENSOR_COUNT*2, 
    TEMP_CELLBOARD_3_OFFSET = TEMP_SENSOR_COUNT*3, 
    TEMP_CELLBOARD_4_OFFSET = TEMP_SENSOR_COUNT*4, 
    TEMP_CELLBOARD_5_OFFSET = TEMP_SENSOR_COUNT*5} TEMP_CELLBOARD_START_INDEX;

void temperature_init();

/**
 * @brief	Updates the pack's temperature stats
 * @details It updates *_temperature variables with the data of the pack
 */
void temperature_check();

temperature_t *temperature_get_all();
temperature_t temperature_get_max();
temperature_t temperature_get_min();
temperature_t temperature_get_average();
void temperature_set_cells(uint8_t index, temperature_t t1, temperature_t t2, temperature_t t3, temperature_t t4, temperature_t t5, temperature_t t6);
