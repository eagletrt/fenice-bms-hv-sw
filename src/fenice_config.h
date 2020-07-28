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
//=================================== General ===============================
//===========================================================================

#define htim_Err htim2

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

#define PRECHARGE_TIMEOUT 10000
#define PRECHARGE_VOLTAGE_THRESHOLD 0.95

/**
 * Feedback bit set bit position 
 */
enum {
	FEEDBACK_VREF_POS,
	FEEDBACK_FROM_TSMS_POS,
	FEEDBACK_TO_TSMS_POS,
	FEEDBACK_FROM_SHUTDOWN_POS,
	FEEDBACK_LATCH_IMD_POS,
	FEEDBACK_LATCH_BMS_POS,
	FEEDBACK_IMD_FAULT_POS,
	FEEDBACK_BMS_FAULT_POS,
	FEEDBACK_TSAL_HV_POS,
	FEEDBACK_AIR_POSITIVE_POS,
	FEEDBACK_AIR_NEGATIVE_POS,
	FEEDBACK_PC_END_POS,
	FEEDBACK_RELAY_LV_POS,
	FEEDBACK_IMD_SHUTDOWN_POS,
	FEEDBACK_BMS_SHUTDOWN_POS,
	FEEDBACK_TS_ON_POS,

	//do not move FEEDBACK_N
	FEEDBACK_N,
};

/**
 * Feedback bit sets 
 */
#define FEEDBACK_NULL 0
#define FEEDBACK_VREF ((FEEDBACK_T)1 << FEEDBACK_VREF_POS)
#define FEEDBACK_FROM_TSMS ((FEEDBACK_T)1 << FEEDBACK_FROM_TSMS_POS)
#define FEEDBACK_TO_TSMS ((FEEDBACK_T)1 << FEEDBACK_TO_TSMS_POS)
#define FEEDBACK_FROM_SHUTDOWN ((FEEDBACK_T)1 << FEEDBACK_FROM_SHUTDOWN_POS)
#define FEEDBACK_LATCH_IMD ((FEEDBACK_T)1 << FEEDBACK_LATCH_IMD_POS)
#define FEEDBACK_LATCH_BMS ((FEEDBACK_T)1 << FEEDBACK_LATCH_BMS_POS)
#define FEEDBACK_IMD_FAULT ((FEEDBACK_T)1 << FEEDBACK_IMD_FAULT_POS)
#define FEEDBACK_BMS_FAULT ((FEEDBACK_T)1 << FEEDBACK_BMS_FAULT_POS)
#define FEEDBACK_TSAL_HV ((FEEDBACK_T)1 << FEEDBACK_TSAL_HV_POS)
#define FEEDBACK_AIR_POSITIVE ((FEEDBACK_T)1 << FEEDBACK_AIR_POSITIVE_POS)
#define FEEDBACK_AIR_NEGATIVE ((FEEDBACK_T)1 << FEEDBACK_AIR_NEGATIVE_POS)
#define FEEDBACK_PC_END ((FEEDBACK_T)1 << FEEDBACK_PC_END_POS)
#define FEEDBACK_RELAY_LV ((FEEDBACK_T)1 << FEEDBACK_RELAY_LV_POS)
#define FEEDBACK_IMD_SHUTDOWN ((FEEDBACK_T)1 << FEEDBACK_IMD_SHUTDOWN_POS)
#define FEEDBACK_BMS_SHUTDOWN ((FEEDBACK_T)1 << FEEDBACK_BMS_SHUTDOWN_POS)
#define FEEDBACK_TS_ON ((FEEDBACK_T)1 << FEEDBACK_TS_ON_POS)
#define FEEDBACK_ALL /**/ (FEEDBACK_T)(((FEEDBACK_T)1 << FEEDBACK_N) - 1)

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
#define SI8900_TIMEOUT 5

/**
 * Reference voltage of the ADC
*/
#define SI8900_VREF 3.33

/*
 * If the cli should echo the input
*/
#define CLI_ECHO 1

#endif /* CHIMERA_CONFIG_H_ */
