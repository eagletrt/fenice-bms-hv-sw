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

void temperature_init();

/**
 * @brief	Updates the pack's temperature stats
 * @details It updates *_temperature variables with the data of the pack
 */
void temperature_check_errors();

temperature_t *temperature_get_all();
temperature_t temperature_get_max();
temperature_t temperature_get_min();
temperature_t temperature_get_average();
void temperature_set_cells(
    uint8_t index,
    temperature_t t1,
    temperature_t t2,
    temperature_t t3,
    temperature_t t4,
    temperature_t t5,
    temperature_t t6);
uint8_t temperature_get_cellboard_offset(uint8_t cellboard_index);