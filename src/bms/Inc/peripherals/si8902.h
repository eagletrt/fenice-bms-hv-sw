/**
 * @file		si8902.h
 * @brief		This file contains the functions to read bus and total pack
 * voltages
 *
 * @date		Gen 09, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef SI8902_H
#define SI8902_H

#include <inttypes.h>
#include <stm32f4xx_hal.h>

#include "main.h"

uint8_t cnfg_0 = 0b11000010;

typedef enum {
	AIN0 = 0b00000000,
	AIN1 = 0b00010000,
	AIN2 = 0b00100000
} MUX_CHANNEL;

void si8902_read_voltages(SPI_HandleTypeDef *hspi, uint16_t ain[3]);
uint16_t si8902_convert_voltage(uint8_t adc_data[2]);

#endif