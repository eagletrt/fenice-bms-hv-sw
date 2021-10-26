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
    float energy;
    float last_power;
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

void energy_load(energy_t handle, float joule, uint32_t time) {
    handle->energy = joule;
    energy_reset_time(handle, time);
}

void energy_reset_time(energy_t handle, uint32_t time) {
    handle->last_sample = time;
    handle->last_power  = 0;
}

void energy_reset_count(energy_t handle, uint32_t time) {
    handle->energy = 0;
    energy_reset_time(handle, time);
}

void energy_sample_energy(energy_t handle, float power, uint32_t time) {
    // Use trapezoidal rule to approximate an integral
    // Energy = ∫Power dt = (1/2 * (P[i-1] + P[i])) * Δt
    handle->energy += ((handle->last_power + power) / 2) * ((time - handle->last_sample) / 1000.0f);

    handle->last_power  = power;
    handle->last_sample = time;
}

float energy_get_wh(energy_t handle) {
    // 1 J = 1/3600s Wh
    return handle->energy / 3600;
}

float energy_get_joule(energy_t handle) {
    return handle->energy;
}