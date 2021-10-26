/**
 * @file    temperature.h
 * @brief   Functions to manage all cell temperatures
 *
 * @date    Apr 11, 2019
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <inttypes.h>

typedef uint8_t temperature_t;

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
