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

voltage_t voltages_pup[CELLBOARD_CELL_COUNT] = {0};
voltage_t voltages_pud[CELLBOARD_CELL_COUNT] = {0};

void volt_start_measure() {
    ltc6813_adcv(&LTC6813_SPI);
}

void volt_read() {
    ltc6813_read_voltages(&LTC6813_SPI, voltages);
}

void volt_start_open_wire_check(uint8_t status) {
    ltc6813_adow(&LTC6813_SPI, status > 2 ? LTC6813_ADOW_PUP_INACTIVE : LTC6813_ADOW_PUP_ACTIVE);
}

void volt_read_open_wire(uint8_t status) {
    ltc6813_read_voltages(&LTC6813_SPI, status > 2 ? voltages_pud : voltages_pup);
}

void volt_open_wire_check() {
    for(uint8_t i = 1; i < CELLBOARD_CELL_COUNT; ++i) {
        int32_t diff = (int32_t)voltages_pup[i] - voltages_pud[i];

        if(diff < -4000) {
            ERROR_SET(ERROR_OPEN_WIRE);
            return;
        }
    }

    if(voltages_pup[0] == 0){
        ERROR_SET(ERROR_OPEN_WIRE);
        return;
    }
    if(voltages_pud[17] == 0){
        ERROR_SET(ERROR_OPEN_WIRE);
        return;
    }

    ERROR_UNSET(ERROR_OPEN_WIRE);
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

voltage_t * volt_get_volts() {
    return voltages;
}
