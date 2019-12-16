/**
 * @file		ltc1865.c
 * @brief		This file contains the functions to read total pack/bus voltages
 *
 * @date		Nov 15, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "comm/ltc1865l.h"

uint16_t _ltc1865l_convert_voltage(uint8_t value[2]) {
	// TODO: Add conversion code
	return (value[1] << 8) + value[0];
}

uint16_t ltc1865l_read_voltage(SPI_HandleTypeDef *spi, GPIO_TypeDef *GPIOx,
							   uint16_t GPIO_Pin, bool single, bool odd) {
	uint8_t cmd[2] = {0};
	uint8_t data[2] = {0};

	cmd[0] |= single << 7;
	cmd[0] |= odd << 6;

	uint32_t start = HAL_GetTick();

	// Start measurement
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	// Generate a rising edge
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
	HAL_Delay(1);

	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(spi, cmd, 2, 10);
	HAL_SPI_Receive(spi, data, 2, 10);
	HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);

	return _ltc1865l_convert_voltage(data);
}