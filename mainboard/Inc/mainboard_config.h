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

#define htim_err   htim3
#define htim_bms   htim2
#define htim_super htim4

#define spi_eeprom hspi2

#define cli_uart huart1

#define STATE_LED_GPIO LED2_GPIO_Port
#define STATE_LED_PIN  LED2_Pin

#endif