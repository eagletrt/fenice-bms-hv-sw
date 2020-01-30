/**
 * @file		pct2075.c
 * @brief		This file contains the functions to measure cell temperatures
 *
 * @date		Jan 30, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "pct2075.h"

uint16_t pct2075_read(I2C_HandleTypeDef *hi2c, uint8_t index) {
	uint8_t recv[2] = {0};
	HAL_I2C_Master_Receive(hi2c, addr[index], recv, 2, 10);

	return ((uint16_t)recv[0]) << 3 | recv[1] >> 5;
}