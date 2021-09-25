/**
 * @file	current.h
 * @brief	Functions that handle current measurement
 *
 * @date	Sep 24, 2021
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */
#pragma once

#include <fenice_config.h>

enum CURRENT_SENSORS { CURRENT_SENSOR_50 = 0, CURRENT_SENSOR_300, CURRENT_SENSOR_SHUNT, CURRENT_SENSOR_NUM };

typedef float current_t;

/**
 * @brief Measures TS current from internal current sensors
 */
void current_measure();

/**
 * @brief Zeroes the Hall-effect sensor
 */
void current_zero();

/**
 * @brief Returns the current flowing through the TS
 */
current_t current_get_current();

/**
 * @brief Returns the currents measured by each current sensor. Refer to CURRENT_SENSORS enum for value-sensor association
 * 
 * @return current_t* 
 */
current_t *current_get_current_sensors();

/**
 * @brief Returns the zero value for each current sensor. For the shunt the zero is always 0
 */
current_t *current_get_zero();