/**
 * @file		fenice_config.h
 * @brief		This file contains configuration settings for Chimera Evoluzione
 *
 * @date		Oct 07, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef FENICE_CONFIG_H
#define FENICE_CONFIG_H

#include <inttypes.h>

// TODO: REMOVE
#define CS_LTC_Pin GPIO_PIN_4
#define CS_LTC_GPIO_Port GPIOA

//===========================================================================
//=================================== LTC6813 ===============================
//===========================================================================

/**
 * Number of daisy chained LTCs
 */
#define LTC6813_COUNT 1

/**
 * Number of cells a single IC controls. Refer to cell_distribution for
 * configuration
 */
#define LTC6813_CELL_COUNT 18

/**
 * Number of registers for a single IC. A, B, C, D
 */
#define LTC6813_REG_COUNT 6

/**
 * Max number of cells handled by a register. Refer to cell distribution
 */
#define LTC6813_REG_CELL_COUNT 3

//===========================================================================
//================================ Pack Settings ============================
//===========================================================================

/**
 * Total number of cells in series
 */
#define PACK_MODULE_COUNT LTC6813_COUNT* LTC6813_CELL_COUNT

/**
 * Max current. In (A * 10)
 */
#define PACK_MAX_CURRENT 2000

/**
 * Cell's limit voltages (mV * 10)
 */
#define CELL_WARN_VOLTAGE 28000
#define CELL_MIN_VOLTAGE 25000
#define CELL_MAX_VOLTAGE 42250

/**
 * Maximum cell temperature (°C * 100)
 */
#define CELL_MAX_TEMPERATURE 6000

// @section Balancing

/**
 *  Maximum voltage delta between cells (mV * 10)
 */
#define BAL_MAX_VOLTAGE_THRESHOLD 100

/**
 *  How much does a balancing cycle last (ms)
 */
#define BAL_CYCLE_LENGTH 120000

//===========================================================================
//================================== Cellboard ==============================
//===========================================================================

/**
 * Temperature measurement interval (ms)
 */
#define TEMP_READ_INTERVAL 100

/**
 * How many temperatures to average together
 */
#define TEMP_SAMPLE_COUNT 4

/**
 * How many I2C buses are used to address all the sensors
 */
#define TEMP_BUS_COUNT 1

/**
 * How many sensor strips per bus
 */
#define TEMP_STRIPS_PER_BUS 1
/**
 * How many sensors are on a strip
 */
#define TEMP_SENSORS_PER_STRIP 6

/**
 * How many sensors on each cellboard
 */
#define TEMP_SENSOR_COUNT \
	TEMP_BUS_COUNT* TEMP_STRIPS_PER_BUS* TEMP_SENSORS_PER_STRIP

/**
 * The address pin coding for each sensor in a strip. The LSB must be changed
 * from 0 to 2 to differentiate between each strip.
 */
static const uint8_t TEMP_SENSOR_ADDRESS_CODING[TEMP_SENSORS_PER_STRIP] = {
	000, 100, 010, 110, 020, 120};

#endif /* CHIMERA_CONFIG_H_ */
