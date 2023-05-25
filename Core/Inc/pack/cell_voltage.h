/**
 * @file cell_voltage.h
 * @brief Functions to manage cells voltages in the pack
 *
 * @date Apr 11, 2019
 * 
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef CELL_VOLTAGE_H
#define CELL_VOLTAGE_H

#include <inttypes.h>
#include <stddef.h>

#include "bms_config.h"
#include "bms-monitor/monitor_config.h"

/** @brief Minimum, maximum and sum of voltage values of a group of cells */
typedef struct {
    voltage_t min;
    voltage_t max;
    size_t sum;

    size_t min_index;
    size_t max_index;
} cell_voltage_t;

/**
 * @brief Read all cells voltages from the pack and copy all the values in the volts array
 * @details The minimum, maximum and sum of groups of cells are saved internally
 * if the values are read correctly
 * 
 * @param volts The array where the values are stored (can be NULL)
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef cell_voltage_measure(voltage_t volts[CELL_COUNT]);

/**
 * @brief Get the minimum voltage value of the pack
 * 
 * @param index A pointer to where the index of the minimum voltage value is written
 * @return voltage_t The minimum voltage value
 */
voltage_t cell_voltage_get_min(size_t * index);
/**
 * @brief Get the maximum voltage value of the pack
 * 
 * @param index A pointer to where the index of the maximum voltage value is written
 * @return voltage_t The maximum voltage value
 */
voltage_t cell_voltage_get_max(size_t * index);
/**
 * @brief Get the sum of all the voltage values of the pack
 * 
 * @return size_t The sum of the voltage values
 */
size_t cell_voltage_get_sum();
/**
 * @brief Get the average of all the voltage values of the pack
 * 
 * @return float The average voltage value
 */
float cell_voltage_get_avg();


#endif // CELL_VOLTAGE_H