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

#define TOTAL_VOLTAGE_T uint32_t
#define ADC_VOLTAGE_T uint16_t
#define VOLTAGE_T uint16_t
#define TEMPERATURE_T uint8_t
#define CURRENT_T int16_t

#define FEEDBACK_T uint16_t

// Feedbacks to check after TS OFF
#define FEEDBACK_TS_OFF_VAL FEEDBACK_VREF
#define FEEDBACK_TS_OFF_MASK FEEDBACK_VREF | FEEDBACK_AIR_POSITIVE | FEEDBACK_AIR_NEGATIVE | FEEDBACK_PC_END | FEEDBACK_TS_ON

// Feedback states before closing AIR- and trigger PC
#define FEEDBACK_IDLE_TS_ON_TRIGGER_VAL /**/ \
	(FEEDBACK_VREF |                         \
	 FEEDBACK_FROM_TSMS |                    \
	 FEEDBACK_TO_TSMS |                      \
	 FEEDBACK_FROM_SHUTDOWN |                \
	 FEEDBACK_LATCH_IMD |                    \
	 FEEDBACK_LATCH_BMS |                    \
	 FEEDBACK_IMD_FAULT |                    \
	 FEEDBACK_BMS_FAULT)
#define FEEDBACK_IDLE_TS_ON_TRIGGER_MASK FEEDBACK_ALL

// Feedback states after closing AIR-
#define FEEDBACK_TO_PRECHARGE_VAL /**/ \
	(FEEDBACK_IDLE_TS_ON_TRIGGER_VAL | \
	 FEEDBACK_AIR_NEGATIVE |           \
	 FEEDBACK_RELAY_LV |               \
	 FEEDBACK_IMD_SHUTDOWN |           \
	 FEEDBACK_BMS_FAULT |              \
	 FEEDBACK_BMS_SHUTDOWN |           \
	 FEEDBACK_TS_ON)
#define FEEDBACK_TO_PRECHARGE_MASK FEEDBACK_ALL &(~FEEDBACK_TSAL_HV)

#define FEEDBACK_ON_VAL FEEDBACK_TO_PRECHARGE_VAL | FEEDBACK_TSAL_HV
#define FEEDBACK_ON_MASK FEEDBACK_ALL

//_________________________________________Private Macros_________________________________________
#define __PD_DEFINE(_data_type_, _data_name_) \
	struct PD_##_data_name_##_s {             \
		_data_type_ data;                     \
		uint32_t timestamp;                   \
	} PD_##_data_name_

extern FEEDBACK_T pd_feedback;

VOLTAGE_T pd_set_voltage(uint8_t index, VOLTAGE_T voltage);
VOLTAGE_T pd_get_voltage(uint8_t index);

/**
 * @brief copy all values of PD_voltage in the array 
 * 
 * @param array reference to a local array: the array values will be changed
 */
void pd_get_voltage_array(VOLTAGE_T *const array);

ADC_VOLTAGE_T pd_set_bus_voltage(ADC_VOLTAGE_T bus_voltage);
ADC_VOLTAGE_T pd_get_bus_voltage();

ADC_VOLTAGE_T pd_set_adc_voltage(ADC_VOLTAGE_T adc_voltage);
ADC_VOLTAGE_T pd_get_adc_voltage();

TOTAL_VOLTAGE_T pd_set_total_voltage(TOTAL_VOLTAGE_T total_voltage);
TOTAL_VOLTAGE_T pd_get_total_voltage();

VOLTAGE_T pd_set_max_voltage(VOLTAGE_T voltage);
VOLTAGE_T pd_get_max_voltage();

VOLTAGE_T pd_set_min_voltage(VOLTAGE_T voltage);
VOLTAGE_T pd_get_min_voltage();

TEMPERATURE_T pd_set_temperature(uint8_t index, TEMPERATURE_T temperature);
TEMPERATURE_T pd_get_temperature(uint8_t index);

TEMPERATURE_T pd_set_avg_temperature(TEMPERATURE_T temperature);
TEMPERATURE_T pd_get_avg_temperature();

TEMPERATURE_T pd_set_max_temperature(TEMPERATURE_T temperature);
TEMPERATURE_T pd_get_max_temperature();

TEMPERATURE_T pd_set_min_temperature(TEMPERATURE_T temperature);
TEMPERATURE_T pd_get_min_temperature();

CURRENT_T pd_set_current(CURRENT_T current);
CURRENT_T pd_get_current();

#endif