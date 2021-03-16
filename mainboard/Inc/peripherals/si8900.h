/**
 * @file		si8900.h
 * @brief		This file contains the functions to read bus and total pack
 * 				voltages
 *
 * @date		Gen 09, 2020
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@tutanota.com]
 */

#ifndef SI8902_H
#define SI8902_H

#include <inttypes.h>
#include <stdbool.h>
#include <stm32g4xx_hal.h>

#include "main.h"

static const uint8_t si8900_cnfg_0 = 0b11000011;

typedef enum {
	SI8900_AIN0 = 0,
	SI8900_AIN1 = 1,
	SI8900_AIN2 = 2
} SI8900_CHANNEL;

/**
 * @brief		Initializes the ADC
 * @details		This function does the auto-baudrate detection initialization	function described
 * 				in si's datasheet: https://www.silabs.com/documents/public/application-notes/AN635.pdf
 * @warning		Make sure the default value of the reset pin is in a non-reset position for the Si8900
 * 				E.G. In CubeMX, the GPIO Output Level is LOW for Fenice's mainboard v1.0.5 (pin Si890x reset on the schematic)
 * 
 * @param		huart		The UART configuration structure
 * @param		reset_gpio	GPIO group for the ADC reset pin
 * @param		reset_pin	ADC reset pin number
 * @returns		whether the initialization ended successfully.
 */
bool si8900_init(UART_HandleTypeDef *huart, GPIO_TypeDef *reset_gpio, uint16_t reset_pin);

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
						 uint16_t *voltage);

/**
 * @brief	Computes the voltage from ADC_H and ADC_L bytes
 * 
 * @param	adc_hl	ADC_H and ADC_L
 * 
 * @returns	The voltage value (eg. 321 -> 3.21V)
 */
uint16_t si8900_convert_voltage(uint8_t adc_hl[2]);

#endif