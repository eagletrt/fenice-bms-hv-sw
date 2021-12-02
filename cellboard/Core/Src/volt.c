/**
 * @file		Volv.h
 * @brief		Voltage measurement functions
 *
 * @date		Jul 17, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "volt.h"

#include "spi.h"

voltage_t voltages[CELLBOARD_CELL_COUNT] = {0};

void volt_start_measure() {
    ltc6813_adcv(&LTC6813_SPI);
}

void volt_read() {
    ltc6813_read_voltages(&LTC6813_SPI, voltages);
}

uint16_t volt_get_min() {
    uint16_t min = 0;
    for (uint16_t i = 0; i < CELLBOARD_CELL_COUNT; i++) {
        if (voltages[i] < voltages[min]) {
            min = i;
        }
    }

    return min;
}