/**
 * @file    soc.h
 * @brief   State of Charge estimation and management. Based on the energy subsystem
 *
 * @date    Sep 25, 2021
 *
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "energy/soc.h"

#include "energy/energy.h"
#include "pack/voltage.h"

#define ENERGY_VERSION 0x5555
#define ENERGY_ADDR    0x30

#define ENERGY_WRITE_INTERVAL 5000

typedef struct {
    float total_joule;
    float charge_joule;
} soc_params;
soc_params soc_params_default = {0, 0};

energy_t energy_total;        // Total energy
energy_t energy_last_charge;  // Energy since last charge
config_t soc_config;          // Config data for config.h

void soc_init() {
    // Reset the counts
    energy_init(&energy_total);
    energy_init(&energy_last_charge);

    // Try to load counts from memory. If errors, revert to default params
    config_init(&soc_config, ENERGY_ADDR, ENERGY_VERSION, &soc_params_default, sizeof(soc_params));

    // Save the loaded values in soc instances
    energy_set_count(&energy_total, ((soc_params *)config_get(soc_config))->total_joule);
    energy_set_time(&energy_total, HAL_GetTick());

    energy_set_count(&energy_last_charge, ((soc_params *)config_get(soc_config))->charge_joule);
    energy_set_time(&energy_last_charge, HAL_GetTick());
}

void soc_sample_energy(uint32_t timestamp) {
    soc_params params = *(soc_params *)config_get(soc_config);

    // Sample current values for SoC calculation
    energy_sample_energy(&energy_total, (current_get_current() * voltage_get_internal()) / 10.0f, timestamp);
    energy_sample_energy(&energy_last_charge, (current_get_current() * voltage_get_internal()) / 10.0f, timestamp);

    // Update newly-calculated energy values to the energy structure
    params.charge_joule = energy_get_joule(energy_last_charge);
    params.total_joule  = energy_get_joule(energy_total);

    // Save energy values to EEPROM
    config_set(soc_config, &params);
    // TODO: make async writes (don't block this function)
    config_write(soc_config);
}

void soc_reset_soc() {
    energy_set_count(&energy_last_charge, 0);
    energy_set_time(&energy_last_charge, 0);
}

float soc_get_soc() {
    // Compute state of charge based on nominal energy
    return energy_get_wh(energy_last_charge) / (PACK_ENERGY_NOMINAL / 10.0) * 100;
}

float soc_get_energy_total() {
    return energy_get_wh(energy_total);
}

float soc_get_energy_last_charge() {
    return energy_get_wh(energy_last_charge);
}