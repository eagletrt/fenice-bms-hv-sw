/**
 * @file		bal.h
 * @brief		This file contains the balancing functions
 *
 * @date		Oct 28, 2019
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef BAL_H
#define BAL_H

#include "mainboard_config.h"
#include "pack/voltage.h"

#include <inttypes.h>
#include <stdbool.h>

#define BAL_NULL_INDEX UINT8_MAX

/**
 * @brief   Wrapper function around bal_compute_imbalance and bal_exclude_neighbors
 * 
 * @param	volts		Array of cell voltages
 * @param	count		Size of `volts`
 * @param	threshold	Balancing tolerance (voltage + threshold)
 * @param	indexes		Output array of indexes to be balanced
 * @param	cells		array of cell's indexes that need to be discharged
 * 
 * @returns amount of cells to be discharged
 */
uint16_t bal_get_cells_to_discharge(voltage_t volts[], uint16_t count, voltage_t threshold, uint16_t cells[]);

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

/**
 * @brief	Exclude adjacent cells to work around an hardware limitation on Fenice
 * 
 * @param	indexes		output array of cell's indexes that need to be discharged
 * @param	count		amount of voltages
 * @param	cells		output array of cell's indexes that need to be discharged
 * 
 * @returns	amount of cells to be discharged
 */
uint16_t bal_exclude_neighbors(uint16_t indexes[], uint16_t count, uint16_t cells[]);
#endif