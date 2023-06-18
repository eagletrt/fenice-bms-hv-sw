/**
 * @file		Volv.h
 * @brief		Voltage measurement functions
 *
 * @date		Jul 17, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include "cellboard_config.h"
#include "peripherals/ltc6813_utils.h"

extern voltage_t voltages[CELLBOARD_CELL_COUNT];

void volt_start_measure();
void volt_read();

void volt_start_open_wire_check(uint8_t status);
void volt_read_open_wire(uint8_t status);
void volt_open_wire_check();

/**
 * @brief Returns the lower-voltage cell
 * 
 * @return uint16_t The index of the lower-voltage cell
 */
uint16_t volt_get_min();
/**
 * @brief Get a pointer to the array of voltages values of the cells
 * 
 * @return voltage_t * The pointer to the array of voltages
 */
voltage_t * volt_get_volts();