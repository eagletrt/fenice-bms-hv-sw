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

static const uint8_t addresses[223] = {
	[000] = 0b10010000, [001] = 0b10010010, [010] = 0b10010100,
	[011] = 0b10010110, [100] = 0b10011000, [101] = 0b10011010,
	[110] = 0b10011100, [111] = 0b10011110, [200] = 0b11100000,
	[202] = 0b11100010, [201] = 0b11100100, [210] = 0b11100110,
	[212] = 0b11101000, [211] = 0b11101010, [220] = 0b11101100,
	[221] = 0b11101110, [020] = 0b01010000, [021] = 0b01010010,
	[120] = 0b01010100, [121] = 0b01010110, [002] = 0b01011000,
	[012] = 0b01011010, [102] = 0b01011100, [112] = 0b01011110,
	[022] = 0b01101010, [122] = 0b01101100, [222] = 0b01101110};

#define NUM_SENSORS 6
static const uint8_t addr[NUM_SENSORS] = {0b10010000, 0b10011000, 0b10010100,
										  0b10011100, 0b01010000, 0b01010100};

uint16_t pct2075_read(I2C_HandleTypeDef *hi2c, uint8_t index);

#endif