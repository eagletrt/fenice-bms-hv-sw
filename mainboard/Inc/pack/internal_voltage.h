/**
 * @file internal_voltage.h
 * @brief Interal voltages management functions
 *
 * @date Apr 11, 2019
 * 
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef INTERNAL_VOLTAGE_H
#define INTERNAL_VOLTAGE_H

#include "stm32f4xx_hal.h"
#include "../../fenice_config.h"

/** @brief Initializes the adc and internal voltages */
void internal_voltage_init();

/**
 * @brief Polls for voltages
 * 
 * @return HAL_StatusTypeDef The status of the spi communication
 */
HAL_StatusTypeDef internal_voltage_measure();

/**
 * @brief Get the TS+ voltage
 * 
 * @return float The TS+ voltage value
 */
float internal_voltage_get_tsp();
/**
 * @brief Get the TS- voltage
 * 
 * @return float The TS- voltage value
 */
float internal_voltage_get_tsn();
/**
 * @brief Get the shunt voltage
 * 
 * @return float The shunt voltage value
 */
float internal_voltage_get_shunt();
/**
 * @brief Get the battery voltage
 * 
 * @return float The battery voltage value
 */
float internal_voltage_get_bat();


#endif // INTERNAL_VOLTAGE_H