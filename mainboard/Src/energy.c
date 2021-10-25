/**
 * @file	energy.h
 * @brief	File containing energy management functions for SoC management.
 *
 * @date	Sep 25, 2021
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "energy.h"

#include "soc.h"

#define ENERGY_VERSION 0x5555
#define ENERGY_ADDR    0x30

#define ENERGY_WRITE_INTERVAL 5000

typedef struct {
    float total_joule;
    float charge_joule;
} soc_params;
soc_params soc_params_default = {0, 0};

soc_t soc_total;        // Total energy
soc_t soc_last_charge;  // Energy since last charge
config_t soc_config;    // Config data for config.h

void energy_init() {
    // Reset the counts
    soc_init(&soc_total);
    soc_init(&soc_last_charge);

    // Try to load counts from memory. If errors, revert to default params
    config_init(&soc_config, ENERGY_ADDR, ENERGY_VERSION, &soc_params_default, sizeof(soc_params));

    // Save the loaded values in soc instances
    soc_load(soc_total, ((soc_params *)config_get(soc_config))->total_joule, HAL_GetTick());
    soc_load(soc_last_charge, ((soc_params *)config_get(soc_config))->charge_joule, HAL_GetTick());
}

void energy_sample_current(uint32_t timestamp) {
    soc_params params = *(soc_params *)config_get(soc_config);

    // Sample current values for SoC calculation
    soc_sample_current(soc_total, current_get_current(), pack_get_int_voltage(), timestamp);
    soc_sample_current(soc_last_charge, current_get_current(), pack_get_int_voltage(), timestamp);

    // Update newly-calculated energy values to the energy structure
    params.charge_joule = soc_get_joule(soc_last_charge);
    params.total_joule  = soc_get_joule(soc_total);

    // Save energy values to EEPROM
    config_set(soc_config, &params);
    // TODO: make async writes (don't block this function)
    config_write(soc_config);
}

void energy_reset_soc() {
    soc_reset_count(soc_last_charge, HAL_GetTick());
}

float energy_get_soc() {
    // Compute Wh from Joule on the fly
    return soc_get_wh(soc_last_charge) / (PACK_ENERGY_NOMINAL / 10.0) * 100;
}

float energy_get_energy_total() {
    return soc_get_wh(soc_total);
}

float energy_get_energy_last_charge() {
    return soc_get_wh(soc_last_charge);
}