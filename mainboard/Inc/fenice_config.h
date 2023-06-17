/**
 * @file		fenice_config.h
 * @brief		This file contains configuration settings for Fenice
 *
 * @date	Oct 07, 2019
 * 
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef FENICE_CONFIG_H
#define FENICE_CONFIG_H

#include <inttypes.h>

//===========================================================================
//=================================== General ===============================
//===========================================================================

#define DISCHARGE_R   10   //Ohm
#define CELL_CAPACITY 3.9  //Ah
/**
 * Maximum can payload. for CAN 2.0A is 8 bytes
 */
#define CAN_MAX_PAYLOAD_LENGTH 8 * 8

/**
 * Number of daisy chained LTCs
 */
#define CELLBOARD_COUNT 6

/**
 * Number of cells a single IC controls. Refer to cell_distribution for
 * configuration
 */
#define CELLBOARD_CELL_COUNT 18

/*
 * Voltage value in mV * 10
 */
typedef uint16_t voltage_t;

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b)) // Get the maximum between two values
#endif // MAX(a, b)

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b)) // Get the minimum between two values
#endif // MIN(a, b)

//===========================================================================
//=================================== LTC6813 ===============================
//===========================================================================

#define LTC6813_PERIPHERAL hspi1

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
#define TEMP_STRIPS_PER_BUS 6
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
#define CELL_MIN_VOLTAGE  30000
#define CELL_MAX_VOLTAGE  42000

/**
 * Maximum cell temperature (°C)
 */
#define CELL_MAX_TEMPERATURE 60.0

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
#define BAL_MAX_VOLTAGE_THRESHOLD 500

/**
 *  How much does a balancing cycle last (ms)
 */
#define BAL_CYCLE_LENGTH 30000
#define BAL_TIME_ON      1000
#define BAL_TIME_OFF     1000

/**
 *  How much to wait for voltages to stabilize after a balancing cycle [ms]
 */
#define BAL_COOLDOWN_DELAY 5000

#define DISCHARGE_DUTY_CYCLE \
    (((float)BAL_CYCLE_LENGTH * BAL_TIME_ON / (BAL_TIME_ON + BAL_TIME_OFF)) / (BAL_CYCLE_LENGTH + BAL_COOLDOWN_DELAY))

// @section Pre-charge

#define PRECHARGE_TIMEOUT                    10000U
#define PRECHARGE_CHECK_INTERVAL             100U
#define PRECHARGE_VOLTAGE_THRESHOLD          0.95f
#define PRECHARGE_VOLTAGE_THRESHOLD_CARELINO 0.90f

enum {
    FEEDBACK_TSAL_GREEN_FAULT_POS,
    FEEDBACK_IMD_LATCHED_POS,
    FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS,
    FEEDBACK_BMS_LATCHED_POS,
    FEEDBACK_EXT_LATCHED_POS,
    FEEDBACK_TSAL_GREEN_POS,
    FEEDBACK_TS_OVER_60V_STATUS_POS,
    FEEDBACK_AIRN_STATUS_POS,
    FEEDBACK_AIRP_STATUS_POS,
    FEEDBACK_AIRP_GATE_POS,
    FEEDBACK_AIRN_GATE_POS,
    FEEDBACK_PRECHARGE_STATUS_POS,
    FEEDBACK_TSP_OVER_60V_STATUS_POS,
    FEEDBACK_CHECK_MUX_POS,
    FEEDBACK_SD_IN_POS,
    FEEDBACK_SD_OUT_POS,

    FEEDBACK_MUX_N
};

enum {
    FEEDBACK_RELAY_SD_POS = FEEDBACK_MUX_N,
    FEEDBACK_IMD_FAULT_POS,
    FEEDBACK_SD_END_POS,

    FEEDBACK_N
};

/**
 * Feedback bit sets 
 */
#define FEEDBACK_NULL                     0
#define FEEDBACK_TSAL_GREEN_FAULT         ((feedback_t)1 << FEEDBACK_TSAL_GREEN_FAULT_POS)
#define FEEDBACK_IMD_LATCHED              ((feedback_t)1 << FEEDBACK_IMD_LATCHED_POS)
#define FEEDBACK_TSAL_GREEN_FAULT_LATCHED ((feedback_t)1 << FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS)
#define FEEDBACK_BMS_LATCHED              ((feedback_t)1 << FEEDBACK_BMS_LATCHED_POS)
#define FEEDBACK_EXT_LATCHED              ((feedback_t)1 << FEEDBACK_EXT_LATCHED_POS)
#define FEEDBACK_TSAL_GREEN               ((feedback_t)1 << FEEDBACK_TSAL_GREEN_POS)
#define FEEDBACK_TS_OVER_60V_STATUS       ((feedback_t)1 << FEEDBACK_TS_OVER_60V_STATUS_POS)
#define FEEDBACK_AIRN_STATUS              ((feedback_t)1 << FEEDBACK_AIRN_STATUS_POS)
#define FEEDBACK_AIRP_STATUS              ((feedback_t)1 << FEEDBACK_AIRP_STATUS_POS)
#define FEEDBACK_AIRP_GATE                ((feedback_t)1 << FEEDBACK_AIRP_GATE_POS)
#define FEEDBACK_AIRN_GATE                ((feedback_t)1 << FEEDBACK_AIRN_GATE_POS)
#define FEEDBACK_PRECHARGE_STATUS         ((feedback_t)1 << FEEDBACK_PRECHARGE_STATUS_POS)
#define FEEDBACK_TSP_OVER_60V_STATUS      ((feedback_t)1 << FEEDBACK_TSP_OVER_60V_STATUS_POS)
#define FEEDBACK_CHECK_MUX                ((feedback_t)1 << FEEDBACK_CHECK_MUX_POS)
#define FEEDBACK_SD_IN                    ((feedback_t)1 << FEEDBACK_SD_IN_POS)
#define FEEDBACK_SD_OUT                   ((feedback_t)1 << FEEDBACK_SD_OUT_POS)
#define FEEDBACK_RELAY_SD                 ((feedback_t)1 << FEEDBACK_RELAY_SD_POS)
#define FEEDBACK_IMD_FAULT                ((feedback_t)1 << FEEDBACK_IMD_FAULT_POS)
#define FEEDBACK_SD_END                   ((feedback_t)1 << FEEDBACK_SD_END_POS)
#define FEEDBACK_ALL                      (feedback_t)(((feedback_t)1 << (FEEDBACK_N)) - 1)

//===========================================================================
//=========================== S160 current transducer =======================
//===========================================================================

// 0A voltage level
#define S160_OFFSET (3.33 / 2)

// Sensitivity of the 50A sensor
#define S160_50A_SENS 6.67

//Sensitivity of the 300A sensor
#define S160_300A_SENS 40

/*
 * If the cli should echo the input
*/
#define CLI_ECHO 1

#endif /* FENICE_CONFIG_H_ */