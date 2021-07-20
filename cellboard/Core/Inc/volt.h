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