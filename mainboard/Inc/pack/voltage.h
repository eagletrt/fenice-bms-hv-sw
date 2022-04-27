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

typedef enum { VOLTAGE_CELLBOARD_0_OFFSET = 0, 
    VOLTAGE_CELLBOARD_1_OFFSET = LTC6813_CELL_COUNT*1, 
    VOLTAGE_CELLBOARD_2_OFFSET = LTC6813_CELL_COUNT*2, 
    VOLTAGE_CELLBOARD_3_OFFSET = LTC6813_CELL_COUNT*3, 
    VOLTAGE_CELLBOARD_4_OFFSET = LTC6813_CELL_COUNT*4, 
    VOLTAGE_CELLBOARD_5_OFFSET = LTC6813_CELL_COUNT*5} VOLTAGE_CELLBOARD_START_INDEX;

void voltage_init();

/**
 * @brief   Polls ADC124S021 for voltages
 */
void voltage_measure(voltage_t voltages[2]);

voltage_t *voltage_get_cells();
voltage_t voltage_get_cell_max();
voltage_t voltage_get_cell_min();
voltage_t voltage_get_bus();
voltage_t voltage_get_internal();
void voltage_set_cells(uint16_t index, voltage_t v1, voltage_t v2, voltage_t v3);
