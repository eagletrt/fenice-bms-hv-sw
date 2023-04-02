/**
 * @file internal_voltage.h
 * @brief Functions to manage internal voltages in the pack
 * 
 * @date Apr 01, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef INTERNAL_VOLTAGE_H
#define INTERNAL_VOLTAGE_H


#define CONV_COEFF 140.86f  //(10MOhm + 71.5kOhm) / 71.5kOhm
#define INTERNAL_VOLTAGE_ADC_TO_VOLTAGE(v) CONV_COEFF * (v)

/** @brief Init internal voltage measurement */
void internal_voltage_init();
/**
 * @brief Measure internal voltages
 * @return HAL_StatusTypeDef The status of the operation
 */
HAL_StatusTypeDef internal_voltage_measure();

/**
 * @brief Get TS voltage
 * @return float TS voltage value
 */
float internal_voltage_get_vts();
/**
 * @brief Get battery voltage
 * @return float Battery voltage value
 */
float internal_voltage_get_vbat();
/**
 * @brief Get shunt voltage
 * @return float Shunt voltage value
 */
float internal_voltage_get_shunt();
/**
 * @brief Get TS- voltage
 * @return float TS- voltage value
 */
float internal_voltage_get_tsn();

#endif // INTERNAL_VOLTAGE_H