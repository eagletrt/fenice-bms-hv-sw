/**
 * @file		si8900.c
 * @brief		This file contains the functions to read bus and total pack
 * voltages
 *
 * @date		Gen 09, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "peripherals/si8900.h"

#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "usart.h"

bool si8900_init(UART_HandleTypeDef *huart) {
	uint8_t recv = 0;
	uint8_t tx = 0xAA;

	uint32_t time = HAL_GetTick();

	bool code_confirm = false;
	bool timeout = false;

	// HAL_UART_Transmit(hspi, tx, 1, 1);

	while (!code_confirm || !timeout) {
		HAL_UART_Transmit(huart, &tx, 1, 1);

		// WARN: Receiving without interrupts can cause unexpected
		// initialization problems
		HAL_UART_Receive(huart, &recv, 1, 1);

		if (recv == 0x55) {
			HAL_UART_Receive(huart, &recv, 1, 1);

			if (recv == 0x55) {
				code_confirm = true;
			}
		}

		timeout = time - HAL_GetTick() >= TIMEOUT;
	}

	HAL_UART_Receive(huart, NULL, 1, 10);
	return code_confirm || timeout;
}

bool si8900_read_channel(UART_HandleTypeDef *huart, SI8900_CHANNEL ch,
						 uint16_t *voltage) {
	uint8_t conf = cnfg_0 | (ch << 4);

	HAL_UART_Transmit(huart, &conf, 1, 1);

	uint8_t recv[3] = {0};
	uint8_t tmp = 0;
	do {
		HAL_UART_Receive(huart, recv, 3, 2);
	} while (tmp != conf);
	// if (recv[0] == conf) {
	// uint8_t recv[2] = {0};

	// HAL_UART_Receive(huart, recv, 2, 1);
	*voltage = si8900_convert_voltage(recv + 1);

	return true;
	//}

	return false;
}

/**
 * @brief	Reads all the voltages using demand mode
 *
 * @param	huart	the UART configuration structure
 * @param ain		the output array
 */
void si8900_read_voltages(UART_HandleTypeDef *huart, uint16_t ain[3]) {
	for (uint8_t i = 0; i <= 2; i++) {
		uint8_t conf = cnfg_0 | (i << 4);

		HAL_UART_Transmit(huart, &conf, 1, 1);

		uint8_t tmp = 0;
		do {
			HAL_UART_Receive(huart, &tmp, 1, 1);
		} while (tmp != conf);

		uint8_t recv[2];
		HAL_UART_Receive(huart, recv, 2, 1);

		ain[i] = si8900_convert_voltage(recv);
	}
}

/**
 * @brief Extracts the voltage from ADC_H and ADC_L bytes
 *si8900_read_voltages
 * @param	adc_hl	the array of ADC_H and ADC_L
 * @returns				The voltage value
 */
uint16_t si8900_convert_voltage(uint8_t adc_hl[2]) {
	// MSB | LSB
	uint16_t dig =
		((adc_hl[0] & 0b00001111) << 6) | ((adc_hl[1] & 0b01111110) >> 1);

	return ((VREF * (float)dig) / 1024) * 100;
}