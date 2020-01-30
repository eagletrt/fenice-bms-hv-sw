/**
 * @file		pct2075.h
 * @brief		This file contains the functions to measure cell temperatures
 *
 * @date		Jan 30, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef PCT2075_H
#define PCT2075_H

#include <inttypes.h>

#include "stm32f4xx_hal.h"
#define CONV_TEMP(x) ((float)x * .125) * 10

#define NUM_SENSORS 6
static const uint8_t addr[NUM_SENSORS] = {0b10010000, 0b10011000, 0b10010100,
										  0b10011100, 0b01010000, 0b01010100};

uint16_t pct2075_read(I2C_HandleTypeDef *hi2c, uint8_t index);

#endif