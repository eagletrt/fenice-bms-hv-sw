/**
 * @file    voltage.h
 * @brief   Functions to manage all pack voltages (cells and internal)
 *
 * @date    Apr 11, 2019
 * 
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include "mainboard_config.h"

#include <inttypes.h>

typedef uint16_t voltage_t;

void voltage_init();

/**
 * @brief   Polls SI8900 ADC for voltages
 */
void voltage_measure();

voltage_t *voltage_get_cells();
voltage_t voltage_get_cell_max();
voltage_t voltage_get_cell_min();
voltage_t voltage_get_bus();
voltage_t voltage_get_internal();
