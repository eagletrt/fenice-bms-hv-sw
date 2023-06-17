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

voltage_t cells[PACK_CELL_COUNT];


voltage_t * cell_voltage_get_cells() {
    return cells;
}
HAL_StatusTypeDef cell_voltage_set_cells(size_t cellboard_id,
    size_t offset,
    voltage_t * volts,
    size_t len) {
    if (volts == NULL || len > CELLBOARD_CELL_COUNT || offset + len > CELLBOARD_CELL_COUNT)
        return HAL_ERROR;

    // Get cellboard index in the distribution
    size_t index = 0;
    uint8_t * distr = bms_get_cellboard_distribution();
    for (; index < CELLBOARD_COUNT && distr[index] != cellboard_id; index++)
        ;
    // Get index in the cells array
    index *= CELLBOARD_CELL_COUNT;

    for (size_t i = 0; i < len; i++)
        cells[index + offset + i] = volts[i];
    return HAL_OK;
}

voltage_t cell_voltage_get_max() {
    voltage_t max = 0;
    for (size_t i = 0; i < PACK_CELL_COUNT; i++)
        max = MAX(max, cells[i]);
    return max;
}
voltage_t cell_voltage_get_min() {
    voltage_t min = UINT16_MAX;
    for (size_t i = 0; i < PACK_CELL_COUNT; i++)
        min = MIN(min, cells[i]);
    return min;
}
float cell_voltage_get_sum() {
    float sum = 0;
    for (size_t i = 0; i < PACK_CELL_COUNT; i++)
        sum += cells[i];
    return sum;
}
float cell_voltage_get_avg() {
    return cell_voltage_get_sum() / PACK_CELL_COUNT;
}

void cell_voltage_check_errors() {
    voltage_t min = cell_voltage_get_min();
    voltage_t max = cell_voltage_get_max();
    error_toggle_check(max > CELL_MAX_VOLTAGE, ERROR_CELL_OVER_VOLTAGE, 0);
    error_toggle_check(min < CELL_MIN_VOLTAGE, ERROR_CELL_UNDER_VOLTAGE, 0);
    error_toggle_check(min < CELL_WARN_VOLTAGE, ERROR_CELL_LOW_VOLTAGE, 0);
}