/**
 * @file    voltage.c
 * @brief   Functions to manage all pack voltages (cells and internal)
 *
 * @date    Apr 11, 2019
 * 
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "pack/voltage.h"

#include "error/error.h"
#include "main.h"
#include "peripherals/si8900.h"
#include "usart.h"

struct voltage {
    voltage_t bus;
    voltage_t internal;
    voltage_t cells[PACK_CELL_COUNT];
};

// This module is a singleton
struct voltage voltage;

/**
 * @brief	Initializes the pack
 *
 */
void voltage_init() {
    voltage.bus      = 0;
    voltage.internal = 0;

    for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
        voltage.cells[i] = 0;
    }
}

void voltage_measure() {
    voltage_t internal = 0;
    voltage_t bus      = 0;
    voltage_t sum      = 0;

    if (si8900_read_channel(&SI8900_UART, SI8900_AIN0, &internal)) {
        voltage.internal = internal;
    }
    HAL_Delay(2);  // TODO: this sucks
    if (si8900_read_channel(&SI8900_UART, SI8900_AIN1, &bus)) {
        voltage.bus = bus;
    }

    for (uint16_t i = 0; i < PACK_CELL_COUNT; i++) {
        sum += voltage.cells[i];
    }

    // Check if difference between readings from the ADC and cellboards is greater than 10V
    if (MAX(internal, sum) - MIN(internal, sum) > 10 * 100) {
        error_set(ERROR_INT_VOLTAGE_MISMATCH, 0, HAL_GetTick());
    } else {
        error_reset(ERROR_INT_VOLTAGE_MISMATCH, 0);
    }
    voltage.internal = MAX(internal, sum);  // TODO: is this a good thing?
}

voltage_t *voltage_get_cells() {
    return voltage.cells;
}

voltage_t voltage_get_cell_max() {
    voltage_t max_volt = 0;
    for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
        max_volt = MAX(max_volt, voltage.cells[i]);
    }
    return max_volt;
}

voltage_t voltage_get_cell_min() {
    voltage_t min_volt = UINT16_MAX;
    for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
        min_volt = MIN(min_volt, voltage.cells[i]);
    }
    return min_volt;
}

voltage_t voltage_get_bus() {
    return voltage.bus;
}

voltage_t voltage_get_internal() {
    return voltage.internal;
}

void voltage_set_cells(uint16_t index, voltage_t v1, voltage_t v2, voltage_t v3) {
    voltage.cells[index] = v1;
    voltage.cells[index+1] = v2;
    voltage.cells[index+2] = v3;
}
