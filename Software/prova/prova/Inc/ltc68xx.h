/*
 * ltc68xx.h
 *
 *  Created on: 14 nov 2018
 *      Author: Utente
 */

#ifndef LTC68XX_H_
#define LTC68XX_H_
#include "stm32f4xx_hal.h"

uint16_t pec15(uint8_t len,uint8_t data[], uint16_t crcTable[]);
uint16_t convert_voltage(uint8_t v_data[]);
uint16_t convert_temp(uint16_t volt);
void wakeup_idle(SPI_HandleTypeDef *hspi1);
void ltc6804_rdcv_temp(uint8_t ic_n, uint8_t parity, uint16_t cell_temp[108][2],	SPI_HandleTypeDef *hspi1);
void ltc6804_rdcv_voltages(uint8_t ic_n, uint16_t cell_voltages[108][2],	SPI_HandleTypeDef *hspi1);
void ltc6804_command_temperatures(uint8_t start, uint8_t parity, SPI_HandleTypeDef *hspi1);
void ltc6804_adcv(uint8_t DCP, SPI_HandleTypeDef *hspi1);


#endif /* LTC68XX_H_ */
