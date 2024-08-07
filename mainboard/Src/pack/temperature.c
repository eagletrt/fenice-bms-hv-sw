/**
 * @file temperature.c
 * @brief Functions to manage all cell temperatures
 *
 * @date Apr 11, 2019
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "pack/temperature.h"

#include <inttypes.h>
#include <math.h>
#include <string.h>

#include "bms_fsm.h"
#include "error_simple.h"
#include "mainboard_config.h"

cell_temperature cell_temps;

void temperature_init() {
    memset(cell_temps.min, CONVERT_TEMPERATURE_TO_VALUE(CELL_MAX_TEMPERATURE), CELLBOARD_COUNT * sizeof(temperature_t));
    memset(cell_temps.max, 0, CELLBOARD_COUNT * sizeof(temperature_t));
    memset(cell_temps.avg, 0, CELLBOARD_COUNT * sizeof(float));
}
void temperature_check_errors() {
    float max_temp = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_max());

    if (max_temp > CELL_MAX_TEMPERATURE) {
        error_simple_set(ERROR_GROUP_ERROR_CELL_OVER_TEMPERATURE, 0);
    } else {
        error_simple_reset(ERROR_GROUP_ERROR_CELL_OVER_TEMPERATURE, 0);
    }

    // Temperature sensors disconnected
#if !defined(TEMP_GROUP_ERROR_ENABLE) && defined(TEMP_ERROR_ENABLE)
    float min_temp = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_min());
    
    error_toggle_check(min_temp <= CELL_MIN_TEMPERATURE + 0.01, ERROR_CONNECTOR_DISCONNECTED, 2);
#endif // TEMP_GROUP_ERROR_ENABLE
}
temperature_t temperature_get_max() {
    temperature_t max = 0;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        max = MAX(max, cell_temps.max[i]);
    return max;
}
temperature_t temperature_get_min() {
    temperature_t min = CONVERT_TEMPERATURE_TO_VALUE(CELL_MAX_TEMPERATURE);
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        min = MIN(min, cell_temps.min[i]);
    return min;
}
float temperature_get_sum() {
    return temperature_get_average() * PACK_TEMP_COUNT;
}
float temperature_get_average() {
    float avg = 0;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        avg += cell_temps.avg[i];
    return avg / CELLBOARD_COUNT;
}

HAL_StatusTypeDef temperature_set_cells(size_t cellboard_id,
    temperature_t min,
    temperature_t max,
    float avg) {

    cell_temps.min[cellboard_id] = min;
    cell_temps.max[cellboard_id] = max;
    cell_temps.avg[cellboard_id] = avg;

    return HAL_OK;
}
