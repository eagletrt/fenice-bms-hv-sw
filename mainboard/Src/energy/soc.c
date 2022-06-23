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

energy_t energy_total;        // Total energy

void soc_init(float lower_cell_voltage) {
    // Reset the counts
    energy_init(&energy_total);

    //assume that at startup the current is 0

    energy_set_count(&energy_total, 4*PACK_CELL_COUNT*soc_volt_to_energy(lower_cell_voltage));
    energy_set_time(&energy_total, HAL_GetTick());
}

void soc_sample_energy(uint32_t timestamp) {
    // Sample current values for SoC calculation
    energy_sample_energy(&energy_total, current_get_current() * voltage_get_vbat_adc(), timestamp);
}

void soc_reset_soc() {
    energy_set_time(&energy_total, 0);
}

float soc_get_soc() {
    // Compute state of charge based on nominal energy
    return energy_get_wh(energy_total) / (PACK_ENERGY_NOMINAL) * 100;
}

float soc_get_energy_total() {
    return energy_get_wh(energy_total);
}

float soc_volt_to_capacity(float volt) {
    return 0.0862 * volt * volt * volt - 1.4260 * volt * volt + 6.0088 * volt - 6.551;
}

float soc_volt_to_energy(float volt) {
    return soc_volt_to_capacity(volt)*volt;
}