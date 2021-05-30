/**
 * @file        soc.h
 * @brief	    This file contains functions and utilities around energy and State of Charge estimation.
 * 
 * @date        May 12, 2021
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _SOC_H_
#define _SOC_H_

/* Includes ------------------------------------------------------------------*/
#include <inttypes.h>

#include "fenice_config.h"
/* Exported types ------------------------------------------------------------*/
typedef struct soc* soc_t;
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initializes soc instance
 */
void soc_init(soc_t* handle);

/**
 * @brief Loads saved values and resets timer
 */
void soc_load(soc_t handle, uint32_t joule, uint32_t time);

/**
 * @brief Resets integration time to a given timestamp.
 * 
 * @details Useful for resetting count when the battery is turned on
 */
void soc_reset_time(soc_t handle, uint32_t time);

/**
 * @brief Resets all recorded values along with integration timestamp
 * 
 * @details Used when battery is fully charged or when consumption restricted to a certain period is wanted. 
 */
void soc_reset_count(soc_t handle, uint32_t time);

/**
 * @brief   Updates coulomb and joule counting with given data
 */
void soc_sample_current(soc_t handle, current_t current, voltage_t voltage, uint32_t time);

/**
 * @brief Calculate total consumption since last charge
 * 
 * @returns the consumption in Wh
 */
double soc_get_wh(soc_t handle);
double soc_get_joule(soc_t handle);

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private Macros -----------------------------------------------------------*/
#endif
