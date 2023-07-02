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
#include "max22530.h"

#define INTERNAL_VOLTAGE_DIVIDER_RATIO 0.003
#define CONVERT_VALUE_TO_INTERNAL_ADC_VOLTAGE(x) MAX22530_CONV_VALUE_TO_VOLTAGE(x)
#define CONVERT_VALUE_TO_INTERNAL_VOLTAGE(x) (CONVERT_VALUE_TO_INTERNAL_ADC_VOLTAGE(x) / INTERNAL_VOLTAGE_DIVIDER_RATIO)

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
 * @return uint16_t The TS+ voltage value
 */
uint16_t internal_voltage_get_tsp();
/**
 * @brief Get the TS- voltage
 * 
 * @return uint16_t The TS- voltage value
 */
uint16_t internal_voltage_get_tsn();
/**
 * @brief Get the shunt voltage
 * 
 * @return uint16_t The shunt voltage value
 */
uint16_t internal_voltage_get_shunt();
/**
 * @brief Get the battery voltage
 * 
 * @return uint16_t The battery voltage value
 */
uint16_t internal_voltage_get_bat();


#endif // INTERNAL_VOLTAGE_H