/**
 * @file		pack.c
 * @brief		This file contains the functions to manage the battery pack
 *
 * @date		Apr 11, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "pack.h"

#include <math.h>
#include <stdbool.h>
#include <stm32g4xx_hal.h>

#include "peripherals/si8900.h"

#define CURRENT_ARRAY_LENGTH 512

uint32_t adc_current[CURRENT_ARRAY_LENGTH];	 // TODO: move to pck_data and update DMA and generate getter no setter
bal_conf_t balancing;						 // TODO: Remove bal_conf_t struct (remove enable and the rest has to be managed by the state machine with the eeprom too)
/**
 * @brief	Initializes the pack
 *
 */
void pack_init() {
	pd_set_bus_voltage(0);
	pd_set_adc_voltage(0);
	pd_set_total_voltage(0);
	pd_set_max_voltage(0);
	pd_set_min_voltage(0);
	pd_set_avg_temperature(0);
	pd_set_max_temperature(0);
	pd_set_min_temperature(0);

	pd_set_current(0);

	pd_feedback = 0;
	for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
		pd_set_voltage(i, 0);
	}

	for (uint8_t i = 0; i < TEMP_SENSOR_COUNT * LTC6813_COUNT; i++) {
		pd_set_temperature(i, 0);
	}

	balancing.enable = false;
	balancing.threshold = BAL_MAX_VOLTAGE_THRESHOLD;
	balancing.slot_time = 2;

	// LTC6813 GPIO configuration
	GPIO_CONFIG = GPIO_I2C_MODE;
}

/**
 * @brief		Polls all the LTCs and the SI8900 ADC for voltages
 * @details	This function should take 10~11ms to fully execute
 *
 * @param		hspi				The SPI configuration structure
 * @param		huart 			The UART configuration structure
 * @returns	The index of the last updated cell
 */
void pack_update_voltages(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart) {
	_ltc6813_adcv(hspi, 0);

	ltc6813_read_voltages(hspi);

	ADC_VOLTAGE_T adc;
	ADC_VOLTAGE_T bus;
	if (si8900_read_channel(huart, SI8900_AIN0, &adc)) {
		pd_set_adc_voltage(adc);
	}
	if (si8900_read_channel(huart, SI8900_AIN1, &bus)) {
		pd_set_bus_voltage(bus);
	}

	pack_update_voltage_stats();
}

/**
 * @brief		Gets quick temperature stats from the cellboards
 * @details	
 *
 * @param		hspi			The SPI configuration structure
 */
void pack_update_temperatures(SPI_HandleTypeDef *hspi) {
	uint8_t max[LTC6813_COUNT * 2];
	uint8_t min[LTC6813_COUNT * 2];

	ltc6813_read_temperatures(hspi, max, min);

	TEMPERATURE_T avg_temp = 0;
	TEMPERATURE_T max_temp = 0;
	TEMPERATURE_T min_temp = UINT8_MAX;

	for (uint8_t i = 0; i < LTC6813_COUNT; i++) {
		avg_temp += max[i * 2] + max[i * 2 + 1];
		avg_temp += min[i * 2] + min[i * 2 + 1];

		max_temp = fmax(max[i * 2], max_temp);
		min_temp = fmin(min[i * 2], min_temp);
	}

	pd_set_avg_temperature(((float)avg_temp / (LTC6813_COUNT * 4)) * 10);
	pd_set_max_temperature(max_temp);
	pd_set_min_temperature(min_temp);
}

void pack_update_all_temperatures(SPI_HandleTypeDef *hspi) {
	ltc6813_read_all_temps(hspi);
}

//TODO: redo DMA and thenreqrite the function
/**
 * @brief		Calculates the current exiting/entering the pack
 */
void pack_update_current() {
	CURRENT_T current = 0;

	int32_t tmp = 0;
	for (uint16_t i = 0; i < CURRENT_ARRAY_LENGTH; i++) {
		tmp += adc_current[i];
	}
	tmp /= CURRENT_ARRAY_LENGTH;

	// We calculate the input voltage
	float in_volt = (((float)tmp * 3.3) / 4096);

	// Check the current sensor datasheet for the correct formula
	current = (int16_t)(-round((((in_volt - 2.048) * 200 / 1.25)) * 10));
	current += 100;

	pd_set_current(current);

	error_toggle_check(current > PACK_MAX_CURRENT, ERROR_OVER_CURRENT, 0);
}

/**
 * @brief		Updates the pack's voltage stats
 * @details	It updates *_voltage variables with the data of the pack
 */
void pack_update_voltage_stats() {
	uint32_t tot_voltage = 0;
	VOLTAGE_T max_voltage = pd_get_voltage(0);
	VOLTAGE_T min_voltage = UINT16_MAX;

	for (uint16_t i = 0; i < PACK_CELL_COUNT; i++) {
		VOLTAGE_T tmp_voltage = pd_get_voltage(i);
		tot_voltage += (uint32_t)tmp_voltage;

		max_voltage = fmax(max_voltage, tmp_voltage);
		min_voltage = fmin(min_voltage, tmp_voltage);
	}

	pd_set_total_voltage(tot_voltage);
	pd_set_max_voltage(max_voltage);
	pd_set_min_voltage(fmin(min_voltage, max_voltage));
}

/**
 * @brief		Updates the pack's temperature stats
 * @details	It updates *_temperature variables with the data of the pack
 */
void pack_update_temperature_stats() {
	uint32_t avg_temperature = 0;
	TEMPERATURE_T max_temperature = 0;
	TEMPERATURE_T min_temperature = UINT8_MAX;

	for (uint16_t i = 0; i < TEMP_SENSOR_COUNT; i++) {
		TEMPERATURE_T tmp_temperature = pd_get_temperature(i);

		avg_temperature += (uint32_t)tmp_temperature;

		max_temperature = fmax(max_temperature, tmp_temperature);
		min_temperature = fmin(min_temperature, tmp_temperature);
	}

	pd_set_avg_temperature((TEMPERATURE_T)(avg_temperature / TEMP_SENSOR_COUNT));
	pd_set_max_temperature(max_temperature);
	pd_set_min_temperature(fmin(min_temperature, max_temperature));
}

bool pack_balance_cells(SPI_HandleTypeDef *hspi) {
	// TODO: Improve logging
	if (balancing.enable) {
		uint8_t indexes[PACK_CELL_COUNT];

		VOLTAGE_T voltages[PACK_CELL_COUNT];

		// makes a copy off all cell voltages
		pd_get_voltage_array(voltages);

		size_t len = bal_compute_indexes(voltages, indexes, balancing.threshold);

		if (len > 0) {
			ltc6813_set_balancing(hspi, indexes, balancing.slot_time);
			return true;
		}

		balancing.enable = false;
	}
	return false;
}

void pack_read_feedback(FEEDBACK_T fb_mask) {
	//initialize the pd_feedback value to 0 on the mask bits;
	pd_feedback &= (~fb_mask);
	for (uint8_t i = 0; i < FEEDBACK_N; ++i) {
		if ((1U << i) & fb_mask) {
			HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (i & 0b00000001));
			HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (i & 0b00000010));
			HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (i & 0b00000100));
			HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (i & 0b00001000));

			pd_feedback |= (HAL_GPIO_ReadPin(ANALOG_DATA_GPIO_Port, ANALOG_DATA_Pin) << i);
		}
	}
}

bool pack_feedback_check(FEEDBACK_T fb_check_mask, FEEDBACK_T fb_value, error_id error_id) {
	//remove not used bit with the mask and find the ones that differ with the xor
	uint16_t difference = (fb_check_mask & pd_feedback) ^ fb_value;

	for (uint8_t i = 0; i < FEEDBACK_N; i++) {
		if (fb_check_mask & (1U << i)) {
			error_toggle_check(difference & (1 << i), error_id, i);
		}
	}

	return pd_feedback == fb_value;
}

bool pack_set_ts_off() {
	//Switch off airs
	HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_RESET);

	pack_read_feedback(FEEDBACK_TS_OFF_MASK);
	pack_feedback_check(FEEDBACK_TS_OFF_MASK, FEEDBACK_TS_OFF_VAL, ERROR_FEEDBACK_HARD);

	// TODO: return something meaningful, or void
	return true;
}

bool pack_set_pc_start() {
	//switch on AIR-
	HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_SET);

	// Check feedback
	pack_read_feedback(FEEDBACK_TO_PRECHARGE_MASK);
	pack_feedback_check(FEEDBACK_TO_PRECHARGE_MASK, FEEDBACK_TO_PRECHARGE_VAL, ERROR_FEEDBACK_HARD);

	// TODO: return something meaningful, or void
	return true;
}

bool pack_set_precharge_end() {
	//switch on AIR+
	HAL_GPIO_WritePin(PC_ENDED_GPIO_Port, PC_ENDED_Pin, GPIO_PIN_SET);
	HAL_Delay(10);	// non so quanto debba essere. In chimera con 1ms si triggerava però boh
	HAL_GPIO_WritePin(PC_ENDED_GPIO_Port, PC_ENDED_Pin, GPIO_PIN_RESET);

	// Check feedback
	pack_read_feedback(FEEDBACK_ON_MASK);
	return pack_feedback_check(FEEDBACK_ON_MASK, FEEDBACK_ON_VAL, ERROR_FEEDBACK_HARD);
}

// //this ckeck is performed during ON state and from PRECHARGE TO ON
// bool pack_feedback_check_on() {
// 	uint16_t difference = pd_feedback ^ FEEDBACK_ON;

// 	for (uint8_t i = 0; i < FEEDBACK_N; i++) {
// 		error_toggle_check(difference & (1 << i), ERROR_FEEDBACK_HARD, i);
// 	}

// 	return pd_feedback == FEEDBACK_ON;
// }

// bool pack_feedback_check_charge() {
// 	uint16_t difference = pd_feedback ^ FEEDBACK_CHARGE;

// 	for (uint8_t i = 0; i < FEEDBACK_N; i++) {
// 		error_toggle_check(difference & (1 << i), ERROR_FEEDBACK_HARD, i);
// 	}

// 	return pd_feedback == FEEDBACK_CHARGE;
// }