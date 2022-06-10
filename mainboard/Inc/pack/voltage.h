/**
 * @file    voltage.h
 * @brief   Functions to manage all pack voltages (cells and internal)
 *
 * @date    Apr 11, 2019
 * 
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author  Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#pragma once

#include "mainboard_config.h"

#include <inttypes.h>

typedef uint16_t voltage_t;

void voltage_init();

/**
 * @brief   Polls ADC124S021 for voltages
 */
void voltage_measure(float voltages[2]);

voltage_t *voltage_get_cells();
voltage_t voltage_get_cell_max(uint8_t *index);
voltage_t voltage_get_cell_min(uint8_t *index);
float voltage_get_vts_p();
float voltage_get_vbat_adc();
float voltage_get_vbat_sum();
void voltage_set_cells(uint16_t index, voltage_t v1, voltage_t v2, voltage_t v3);
uint8_t voltage_get_cellboard_offset(uint8_t cellboard_index);
void voltage_check_errors();
