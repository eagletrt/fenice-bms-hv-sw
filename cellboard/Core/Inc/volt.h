/**
 * @file volt.h
 * @brief Voltage measurement functions
 *
 * @date Jul 17, 2021
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef VOLT_H
#define VOLT_H

#include "cellboard_config.h"
#include "peripherals/ltc6813_utils.h"

#define CONVERT_VALUE_TO_VOLTAGE(x) ((float)(x) / 10000.f)
#define CONVERT_VOLTAGE_TO_VALUE(x) ((x) * 10000.f)

extern voltage_t voltages[CELLBOARD_CELL_COUNT];

/** @brief Initialize voltage measurement */
void volt_init();

/** @brief Start voltage measure */
void volt_start_measure();
/** @brief Read voltages */
void volt_read();

void volt_start_open_wire_check(uint8_t status);
void volt_read_open_wire(uint8_t status);
void volt_open_wire_check();


voltage_t volt_get_min();
voltage_t volt_get_max();
float volt_get_avg();

/**
 * @brief Returns the lower-voltage cell
 * 
 * @return uint16_t The index of the lower-voltage cell
 */
uint16_t volt_get_min_index();
/**
 * @brief Get a pointer to the array of voltages values of the cells
 * 
 * @return voltage_t * The pointer to the array of voltages
 */
voltage_t * volt_get_volts();

#endif