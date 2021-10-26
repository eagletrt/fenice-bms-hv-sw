/**
 * @file        soc.h
 * @brief	    This file contains functions and utilities around energy and State of Charge estimation.
 * 
 * @date        May 12, 2021
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _ENERGY_H_
#define _ENERGY_H_

/* Includes ------------------------------------------------------------------*/
#include "current.h"
#include "mainboard_config.h"

#include <inttypes.h>
/* Exported types ------------------------------------------------------------*/
typedef struct energy *energy_t;
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initializes soc instance
 */
void energy_init(energy_t *handle);

void energy_deinit(energy_t *handle);

/**
 * @brief Loads saved values and resets timer
 */
void energy_load(energy_t handle, float joule, uint32_t time);

/**
 * @brief Resets integration time to a given timestamp.
 * 
 * @details Useful for resetting count when the battery is turned on
 */
void energy_reset_time(energy_t handle, uint32_t time);

/**
 * @brief Resets all recorded values along with integration timestamp
 * 
 * @details Used when battery is fully charged or when consumption restricted to a certain period is wanted. 
 */
void energy_reset_count(energy_t handle, uint32_t time);

/**
 * @brief   Updates joule counting with given data
 */
void energy_sample_energy(energy_t handle, float power, uint32_t time);

/**
 * @brief Calculate total consumption since last charge
 * 
 * @returns the consumption in Wh
 */
float energy_get_wh(energy_t handle);
float energy_get_joule(energy_t handle);

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private Macros -----------------------------------------------------------*/
#endif
