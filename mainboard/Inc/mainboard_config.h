/**
 * @file		mainboard_config.h
 * @brief		This file contains configuration settings for the mainboard
 *
 * @date		Jul 07, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef MAINBOARD_CONFIG_H
#define MAINBOARD_CONFIG_H

#include "../../fenice_config.h"

//===========================================================================
//=================================== General ===============================
//===========================================================================

#define HTIM_ERR   htim3
#define HTIM_BMS   htim2
#define HTIM_SUPER htim4

#define SPI_EEPROM hspi2

#define ADC_HALL50  hadc3
#define ADC_HALL300 hadc2

#define STATE_LED_GPIO LED2_GPIO_Port
#define STATE_LED_PIN  LED2_Pin

// @section Pre-charge

#define PRECHARGE_TIMEOUT           10000U
#define PRECHARGE_CHECK_INTERVAL    100U
#define PRECHARGE_VOLTAGE_THRESHOLD 0.95

//===========================================================================
//=============================== SI8900 Settings ===========================
//===========================================================================

/**
 * Max time to wait for the sensor to initialize (auto-baudrate detection)
*/
#define SI8900_INIT_TIMEOUT 400

/**
 * Max time to wait for a voltage reading
 * 
 * Keep it low, it will pause the main loop
*/
#define SI8900_TIMEOUT 5

/**
 * Reference voltage of the ADC
*/
#define SI8900_VREF 3.33

#endif