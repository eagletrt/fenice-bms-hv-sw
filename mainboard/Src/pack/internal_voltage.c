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

#include "mainboard_config.h"
#include "main.h"
#include "cell_voltage.h"

/** @brief Internal voltages of the mainboard */
struct internal_voltage {
    uint16_t tsp;   // TS+ voltage
    uint16_t bat;   // Battery voltage
    uint16_t shunt; // Shunter voltage
    uint16_t tsn;   // TS- voltage
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
    uint16_t volts[MAX22530_CHANNEL_COUNT] = { 0 };
    HAL_StatusTypeDef status = max22530_read_all_channels(&internal_adc, volts);

    if (status != HAL_OK)
        return status;
    
    internal_voltages.tsp   = volts[MAX22530_VTS_CHANNEL - 1];
    internal_voltages.tsn   = volts[MAX22530_TSN_CHANNEL - 1];
    internal_voltages.shunt = volts[MAX22530_SHUNT_CHANNEL - 1];
    internal_voltages.bat   = volts[MAX22530_VBATT_CHANNEL - 1];

    // Check if difference between readings from the ADC and cellboards is greater than 10V
    error_toggle_check(fabsf(CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltages.bat) - CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_sum())) > INTERNAL_VOLTAGE_MAX_DELTA, ERROR_INT_VOLTAGE_MISMATCH, 0);
    return HAL_OK;
}

uint16_t internal_voltage_get_tsp() {
    return internal_voltages.tsp;
}
uint16_t internal_voltage_get_tsn() {
    return internal_voltages.tsn;
}
uint16_t internal_voltage_get_shunt() {
    return internal_voltages.shunt;
}
uint16_t internal_voltage_get_bat() {
    return internal_voltages.bat;
}

bool internal_voltage_is_precharge_complete() {
    float tsp = CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltages.tsp);
    float target;
    if (is_handcart_connected)
        // target = CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltages.bat) * INTERNAL_VOLTAGE_PRECHARGE_HANDCART_THRESHOLD;
        target = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_sum()) * INTERNAL_VOLTAGE_PRECHARGE_HANDCART_THRESHOLD;
    else
        // target = CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltages.bat) * INTERNAL_VOLTAGE_PRECHARGE_THRESHOLD;
        target = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_sum()) * INTERNAL_VOLTAGE_PRECHARGE_THRESHOLD;
    
    return tsp >= target;
}