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

#define CURRENT_SENSOR_DISCONNECTED_THRESHOLD 0.25 // Current values below this threshold are consider as the sensor is disconnected

enum CURRENT_SENSORS {
    CURRENT_SENSOR_50 = 0,
    CURRENT_SENSOR_300,
    CURRENT_SENSOR_SHUNT,
    CURRENT_SENSOR_NUM
};

typedef float current_t;

/** @brief Start current measurement */
void current_start_measure();

/**
 * @brief Reads TS current from current sensors
 * 
 * @param shunt_adc_val The voltage value read from the shunt
 * @return uint32_t The timestamp at which the measurement occurred
 */
uint32_t current_read(float shunt_adc_val);

/** @brief Zeroes the Hall-effect sensor */
void current_zero();

/** @brief Returns the current flowing through the TS */
current_t current_get_current();

/**
 * @brief Returns the currents measured by each current sensor
 * @details Refer to CURRENT_SENSORS enum for value-sensor association
 * 
 * @return current_t * A pointer to the array of currents
 */
current_t * current_get_current_sensors();

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