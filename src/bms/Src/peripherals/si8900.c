/**
 * @file		si8900.c
 * @brief		This file contains the functions to read bus and total pack
 * voltages
 *
 * @date		Gen 09, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @coauthor Simone Ruffini [simone.ruffini@tutanota.com]
 */

#include "peripherals/si8900.h"

#include <stdlib.h>
#include <string.h>

#include "../../../fenice_config.h"
#include "cli.h"
#include "error/error.h"
#include "usart.h"

bool si8900_ready = false;

/**
 * @brief		Initializes the ADC
 * @details	This function does the auto-baudrate detection initialization	function described
 * 					in si's datasheet: https://www.silabs.com/documents/public/application-notes/AN635.pdf
 * 
 * @param		huart	The UART configuration structure
 * @returns	whether the initialization ended successfully.
 */
bool si8900_init(UART_HandleTypeDef *huart) {
	uint8_t recv = 0;
	uint8_t tx = 0xAA;

	bool timeout = false;
	bool code_receive = false;
	bool code_confirm = false;

	uint32_t time = HAL_GetTick();

	HAL_UART_Transmit(huart, &tx, 1, 10);

	while ((!code_receive || !code_confirm) && !timeout) {
		HAL_UART_Receive(huart, &recv, 1, 10);
		if (recv == 0x55) {
			if (code_receive) {
				code_confirm = true;
				si8900_ready = true;
			}
			code_receive = true;
		} else {
			code_receive = false;
			code_confirm = false;
		}
		HAL_UART_Transmit(huart, &tx, 1, 10);

		timeout = (HAL_GetTick() - time) >= SI8900_INIT_TIMEOUT;
	}

	if (timeout) {
		error_set(ERROR_ADC_INIT, 0, HAL_GetTick());
		return false;
	}
	error_unset(ERROR_ADC_INIT, 0);
	return true;
}

/**
 * @brief	Reads a single ADC channel in demand mode
 * 
 * @param	huart		The UART configuration structure
 * @param	ch			The channel
 * @param	voltage	The output voltage
 * 
 * @returns whether the reading succeded
 */
bool si8900_read_channel(UART_HandleTypeDef *huart, SI8900_CHANNEL ch,
						 uint16_t *voltage) {
	if (si8900_ready) {
		uint8_t conf = si8900_cnfg_0 | (ch << 4);

		HAL_UART_Transmit(huart, &conf, 1, 1);

		uint32_t time = HAL_GetTick();
		uint8_t tmp = 0;
		do {
			HAL_UART_Receive(huart, &tmp, 1, 1);

			if ((HAL_GetTick() - time) >= SI8900_TIMEOUT) {
				error_set(ERROR_ADC_TIMEOUT, 0, HAL_GetTick());
				return false;
			}
		} while (tmp != conf);
		error_unset(ERROR_ADC_TIMEOUT, 0);

		uint8_t recv[2];
		HAL_UART_Receive(huart, recv, 2, 1);
		*voltage = si8900_convert_voltage(recv);

		return true;
	}
	return false;
}

/**
 * @brief Computes the voltage from ADC_H and ADC_L bytes
 *				si8900_read_voltages
 * @param	adc_hl	ADC_H and ADC_L
 * @returns	The voltage value (eg. 321 -> 3.21V)
 */
uint16_t si8900_convert_voltage(uint8_t adc_hl[2]) {
	// MSB | LSB
	uint16_t dig =
		((adc_hl[0] & 0b00001111) << 6) | ((adc_hl[1] & 0b01111110) >> 1);

	return ((SI8900_VREF * (float)dig) / 1024) * 100;
}