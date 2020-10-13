/**
 * @file		pack_data.c
 * @brief		
 *
 * @date		Mar 27,2020
 * 
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include "pack_data.h"

#include <stm32g4xx_hal.h>

#include "../../fenice_config.h"

__PD_DEFINE(VOLTAGE_T, voltages)
[PACK_CELL_COUNT];							  /*!< [mV * 10] Cell voltages */
__PD_DEFINE(ADC_VOLTAGE_T, bus_voltage);	  /* [V * 100] Voltage outside the AIRs */
__PD_DEFINE(ADC_VOLTAGE_T, internal_voltage); /* [V * 100] Voltage inside the AIRs (pack voltage) */
__PD_DEFINE(TOTAL_VOLTAGE_T, total_voltage);  /*!< [mV * 10] Total pack voltage (sum of all cells) */
__PD_DEFINE(VOLTAGE_T, max_voltage);		  /*!< [mV * 10] Maximum cell voltage */
__PD_DEFINE(VOLTAGE_T, min_voltage);		  /*!< [mV * 10] Minimum cell voltage */

__PD_DEFINE(TEMPERATURE_T, temperatures)
[PACK_TEMP_COUNT];							 /*!< [°C] */
__PD_DEFINE(TEMPERATURE_T, avg_temperature); /*!< [°C * 100] Average pack temperature */
__PD_DEFINE(TEMPERATURE_T, max_temperature); /*!< [°C * 100] Maximum temperature */
__PD_DEFINE(TEMPERATURE_T, min_temperature); /*!< [°C * 100] Mimimum temperature */

__PD_DEFINE(CURRENT_T, current); /*!< [A * 10] Instant current draw. */

FEEDBACK_T pd_feedback;

// Voltage
VOLTAGE_T
pd_set_voltage(uint8_t index, VOLTAGE_T voltage) {
	PD_voltages[index].data = voltage;
	PD_voltages[index].timestamp = HAL_GetTick();
	return voltage;
}
VOLTAGE_T pd_get_voltage(uint8_t index) {
	return (VOLTAGE_T)PD_voltages[index].data;
}
void pd_get_voltage_array(VOLTAGE_T *const array) {
	for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
		array[i] = PD_voltages[i].data;
	}
}

// Bus Voltage
ADC_VOLTAGE_T pd_set_bus_voltage(ADC_VOLTAGE_T bus_voltage) {
	PD_bus_voltage.data = bus_voltage;
	PD_bus_voltage.timestamp = HAL_GetTick();
	return bus_voltage;
}
ADC_VOLTAGE_T pd_get_bus_voltage() {
	return (ADC_VOLTAGE_T)PD_bus_voltage.data;
}

// Internal Voltage
ADC_VOLTAGE_T pd_set_internal_voltage(ADC_VOLTAGE_T internal_voltage) {
	PD_internal_voltage.data = internal_voltage;
	PD_internal_voltage.timestamp = HAL_GetTick();
	return internal_voltage;
}
ADC_VOLTAGE_T pd_get_internal_voltage() {
	return (ADC_VOLTAGE_T)PD_internal_voltage.data;
}

// Total Voltage
TOTAL_VOLTAGE_T pd_set_total_voltage(TOTAL_VOLTAGE_T total_voltage) {
	PD_total_voltage.data = total_voltage;
	PD_total_voltage.timestamp = HAL_GetTick();
	return total_voltage;
}
TOTAL_VOLTAGE_T pd_get_total_voltage() {
	return (TOTAL_VOLTAGE_T)PD_total_voltage.data;
}

// Max Voltage
VOLTAGE_T pd_set_max_voltage(VOLTAGE_T voltage) {
	PD_max_voltage.data = voltage;
	PD_max_voltage.timestamp = HAL_GetTick();
	return voltage;
}
VOLTAGE_T pd_get_max_voltage() {
	return (VOLTAGE_T)PD_max_voltage.data;
}

// Min Voltage
VOLTAGE_T pd_set_min_voltage(VOLTAGE_T voltage) {
	PD_min_voltage.data = voltage;
	PD_min_voltage.timestamp = HAL_GetTick();
	return voltage;
}
VOLTAGE_T pd_get_min_voltage() {
	return (VOLTAGE_T)PD_min_voltage.data;
}

// Temperature
TEMPERATURE_T pd_set_temperature(uint8_t index, TEMPERATURE_T temperature) {
	PD_temperatures[index].data = temperature;
	PD_temperatures[index].timestamp = HAL_GetTick();
	return temperature;
}
TEMPERATURE_T pd_get_temperature(uint8_t index) {
	return (TEMPERATURE_T)PD_temperatures[index].data;
}

// Avg Temperature
TEMPERATURE_T pd_set_avg_temperature(TEMPERATURE_T temperature) {
	PD_avg_temperature.data = temperature;
	PD_avg_temperature.timestamp = HAL_GetTick();
	return temperature;
}
TEMPERATURE_T pd_get_avg_temperature() {
	return (TEMPERATURE_T)PD_avg_temperature.data;
}

// Max Temperature
TEMPERATURE_T pd_set_max_temperature(TEMPERATURE_T temperature) {
	PD_max_temperature.data = temperature;
	PD_max_temperature.timestamp = HAL_GetTick();
	return temperature;
}
TEMPERATURE_T pd_get_max_temperature() {
	return (TEMPERATURE_T)PD_max_temperature.data;
}

// Min Temperature
TEMPERATURE_T pd_set_min_temperature(TEMPERATURE_T temperature) {
	PD_min_temperature.data = temperature;
	PD_min_temperature.timestamp = HAL_GetTick();
	return temperature;
}
TEMPERATURE_T pd_get_min_temperature() {
	return (TEMPERATURE_T)PD_min_temperature.data;
}

// Current
CURRENT_T pd_set_current(CURRENT_T current) {
	PD_current.data = current;
	PD_current.timestamp = HAL_GetTick();
	return current;
}
CURRENT_T pd_get_current() {
	return (CURRENT_T)PD_current.data;
}
