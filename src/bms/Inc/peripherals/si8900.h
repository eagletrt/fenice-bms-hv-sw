/**
 * @file		si8900.h
 * @brief		This file contains the functions to read bus and total pack
 * voltages
 *
 * @date		Gen 09, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @coauthor Simone Ruffini [simone.ruffini@tutanota.com]
 */

#ifndef SI8902_H
#define SI8902_H

#include <inttypes.h>
#include <stdbool.h>
#include <stm32f4xx_hal.h>

#include "main.h"

#define SI8900_VREF 3.33
#define SI8900_INIT_TIMEOUT 1000
#define SI8900_TIMEOUT 50

static const uint8_t si8900_cnfg_0 = 0b11000011;

typedef enum {
	SI8900_AIN0 = 0,
	SI8900_AIN1 = 1,
	SI8900_AIN2 = 2
} SI8900_CHANNEL;

bool si8900_init(UART_HandleTypeDef *hspi);
bool si8900_read_channel(UART_HandleTypeDef *huart, SI8900_CHANNEL ch,
						 uint16_t *voltage);
void si8900_read_voltages(UART_HandleTypeDef *huart, uint16_t ain[3]);
uint16_t si8900_convert_voltage(uint8_t adc_hl[2]);

#endif