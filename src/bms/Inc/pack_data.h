/**
 * @file		pack_data.h
 * @brief		
 *
 * @date		Mar 9, 2020
 * 
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef PACK_DATA_H
#define PACK_DATA_H

#include <inttypes.h>
#include <stdbool.h>

#include "../../fenice_config.h"

//_________________________________________Private Macros_________________________________________
#define __PD_DEFINE(_data_type_, _data_name_) \
	struct {                                  \
		_data_type_ data;                     \
		uint32_t timestamp;                   \
	} PD_##_data_name_

#define TOTAL_VOLTAGE_T uint32_t
#define ADC_VOLTAGE_T uint16_t
#define VOLTAGES_T uint16_t
#define TEMPERATURES_T uint8_t

#define CURRENT_T int16_t

void set_voltage(uint8_t index, VOLTAGES_T voltage);
VOLTAGES_T get_voltage(uint8_t index);

void set_bus_voltage(ADC_VOLTAGE_T bus_voltage);
ADC_VOLTAGE_T get_bus_voltage();

void set_adc_voltage(ADC_VOLTAGE_T adc_voltage);
ADC_VOLTAGE_T get_adc_voltage();

void set_total_voltage(TOTAL_VOLTAGE_T total_voltage);
TOTAL_VOLTAGE_T get_total_voltage();

void set_max_voltage(VOLTAGES_T voltage);
VOLTAGES_T get_max_voltage();

void set_min_voltage(VOLTAGES_T voltage);
VOLTAGES_T get_min_voltage();

void set_temperature(uint8_t index, TEMPERATURES_T temperature);
TOTAL_VOLTAGE_T get_temperature(uint8_t index);

void set_avg_temperature(TEMPERATURES_T temperature);
TOTAL_VOLTAGE_T get_avg_temperature();

void set_max_temperature(TEMPERATURES_T temperature);
TOTAL_VOLTAGE_T get_max_temperature();

void set_min_temperature(TEMPERATURES_T temperature);
TOTAL_VOLTAGE_T get_min_temperature();

void set_current(TEMPERATURES_T temperature);
TOTAL_VOLTAGE_T get_current();

#endif