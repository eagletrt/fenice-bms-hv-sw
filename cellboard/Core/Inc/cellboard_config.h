/**
 * @file		cellboard_config.h
 * @brief		This file contains configuration settings for the cellboard
 *
 * @date		Jul 07, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef CELLBOARD_CONFIG_H
#define CELLBOARD_CONFIG_H

#include "../../../fenice_config.h"
#include "tim.h"

//===========================================================================
//=================================== LTC6813 ===============================
//===========================================================================

// Set to 1 to emulate the LTC daisy chain
#define LTC6813_EMU 0

/**
 * Number of registers for each LTC
 */
#define LTC6813_REG_COUNT 6

/**
 * Number of cells handled by a register
 */
#define LTC6813_REG_CELL_COUNT 3

#define VOLT_MEASURE_INTERVAL 8
#define VOLT_MEASURE_TIME     5

//===========================================================================
//================================= Temperature =============================
//===========================================================================

/**
 * Temperature measurement interval (ms)
 */
#define TEMP_MEASURE_INTERVAL 200

#define TEMP_ADC_COUNT 6

#define TEMP_ADC_SENSOR_COUNT 6

/**
 * How many sensors on each cellboard
 */
#define CELLBOARD_TEMP_SENSOR_COUNT (TEMP_ADC_COUNT * TEMP_ADC_SENSOR_COUNT)

//===========================================================================
//================================= Timers ==================================
//===========================================================================

/**
 * Timers interval (ms)
 */

#define TIM_MEASUREMENTS htim2
#define TIM_DISCHARGE htim15
#define TIM_COOLDOWN htim16

typedef float temperature_t;
typedef int16_t current_t;

#endif
