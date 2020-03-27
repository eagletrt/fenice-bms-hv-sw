
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

__PD_DEFINE(VOLTAGES_T, voltages)
[PACK_CELL_COUNT];							 /*!< [mV * 10] Cell voltages */
__PD_DEFINE(ADC_VOLTAGE_T, bus_voltage);	 /* [V * 100] Voltage outside the AIRs */
__PD_DEFINE(ADC_VOLTAGE_T, adc_voltage);	 /* [V * 100] Voltage inside the AIRs */
__PD_DEFINE(TOTAL_VOLTAGE_T, total_voltage); /*!< [mV * 10] Total pack voltage */
__PD_DEFINE(VOLTAGES_T, max_voltage);		 /*!< [mV * 10] Maximum cell voltage */
__PD_DEFINE(VOLTAGES_T, min_voltage);		 /*!< [mV * 10] Minimum cell voltage */

__PD_DEFINE(TEMPERATURES_T, temperatures)
[PACK_TEMP_COUNT];							  /*!< [°C] */
__PD_DEFINE(TEMPERATURES_T, avg_temperature); /*!< [°C * 100] Average pack temperature */
__PD_DEFINE(TEMPERATURES_T, max_temperature); /*!< [°C * 100] Maximum temperature */
__PD_DEFINE(TEMPERATURES_T, min_temperature); /*!< [°C * 100] Mimimum temperature */

__PD_DEFINE(CURRENT_T, current); /*!< [A * 10] Instant current draw. */

// Voltage
void set_voltage(uint8_t index, VOLTAGES_T voltage) {
	PD_voltages[index].data = voltage;
	PD_voltages[index].timestamp = HAL_GetTick();
}
VOLTAGES_T get_voltage(uint8_t index) {
	return (VOLTAGES_T)PD_voltages[index].data;
}

// Bus Voltage
void set_bus_voltage(ADC_VOLTAGE_T bus_voltage) {
	PD_bus_voltage.data = bus_voltage;
	PD_bus_voltage.timestamp = HAL_GetTick();
}
ADC_VOLTAGE_T get_bus_voltage() {
	return (ADC_VOLTAGE_T)PD_bus_voltage.data;
}

// ADC Voltage
void set_adc_voltage(ADC_VOLTAGE_T adc_voltage) {
	PD_adc_voltage.data = adc_voltage;
	PD_adc_voltage.timestamp = HAL_GetTick();
}
ADC_VOLTAGE_T get_adc_voltage() {
	return (ADC_VOLTAGE_T)PD_adc_voltage.data;
}

// Total Voltage
void set_total_voltage(TOTAL_VOLTAGE_T total_voltage) {
	PD_total_voltage.data = total_voltage;
	PD_total_voltage.timestamp = HAL_GetTick();
}
TOTAL_VOLTAGE_T get_total_voltage() {
	return (TOTAL_VOLTAGE_T)PD_total_voltage.data;
}

// Max Voltage
void set_max_voltage(VOLTAGES_T voltage) {
	PD_max_voltage.data = voltage;
	PD_max_voltage.timestamp = HAL_GetTick();
}
VOLTAGES_T get_max_voltage() {
	return (VOLTAGES_T)PD_max_voltage.data;
}

// Min Voltage
void set_min_voltage(VOLTAGES_T voltage) {
	PD_min_voltage.data = voltage;
	PD_min_voltage.timestamp = HAL_GetTick();
}
VOLTAGES_T get_min_voltage() {
	return (VOLTAGES_T)PD_min_voltage.data;
}

// Temperature
void set_temperature(uint8_t index, TEMPERATURES_T temperature) {
	PD_temperatures[index].data = temperature;
	PD_temperatures[index].timestamp = HAL_GetTick();
}
TOTAL_VOLTAGE_T get_temperature(uint8_t index) {
	return (TEMPERATURES_T)PD_temperatures[index].data;
}

// Avg Temperature
void set_avg_temperature(TEMPERATURES_T temperature) {
	PD_avg_temperature.data = temperature;
	PD_avg_temperature.timestamp = HAL_GetTick();
}
TOTAL_VOLTAGE_T get_avg_temperature() {
	return (TEMPERATURES_T)PD_avg_temperature.data;
}

// Max Temperature
void set_max_temperature(TEMPERATURES_T temperature) {
	PD_max_temperature.data = temperature;
	PD_max_temperature.timestamp = HAL_GetTick();
}
TOTAL_VOLTAGE_T get_max_temperature() {
	return (TEMPERATURES_T)PD_max_temperature.data;
}

// Min Temperature
void set_min_temperature(TEMPERATURES_T temperature) {
	PD_min_temperature.data = temperature;
	PD_min_temperature.timestamp = HAL_GetTick();
}
TOTAL_VOLTAGE_T get_min_temperature() {
	return (TEMPERATURES_T)PD_min_temperature.data;
}

// Current
void set_current(TEMPERATURES_T temperature) {
	PD_current.data = temperature;
	PD_current.timestamp = HAL_GetTick();
}
TOTAL_VOLTAGE_T get_current() {
	return (TEMPERATURES_T)PD_current.data;
}
