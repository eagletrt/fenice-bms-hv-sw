/**
 * @file		ltc1865.h
 * @brief		This file contains the functions to read total pack/bus voltages
 *
 * @date		Nov 15, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef LTC1865L_H
#define LTC1865L_H

#include <stdbool.h>
#include <stm32f4xx_hal.h>

uint16_t _ltc1865l_convert_voltage(uint8_t value[2]);
uint16_t ltc1865l_read_voltage(SPI_HandleTypeDef *spi, GPIO_TypeDef *GPIOx,
							   uint16_t GPIO_Pin, bool single, bool odd);

#endif