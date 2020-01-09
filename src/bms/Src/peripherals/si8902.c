/**
 * @file		si8902.c
 * @brief		This file contains the functions to read bus and total pack
 * voltages
 *
 * @date		Gen 09, 2020
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "peripherals/si8902.h"

#include "spi.h"

void si8902_read_voltages(SPI_HandleTypeDef *hspi, uint16_t ain[3]) {
	spi_enable_cs(hspi, CS_ADC_GPIO_Port, LTC_CS_Pin);

	for (uint8_t i = 0; i < 2; i++) {
		HAL_SPI_Transmit(hspi, (uint8_t *)(cnfg_0 | (i << 4)), 1, 10);

		// Wait at least 8us
		HAL_Delay(1);

		uint8_t recv[2];
		HAL_SPI_Receive(hspi, recv, 2, 10);
		ain[i] = si8902_convert_voltage(recv);
	}

	spi_disable_cs(hspi, CS_ADC_GPIO_Port, LTC_CS_Pin);
}

uint16_t si8902_convert_voltage(uint8_t adc_data[2]) {
	// MSB | LSB
	return ((adc_data[0] & 0b00001111) << 6) | (adc_data[1] & 0b01111110) >> 1;
}