/**
 * @file cell_voltage.c
 * @brief Cells voltages management functions
 *
 * @date Apr 11, 2019
 * 
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "pack/cell_voltage.h"

#include <limits.h>
#include <string.h>

#include "max22530.h"
#include "mainboard_config.h"
#include "main.h"
#include "bms_fsm.h"

static cell_voltage cell_volts;

void cell_voltage_init() {
    memset(cell_volts.min, 0, CELLBOARD_COUNT * sizeof(voltage_t));
    memset(cell_volts.max, 0, CELLBOARD_COUNT * sizeof(voltage_t));
    memset(cell_volts.avg, 0, CELLBOARD_COUNT * sizeof(float));
}
HAL_StatusTypeDef cell_voltage_set_cells(size_t cellboard_id,
    voltage_t * volts,
    size_t len) {
    if (volts == NULL || len > CELLBOARD_CELL_COUNT || cellboard_id > CELLBOARD_COUNT)
        return HAL_ERROR;

    // Update min, max and average voltages
    const float perc = 1.f / (CELLBOARD_CELL_COUNT - len);
    float sum = 0;
    for (size_t i = 0; i < len; i++) {
        cell_volts.max[cellboard_id] = MAX(cell_volts.max[cellboard_id], volts[i]);
        cell_volts.min[cellboard_id] = MIN(cell_volts.min[cellboard_id], volts[i]);
        sum += volts[i];
    }
    cell_volts.avg[cellboard_id] = cell_volts.avg[cellboard_id] * (1 - perc) + sum * perc;
    
    return HAL_OK;
}

voltage_t cell_voltage_get_max() {
    voltage_t max = 0;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        max = MAX(max, cell_volts.max[i]);
    return max;
}
voltage_t cell_voltage_get_min() {
    voltage_t min = UINT16_MAX;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        min = MIN(min, cell_volts.min[i]);
    return min;
}
float cell_voltage_get_sum() {
    cell_voltage_get_avg() * PACK_CELL_COUNT;
}
float cell_voltage_get_avg() {
    float avg = 0;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        avg += cell_volts.avg[i];
    return avg / CELLBOARD_COUNT;
}

void cell_voltage_check_errors() {
    voltage_t min = cell_voltage_get_min();
    voltage_t max = cell_voltage_get_max();
    error_toggle_check(max > CELL_MAX_VOLTAGE, ERROR_CELL_OVER_VOLTAGE, 0);
    error_toggle_check(min < CELL_MIN_VOLTAGE, ERROR_CELL_UNDER_VOLTAGE, 0);
    error_toggle_check(min < CELL_WARN_VOLTAGE, ERROR_CELL_LOW_VOLTAGE, 0);
}