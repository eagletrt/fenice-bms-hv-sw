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

#define HTIM_IMD        htim2
#define HTIM_ERR        htim3
#define HTIM_MEASURES   htim4
#define HTIM_BAL        htim5
#define HTIM_BMS        htim8
#define HTIM_MUX        htim10

#define SPI_EEPROM      hspi2
#define SPI_ADC124S     hspi1

#define ADC_HALL50      hadc2
#define ADC_HALL300     hadc3
#define ADC_MUX         hadc1

#define CAR_CAN         hcan1
#define BMS_CAN         hcan2

#define STATE_LED_GPIO  LED2_GPIO_Port
#define STATE_LED_PIN   LED2_Pin

#endif