/**
 * @file    energy.h
 * @brief   This file contains functions and utilities around energy monitoring.
 * 
 * @date    May 12, 2021
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <inttypes.h>
/* Exported types ------------------------------------------------------------*/
typedef struct energy {
    uint32_t last_sample;
    float energy;
    float last_power;
} energy_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief Initializes energy instance
 */
void energy_init(energy_t *handle);

/**
 * @brief Sets a given energy value as the current energy count
 * 
 * @param handle energy_t instance
 * @param joule Energy to set (in Joule)
 */
void energy_set_count(energy_t *handle, float energy);

/**
 * @brief Resets integration time to a given timestamp.
 * 
 * @param handle energy_t instance
 * @param time Time to set (in milliseconds)
 */
void energy_set_time(energy_t *handle, uint32_t time);

/**
 * @brief   Updates energy counting with given data
 * 
 * @param handle energy_t instance
 * @param power	Sampled power value (in Watt)
 * @param time Time at which the measurement was made (in milliseconds)
 */
void energy_sample_energy(energy_t *handle, float power, uint32_t time);

/**
 * @brief Calculate total consumption since last charge
 * 
 * @returns the consumption in Wh
 */
float energy_get_wh(energy_t handle);

/**
 * @brief Calculate total consumption since last charge
 * 
 * @returns the consumption in Joule
 */
float energy_get_joule(energy_t handle);

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private Macros -----------------------------------------------------------*/
