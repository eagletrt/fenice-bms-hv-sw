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
    _ltc6813_adcv(&LTC6813_SPI, 0);
}

void volt_read() {
    ltc6813_read_voltages(&LTC6813_SPI, voltages);
}
