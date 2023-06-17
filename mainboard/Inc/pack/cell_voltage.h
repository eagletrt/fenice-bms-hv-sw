/**
 * @file cell_voltage.h
 * @brief Cells voltages management functions
 *
 * @date Apr 11, 2019
 * 
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef CELL_VOLTAGE_H
#define CELL_VOLTAGE_H

#include "stm32f4xx_hal.h"
#include "fenice_config.h"

/** @brief Intialize the cell voltages */
void cell_voltage_init();

/**
 * @brief Get cells voltage values
 * 
 * @return voltage_t * A pointer to the array of cell voltages
 */
voltage_t * cell_voltage_get_cells();
/**
 * @brief Set cells voltage values
 * 
 * @param cellboard_id The index of the cellboard where the values are read from
 * @param offset The cell index offset
 * @param volts An array of voltages to set
 * @param len The length of the array
 * @return HAL_StatusTypeDef HAL_OK if all the values has been copied to the array
 */
HAL_StatusTypeDef cell_voltage_set_cells(size_t cellboard_id,
    size_t offset,
    voltage_t * volts,
    size_t len);
/**
 * @brief Get the maximum voltage value of the pack
 * 
 * @return voltage_t The maximum voltage
 */
voltage_t cell_voltage_get_max();
/**
 * @brief Get the minimum voltage value of the pack
 * 
 * @return voltage_t The minimum voltage
 */
voltage_t cell_voltage_get_min();
/**
 * @brief Get the sum of all the voltage values of the pack
 * 
 * @return float The sum of all cells voltages
 */
float cell_voltage_get_sum();
/**
 * @brief Get the average voltage of the pack
 * 
 * @return float The average voltage
 */
float cell_voltage_get_avg();
/** @brief Check and set voltage related errors */
void cell_voltage_check_errors();

#endif // CELL_VOLTAGE_H