/**
 * @file current.h
 * @brief Functions that handle current measurement
 *
 * @date Sep 24, 2021
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.untin.it]
 */

#ifndef CURRENT_H
#define CURRENT_H

#include <stdint.h>
#include <inttypes.h>

/** @brief Voltage values below this threshold are consider as the sensor is disconnected */
#define CURRENT_SENSOR_DISCONNECTED_THRESHOLD 0.25 // V

/** @brief Hall sensor sensitivity values */
#define CURRENT_SENSITIVITY_LOW 40e-3f // V / A
#define CURRENT_SENSITIVITY_HIGH 6.67e-3f // V / A

/** @brief Inverse divider ratio */
#define CURRENT_DIVIDER_RATIO_INVERSE ((330.f + 169.f) / 330.f)

// TODO: Use defines for shunt current conversion
/**
 * @brief Convert the shunt voltage value to current
 * 
 * @param volt The shunt voltage
 * @return current_t The shunt current
 */
#define CURRENT_SHUNT_TO_CURRENT(volt) (((volt) - 0.468205124f) / (1e-4f * 75.f))
/**
 * @brief Convert the 50A hall channel voltage value to current
 * 
 * @param volt The hall voltage
 * @return current_t The hall current
 */
#define CURRENT_HALL_50_TO_CURRENT(volt) ((CURRENT_DIVIDER_RATIO_INVERSE / CURRENT_SENSITIVITY_LOW) * (volt))
/**
 * @brief Convert the 300A hall channel voltage value to current
 * 
 * @param volt The hall voltage
 * @return current_t The hall current
 */
#define CURRENT_HALL_300_TO_CURRENT(volt) ((CURRENT_DIVIDER_RATIO_INVERSE / CURRENT_SENSITIVITY_HIGH) * (volt))


/** @brief Current sensor indices */
typedef enum {
    CURRENT_SENSOR_50 = 0,
    CURRENT_SENSOR_300,
    CURRENT_SENSOR_SHUNT,
    CURRENT_SENSOR_NUM
} CURRENT_SENSOR;

typedef float current_t;


/** @brief Start current measurement */
void current_start_measure();

/**
 * @brief Reads TS current from current sensors
 * 
 * @param shunt_voltage The voltage value read from the shunt
 */
void current_read(voltage_t shunt_voltage);

/** @brief Zeroes the Hall-effect sensor */
void current_zero();

/** @brief Returns the current flowing through the TS */
current_t current_get_current();

/**
 * @brief Returns the zero value for each current sensor
 * @attention For the shunt the zero value is always 0
 */
current_t * current_get_zero();

/** @brief Returns the current flowing through the specified sensor */
current_t current_get_current_from_sensor(uint8_t sensor);

/** @brief Check current related errors */
void current_check_errors();

#endif // CURRENT_H