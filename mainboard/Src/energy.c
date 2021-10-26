/**
 * @file        energy.c
 * @brief	    This file contains functions and utilities around energy and State of Charge estimation.
 * 
 * @date        May 12, 2021
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

/* Includes ------------------------------------------------------------------*/
#include "energy.h"

#include <stdlib.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
struct energy {
    uint32_t last_sample;
    float joule;
    float last_watt;
};

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

void energy_init(energy_t *handle) {
    *handle = malloc(sizeof(struct energy));
    energy_reset_count(*handle, 0);
}

void energy_deinit(energy_t *handle) {
    free(*handle);
}

void energy_load(energy_t handle, uint32_t joule, uint32_t time) {
    handle->joule = joule;
    energy_reset_time(handle, time);
}

void energy_reset_time(energy_t handle, uint32_t time) {
    handle->last_sample = time;
    handle->last_watt   = 0;
}

void energy_reset_count(energy_t handle, uint32_t time) {
    handle->joule = 0;
    energy_reset_time(handle, time);
}

void energy_sample_current(energy_t handle, current_t current, voltage_t voltage, uint32_t time) {
    float watt = current * (voltage / 10.0f);

    // Use trapezoidal rule to approximate an integral
    // Energy = ∫Power dt = (1/2 * (P[i-1] + P[i])) * Δt
    handle->joule += ((handle->last_watt + watt) / 2) * ((time - handle->last_sample) / 1000.0f);

    handle->last_watt   = watt;
    handle->last_sample = time;
}

float energy_get_wh(energy_t handle) {
    // 1 J = 1/3600s Wh
    return handle->joule / 3600;
}

float energy_get_joule(energy_t handle) {
    return handle->joule;
}