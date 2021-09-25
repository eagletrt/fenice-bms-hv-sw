/**
 * @file		current.h
 * @brief		Functions that handle current measurement
 *
 * @date		Sep 24, 2021
 *
 * @author  	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */
#pragma once

#include <fenice_config.h>

enum { CURRENT_SENSOR_50 = 0, CURRENT_SENSOR_300, CURRENT_SENSOR_SHUNT, CURRENT_SENSOR_AMOUNT };

typedef float current_t;

void current_measure();
void current_zero();
current_t current_get_current();
current_t *current_get_current_sensors();
current_t *current_get_zero();