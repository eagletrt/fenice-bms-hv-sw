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
void soc_init(soc_t handle);

/**
 * @brief Resets integration time to a given timestamp.
 * 
 * @details Useful for resetting count when the battery is turned on
 */
void soc_reset_time(soc_t handle, uint32_t time);

/**
 * @brief   Updates coulomb and joule counting with given data
 */
void soc_sample_current(soc_t handle, current_t current, voltage_t voltage, uint32_t time);

/**
 * @brief Calculate total consumption since last charge
 * 
 * @returns the consumption in Wh
 */
double soc_get_total_consumption(soc_t handle);

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private Macros -----------------------------------------------------------*/
#endif
