/**
 * @file        soc.c
 * @brief	    This file contains functions and utilities around energy and State of Charge estimation.
 * 
 * @date        May 12, 2021
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

/* Includes ------------------------------------------------------------------*/
#include "soc.h"

#include <stdlib.h>
#include <string.h>

#include "pack.h"
/* Private typedef -----------------------------------------------------------*/
struct soc {
	uint32_t last_sample;
	double joule;
	current_t last_current;
};

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void soc_init(soc_t* handle) {
	*handle = malloc(sizeof(struct soc));
	soc_reset_count(*handle, 0);
}

void soc_load(soc_t handle, uint32_t joule, uint32_t time) {
	handle->joule = joule;
	soc_reset_time(handle, time);
}

void soc_reset_time(soc_t handle, uint32_t time) {
	handle->last_sample = time;
	handle->last_current = 0;
}

void soc_reset_count(soc_t handle, uint32_t time) {
	handle->joule = 0;
	soc_reset_time(handle, time);
}

void soc_sample_current(soc_t handle, current_t current, voltage_t voltage, uint32_t time) {
	// C = ∫I dt = I*Δt
	double coulomb = ((double)(handle->last_current + current) * (time - handle->last_sample)) / 20000.0;

	// J = C*V
	handle->joule += (coulomb * voltage) / 100.0;

	handle->last_current = current;
	handle->last_sample = time;
}

double soc_get_wh(soc_t handle) {
	// 1 J = 1/3600s Wh
	return handle->joule / 3600;
}

double soc_get_joule(soc_t handle) {
	return handle->joule;
}