/**
 * @file		ltc6813_utils.h
 * @brief		This file contains utilities for improving LTC6813
 * 					communications
 *
 * @date		Nov 16, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include <inttypes.h>
#include "error.h"

void ltc6813_set_dcc(uint8_t indexes[], uint8_t cfgar[8], uint8_t cfgbr[8]);

void ltc6813_check_voltage(uint16_t volt, ERROR_STATUS_T *volt_error,
						   WARNING_T *warning, ERROR_T *error);
void ltc6813_check_temperature(uint16_t temp, ERROR_STATUS_T *temp_error,
							   ERROR_T *error);