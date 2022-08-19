/**
 * @file    soc.c
 * @brief   State of Charge estimation and management. Based on the energy subsystem
 *
 * @date    Sep 25, 2021
 *
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include "eeprom-config.h"
#include "pack/current.h"

/**
 * @brief Initializes the SoC management subsystem by loading energy info from EEPROM
 */
void soc_init(float lower_cell_voltage);

/**
 * @brief Process a new current sample.
 * 
 * @param timestamp The timestamp at which the measurement occurred
 */
void soc_sample_energy(uint32_t timestamp);

/**
 * @brief Resets the SoC count since last charge
 */
void soc_reset_soc();

/**
 * @brief returns the percentage of energy spent since last charge
 * 
 * @return float current value in %
 */
float soc_get_soc();

/**
 * @brief Returns the total energy count
 * 
 * @return float current value in Wh
 */
float soc_get_energy_total();

float soc_volt_to_capacity(float volt);
float soc_volt_to_energy(float volt);
