/**
 * @file temperature.h
 * @brief Functions to manage all cell temperatures
 *
 * @date Apr 11, 2019
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <stddef.h>

#include "bms_config.h"
#include "pack/temp.h"


/** @brief Minimum, maximum and sum of temperature values of a group of cells */
typedef struct {
    temperature_t min;
    temperature_t max;
    size_t sum;
} cell_temperature_t;


/**
 * @brief Read all cells temperaratures from the pack and copy all the values in the temp array
 * @details The minimum, maximum and sum of groups of cells are saved internally
 * if the values are read correctly
 * 
 * @param temps The array where the values are stored (can be NULL)
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef cell_temperature_measure(temperature_t temps[PACK_TEMP_COUNT]);

/**
 * @brief Get the minimum temperature value of the pack
 * 
 * @return temperature_t The minimum temperature
 */
temperature_t temperature_get_min();
/**
 * @brief Get the maximum temperature value of the pack
 * 
 * @return temperature_t The maximum temperature value
 */
temperature_t temperature_get_max();
/**
 * @brief Get the sum of all the temperature values of the pack
 * 
 * @return size_t The sum of all the temperature values
 */
size_t temperature_get_sum();
/**
 * @brief Get the average of all the temperature values of the pack
 * 
 * @return float The average temperature value
 */
float temperature_get_avg();


#endif // TEMPERATURE_H
