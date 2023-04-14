/**
 * @file    voltage.c
 * @brief   Functions to manage all pack voltages (cells and internal)
 *
 * @date    Apr 11, 2019
 * 
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "pack/voltage.h"

#include "bms_fsm.h"
#include "error/error.h"
#include "main.h"
#include "peripherals/adc124s021.h"
#include "spi.h"
#include "mainboard_config.h"

#include <string.h>
#include <math.h>

#define CONV_COEFF 140.86f  //(10MOhm + 71.5kOhm) / 71.5kOhm

struct voltage {
    float vts_p;
    float vbat_adc;
    float vbat_sum;
    voltage_t cells[PACK_CELL_COUNT];
};

// This module is a singleton
struct voltage voltage;

/**
 * @brief	Initializes the pack
 *
 */
void voltage_init() {
    voltage.vts_p      = 0;
    voltage.vbat_adc = 0;

    memset(voltage.cells, 0, sizeof(voltage.cells));
}

float _voltage_conv_adc_to_voltage(voltage_t v) {
    return CONV_COEFF * v;
}

void voltage_measure(float voltages[2]) {
    ADC124S021_CH chs[2] = {ADC124_BUS_CHANNEL, ADC124_INTERNAL_CHANNEL};
    voltage.vbat_sum        = 0;

    if (voltages != NULL || adc124s021_read_channels(&SPI_ADC124S, chs, 2, voltages)) {
        voltage.vts_p      = _voltage_conv_adc_to_voltage(voltages[0]);
        voltage.vbat_adc = _voltage_conv_adc_to_voltage(voltages[1]);
    }

    for (uint16_t i = 0; i < PACK_CELL_COUNT; i++) {
        voltage.vbat_sum += voltage.cells[i];
    }
    voltage.vbat_sum /= 10000;

    // Check if difference between readings from the ADC and cellboards is greater than 10V
    error_toggle_check(fabsf(voltage.vbat_adc - voltage.vbat_sum) > 10, ERROR_INT_VOLTAGE_MISMATCH, 0);

    //voltage.internal = MAX(voltages[1], sum);  // TODO: is this a good thing?
}

voltage_t *voltage_get_cells() {
    return voltage.cells;
}

voltage_t voltage_get_cell_max(uint8_t *index) {
    voltage_t max_volt_index = 0;
    for (size_t i = 1; i < PACK_CELL_COUNT; i++) {
        if (voltage.cells[i] > voltage.cells[max_volt_index])
            max_volt_index = i;
    }

    if (index != NULL)
        *index = max_volt_index;
    return voltage.cells[max_volt_index];
}

voltage_t voltage_get_cell_min(uint8_t *index) {
    voltage_t min_volt_index = 0;
    for (size_t i = 1; i < PACK_CELL_COUNT; i++) {
        if (voltage.cells[i] < voltage.cells[min_volt_index] && voltage.cells[i] != 0)
            min_volt_index = i;
    }

    if (index != NULL)
        *index = min_volt_index;
    return voltage.cells[min_volt_index];
}

float voltage_get_vts_p() {
    return voltage.vts_p;
}

float voltage_get_vbat_adc() {
    return voltage.vbat_adc;
}

float voltage_get_vbat_sum() {
    return voltage.vbat_sum;
}

void voltage_set_cells(uint16_t index, voltage_t v1, voltage_t v2, voltage_t v3) {
    voltage.cells[index]     = v1;
    voltage.cells[index + 1] = v2;
    voltage.cells[index + 2] = v3;
}

uint8_t voltage_get_cellboard_offset(uint8_t cellboard_index) {
    uint8_t index = 0;
    while (bms_get_cellboard_distribution()[index] != cellboard_index)
        ++index;
    return index * CELLBOARD_CELL_COUNT;
}

void voltage_check_errors() {
    for(uint8_t i=0; i<PACK_CELL_COUNT; ++i) {
        error_toggle_check(voltage.cells[i] > CELL_MAX_VOLTAGE, ERROR_CELL_OVER_VOLTAGE, 0);
        error_toggle_check(voltage.cells[i] < CELL_MIN_VOLTAGE, ERROR_CELL_UNDER_VOLTAGE, 0);
        error_toggle_check(voltage.cells[i] < CELL_WARN_VOLTAGE, ERROR_CELL_LOW_VOLTAGE, 0);
    }
}