/**
 * @file internal_voltage.c
 * @brief Internal voltages management functions
 *
 * @date Apr 11, 2019
 * 
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "pack/internal_voltage.h"

#include <math.h>

#include "max22530.h"
#include "mainboard_config.h"
#include "main.h"
#include "cell_voltage.h"

/** @brief Internal voltages of the mainboard */
struct internal_voltage {
    voltage_t tsp;   // TS+ voltage
    voltage_t bat;   // Battery voltage
    voltage_t shunt; // Shunter voltage
    voltage_t tsn;   // TS- voltage
};

// This module is a singleton
struct internal_voltage internal_voltages;
MAX22530_HandleTypeDef internal_adc;


void internal_voltage_init() {
    // Init adc
    max22530_init(&internal_adc, &SPI_ADC, ADC_CS_GPIO_Port, ADC_CS_Pin);

    // Init voltages
    internal_voltages.tsp   = 0;
    internal_voltages.tsn   = 0;
    internal_voltages.shunt = 0;
    internal_voltages.bat   = 0;
}

HAL_StatusTypeDef internal_voltage_measure() {
    // Read voltages from the adc
    float volts[MAX22530_CHANNEL_COUNT] = { 0 };
    HAL_StatusTypeDef status = max22530_read_all_channels(&internal_adc, volts);

    if (status != HAL_OK)
        return status;
    
    internal_voltages.tsp   = MAX22530_CONV_VALUE_TO_VOLTAGE(volts[MAX22530_VTS_CHANNEL]);
    internal_voltages.tsn   = MAX22530_CONV_VALUE_TO_VOLTAGE(volts[MAX22530_TSN_CHANNEL]);
    internal_voltages.shunt = MAX22530_CONV_VALUE_TO_VOLTAGE(volts[MAX22530_SHUNT_CHANNEL]);
    internal_voltages.bat   = MAX22530_CONV_VALUE_TO_VOLTAGE(volts[MAX22530_VBATT_CHANNEL]);

    // Check if difference between readings from the ADC and cellboards is greater than 10V
    error_toggle_check(fabsf(internal_voltages.bat - cell_voltage_get_sum()) > 10000, ERROR_INT_VOLTAGE_MISMATCH, 0);
    return HAL_OK;
}

voltage_t internal_voltage_get_tsp() {
    return internal_voltages.tsp;
}
voltage_t internal_voltage_get_tsn() {
    return internal_voltages.tsn;
}
voltage_t internal_voltage_get_shunt() {
    return internal_voltages.shunt;
}
voltage_t internal_voltage_get_bat() {
    return internal_voltages.bat;
}