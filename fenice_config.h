/**
 * @file		fenice_config.h
 * @brief		This file contains configuration settings for Chimera Evoluzione
 *
 * @date		Oct 07, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef FENICE_CONFIG_H
#define FENICE_CONFIG_H

#include <inttypes.h>

//===========================================================================
//=================================== General ===============================
//===========================================================================

/**
 * Maximum can payload. for CAN 2.0A is 8 bytes
 */
#define CAN_MAX_PAYLOAD_LENGTH 8

//===========================================================================
//=================================== LTC6813 ===============================
//===========================================================================

#define LTC6813_PERIPHERAL hspi1

// Set to 1 to emulate the LTC daisy chain
#define LTC6813_EMU 1

/**
 * Number of daisy chained LTCs
 */
#define LTC6813_COUNT 6

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
static const uint8_t TEMP_SENSOR_ADDRESS_CODING[TEMP_SENSORS_PER_STRIP] = {000, 100, 010, 110, 020, 120};

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
 * Max current. (A)
 */
#define PACK_MAX_CURRENT 180.0f

/**
 * Cell's limit voltages (mV * 10)
 */
#define CELL_WARN_VOLTAGE 28000
#define CELL_MIN_VOLTAGE  25000
#define CELL_MAX_VOLTAGE  42250

/**
 * Maximum cell temperature (°C * 100)
 */
#define CELL_MAX_TEMPERATURE 6000

/**
 * Cell nominal energy (Wh * 10)
 */
#define CELL_ENERGY_NOMINAL 576

/**
 * Pack nominal energy (Wh * 10)
 */
#define PACK_ENERGY_NOMINAL (CELL_ENERGY_NOMINAL * PACK_CELL_COUNT)

// @section Balancing

/**
 *  Maximum voltage delta between cells (mV * 10)
 */
#define BAL_MAX_VOLTAGE_THRESHOLD 1000

/**
 *  How much does a balancing cycle last (ms)
 */
#define BAL_CYCLE_LENGTH 120000

/**
 *  How much to wait for voltages to stabilize after a balancing cycle [ms]
 */
#define BAL_COOLDOWN_DELAY 5000

typedef uint16_t voltage_t;
typedef uint8_t temperature_t;

/*
 * If the cli should echo the input
*/
#define CLI_ECHO 1

#endif /* FENICE_CONFIG_H_ */