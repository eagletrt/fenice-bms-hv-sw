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

#include <inttypes.h>
#include "mainboard_config.h"

#define CONVERT_VALUE_TO_TEMPERATURE(x) ((float)(x) / 2.56 - 20)
#define CONVERT_TEMPERATURE_TO_VALUE(x) (((x) + 20) * 2.56f)

typedef uint8_t temperature_t;

/** @brief Minimum, maximum and average temperatures of the cells of the pack */
typedef struct {
    temperature_t min[CELLBOARD_COUNT];
    temperature_t max[CELLBOARD_COUNT];
    float avg[CELLBOARD_COUNT];
} cell_temperature;

extern cell_temperature cell_temps;

/** @brief Initialize cell temperatures */
void temperature_init();

/** @brief Check temperature errors */
void temperature_check_errors();

/**
 * @brief Get the maximum temperature value of the pack
 * 
 * @return temperature_t The maximum temperature value
 */
temperature_t temperature_get_max();
/**
 * @brief Get the minimum temperature value of the pack
 * 
 * @return temperature_t The minimum temperature value
 */
temperature_t temperature_get_min();
/**
 * @brief Get the sum of all the temperatures of the pack
 * 
 * @return float The sum of the temperatures
 */
float temperature_get_sum();
/**
 * @brief Get the average temperature value of the pack
 * 
 * @return float The average temperature value
 */
float temperature_get_average();
/**
 * @brief Set cells temperature values
 * 
 * @param cellboard_id The cellboard index where the values are read from
 * @param temps An array of temperatures to set
 * @param len The length of the array
 */
HAL_StatusTypeDef temperature_set_cells(size_t cellboard_id,
    temperature_t min,
    temperature_t max,
    float avg);

#endif // TEMPERATURE_H
