/**
 * @file	soc.c
 * @brief	File containing energy management functions for SoC management.
 *
 * @date	Sep 25, 2021
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include "config.h"
#include "current.h"

/**
 * @brief Initializes the energy management subsystem by reading SoC info from EEPROM
 */
void soc_init();

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
 * @brief returns the total energy spent since last charge
 * 
 * @return float current value in A
 */
float soc_get_soc();

/**
 * @brief Returns the total energy count
 * 
 * @return float current value in A
 */
float soc_get_energy_total();

/**
 * @brief Returns the energy since last charge
 * 
 * @return float current value in A
 */
float soc_get_energy_last_charge();