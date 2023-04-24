/**
 * @file temp.c
 * @brief Wrapper for temperature measurement with the LTC6811
 * 
 * @date Apr 17, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "temp.h"

void temp_start_measure(SPI_HandleTypeDef * spi,
    VOLT_ADC_MODE MD,
    VOLT_DISCHARGE_CELL CHG,
    GPIO_TypeDef * gpio,
    uint16_t pin) {
    ltc6811_adax(spi, MD, CHG, gpio, pin);
}

// TODO: Check, test and fix temp_read
HAL_StatusTypeDef temp_read(SPI_HandleTypeDef * spi,
    temperature_t temps[VOLT_CELL_COUNT],
    GPIO_TypeDef * gpio,
    uint16_t pin) {
    ltc6811_read_auxiliary(spi, temps, gpio, pin);
}
