/**
 * @file		ltc6813_utils.c
 * @brief		This file contains utilities for improving LTC6813
 * 					communications
 *
 * @date		Nov 16, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "comm/ltc6813_utils.h"

/**
 * @brief		Checks that voltage is between its thresholds.
 *
 * @param		volts		The voltage
 * @param		error		The error return code
 */
void ltc6813_check_voltage(uint16_t volts, ERROR_STATUS_T *volt_error,
						   WARNING_T *warning, ERROR_T *error) {
	if (volts < CELL_WARN_VOLTAGE) {
		*warning = WARN_CELL_LOW_VOLTAGE;
	}

	if (volts < CELL_MIN_VOLTAGE) {
		error_set(ERROR_CELL_UNDER_VOLTAGE, volt_error, HAL_GetTick());
	} else {
		error_unset(ERROR_CELL_UNDER_VOLTAGE, volt_error);
	}

	if (volts > CELL_MAX_VOLTAGE) {
		error_set(ERROR_CELL_OVER_VOLTAGE, volt_error, HAL_GetTick());
	} else {
		error_unset(ERROR_CELL_OVER_VOLTAGE, volt_error);
	}

	*error = error_check_fatal(volt_error, HAL_GetTick());
	ER_CHK(error);

End:;
}

/**
 * @brief		Checks that temperature is between its thresholds.
 *
 * @param		temp		The temperature
 * @param		error		The error return code
 */
void ltc6813_check_temperature(uint16_t temps, ERROR_STATUS_T *temp_error,
							   ERROR_T *error) {
	if (temps >= CELL_MAX_TEMPERATURE) {
		error_set(ERROR_CELL_OVER_TEMPERATURE, temp_error, HAL_GetTick());
	} else {
		error_unset(ERROR_CELL_OVER_TEMPERATURE, temp_error);
	}

	*error = error_check_fatal(temp_error, HAL_GetTick());
	ER_CHK(error);

End:;
}

void ltc6813_set_dcc(uint8_t indexes[], uint8_t cfgar[8], uint8_t cfgbr[8]) {
	for (uint8_t i = 0; i < PACK_MODULE_COUNT; i++) {
		if (indexes[i] < 8) {
			cfgar[4] += dcc[indexes[i]];
		} else if (indexes[i] >= 8 && indexes[i] < 12) {
			cfgar[5] += dcc[indexes[i]];
		} else if (indexes[i] >= 12 && indexes[i] < 16) {
			cfgbr[0] += dcc[indexes[i]];
		} else if (indexes[i] >= 16 && indexes[i] < 18) {
			cfgbr[1] += dcc[indexes[i]];
		}

		uint16_t pec = _pec15(6, cfgar);
		cfgar[6] = (uint8_t)(pec >> 8);
		cfgar[7] = (uint8_t)(pec);

		pec = _pec15(6, cfgbr);
		cfgbr[6] = (uint8_t)(pec >> 8);
		cfgbr[7] = (uint8_t)(pec);
	}
}