/**
 * @file        soc.c
 * @brief	    This file contains functions and utilities around energy and State of Charge estimation.
 * 
 * @date        May 12, 2021
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

/* Includes ------------------------------------------------------------------*/
#include "soc.h"

#include "pack.h"
/* Private typedef -----------------------------------------------------------*/
struct soc {
	uint32_t last_sample;
	uint32_t coulomb;
	uint32_t joule;
};

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void soc_init(soc_t handle) {
	handle->last_sample = 0;
	handle->coulomb = 0;
	handle->joule = 0;
}

void soc_reset_time(soc_t handle, uint32_t time) {
	handle->last_sample = time;
}

void soc_sample_current(soc_t handle, current_t current, voltage_t voltage, uint32_t time) {
	// C = ∫I dt = I*Δt
	handle->coulomb += current * (time - handle->last_sample);

	// J = C*V
	handle->joule += handle->coulomb * voltage;

	handle->last_sample = time;
}

double soc_get_total_consumption(soc_t handle) {
	// 1 J = 1/3600s Wh
	return (double)handle->joule / 3600000;
}
