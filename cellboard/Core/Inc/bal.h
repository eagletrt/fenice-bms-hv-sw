/**
 * @file bal.h
 * @brief This file contains the balancing functions
 *
 * @date Oct 28, 2019
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Simone Ruffini [simone.ruffini@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef BAL_H
#define BAL_H

#include "bms/bms_network.h"
#include "cellboard_config.h"

#include <inttypes.h>
#include <stdbool.h>

#define BAL_NULL_INDEX UINT8_MAX

/**
 * @brief   Wrapper function around bal_compute_imbalance and bal_exclude_neighbors
 * 
 * @param	volts		Array of cell voltages
 * @param	cells		array of cell's indexes that need to be discharged
 * @param	target		The target voltage to reach
 * @param	threshold	Balancing tolerance (voltage + threshold)
 * 
 * @returns amount of cells to be discharged
 */
uint16_t bal_get_cells_to_discharge(
    voltage_t volts[CELLBOARD_CELL_COUNT],
    bms_balancing_converted_t * cells,
    voltage_t target,
    voltage_t threshold);

/**
 * @brief	Computes the cells with voltage exceeding the given threshold + lowest cell voltage
 * 
 * @param	volts		Array of cell voltages
 * @param	count		Size of `volts`
 * @param	threshold	Balancing tolerance (voltage + threshold)
 * @param	indexes		Output array of indexes to be balanced
 * @param	cells		array of cell's indexes that need to be discharged
 * 
 * @returns amount of cells to be discharged
 */
uint16_t bal_compute_imbalance(voltage_t volts[], uint16_t count, voltage_t threshold, uint16_t cells[]);
uint16_t bal_compute_imbalance_with_target(
    voltage_t volts[],
    uint16_t count,
    voltage_t threshold,
    uint16_t cells[],
    voltage_t target);

/**
 * @brief	Exclude adjacent cells to work around an hardware limitation on Fenice
 * 
 * @param	indexes		output array of cell's indexes that need to be discharged
 * @param	count		amount of voltages
 * @param	cells		output cell's indexes that need to be discharged
 * 
 * @returns	amount of cells to be discharged
 */
uint16_t bal_exclude_neighbors(uint16_t indexes[], uint16_t count, bms_balancing_converted_t cells);
#endif