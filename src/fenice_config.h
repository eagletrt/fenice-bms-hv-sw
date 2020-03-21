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

//===========================================================================
//=================================== LTC6813 ===============================
//===========================================================================

// Set to 1 to emulate the LTC daisy chain
#define LTC6813_EMU 0

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
 * Number of registers for each LTC
 */
#define LTC6813_REG_COUNT 6

/**
 * Number of cells handled by a register
 */
#define LTC6813_REG_CELL_COUNT 3

#define LTC6813_TEMP_ADDRESS 69

//===========================================================================
//================================= Temperature =============================
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
 * How many strips in each bus
 */
#define TEMP_STRIPS_PER_BUS 1
/**
 * How many sensors are on a strip
 */
#define TEMP_SENSORS_PER_STRIP 6

/**
 * How many sensors on each cellboard
 */
#define TEMP_SENSOR_COUNT (TEMP_BUS_COUNT * TEMP_STRIPS_PER_BUS * TEMP_SENSORS_PER_STRIP)

/**
 * The address pin coding for each sensor in a strip. The LSB must be changed
 * from [0-2] to differentiate between each strip.
 */
static const uint8_t TEMP_SENSOR_ADDRESS_CODING[TEMP_SENSORS_PER_STRIP] = {
	000, 100, 010, 110, 020, 120};

//===========================================================================
//================================ Pack Settings ============================
//===========================================================================

/**
 * Total number of cells in series
 */
#define PACK_CELL_COUNT (LTC6813_COUNT * LTC6813_CELL_COUNT)

/**
 * How many temperature sensors in the pack
 */
#define PACK_TEMP_COUNT (TEMP_SENSOR_COUNT * LTC6813_COUNT)

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
//=============================== SI8900 Settings ===========================
//===========================================================================

/**
 * Max time to wait for the sensor to initialize (auto-baudrate detection)
*/
#define SI8900_INIT_TIMEOUT 1000

/**
 * Max time to wait for a voltage reading
 * 
 * Keep it low, it will pause the main loop
*/
#define SI8900_TIMEOUT 0

/**
 * Reference voltage of the ADC
*/
#define SI8900_VREF 3.33

/*
 * If the cli should echo the input
*/
#define CLI_ECHO 1

#endif /* CHIMERA_CONFIG_H_ */
