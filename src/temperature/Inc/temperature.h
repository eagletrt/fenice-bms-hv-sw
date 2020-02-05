/**
 * @file		temperature.h
 * @brief		This file contains the functions to manage the cellboard
 * temperature
 *
 * @date		Feb 03, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include "../../fenice_config.h"
#include "stm32f4xx_hal.h"

void temperature_read(I2C_HandleTypeDef *hi2c[TEMP_BUS_COUNT], uint16_t *temps);

void temperature_get_extremes(uint8_t temps[], uint8_t min[2], uint8_t max[2]);
#endif