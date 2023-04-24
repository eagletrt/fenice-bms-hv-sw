/**
 * @file temp.h
 * @brief Wrapper for temperature measurement with the LTC6811
 * 
 * @date Apr 17, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef TEMP_H
#define TEMP_H

#include <inttypes.h>

#include "bms-monitor/volt/volt.h"

typedef uint8_t temperature_t;

/**
 * @brief Start temperature measurement
 * 
 * @param spi The spi configuration structure
 * @param MD The ADC conversion mode
 * @param CHG The GPIO to select for ADC conversion
 * @param gpio The GPIO port
 * @param pin The GPIO pin
 */
void temp_start_measure(SPI_HandleTypeDef * spi,
    VOLT_ADC_MODE MD,
    VOLT_DISCHARGE_CELL CHG,
    GPIO_TypeDef * gpio,
    uint16_t pin);
/**
 * @brief Read cell temperatures and save the values in the 'temps' array
 * 
 * @param spi The SPI configuration structure
 * @param temps An array where the result is stored
 * @param gpio The GPIO port
 * @param pin The GPIO pin
 * @return HAL_StatusTypeDef The status of the operation
 */
HAL_StatusTypeDef temp_read(SPI_HandleTypeDef * spi,
    temperature_t temps[VOLT_CELL_COUNT],
    GPIO_TypeDef * gpio,
    uint16_t pin);

#endif // TEMP_H
