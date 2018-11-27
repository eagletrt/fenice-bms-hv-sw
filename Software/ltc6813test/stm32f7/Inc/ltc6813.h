/*
 * ltc6813.h
 *
 *  Created on: 27 nov 2018
 *      Author: Utente
 */

#ifndef LTC6813_H_
#define LTC6813_H_
#include "stm32f7xx_hal.h"

void ltc6813_adcv(uint8_t DCP, SPI_HandleTypeDef *hspi1);
void wakeup_idle(SPI_HandleTypeDef *hspi1);
void ltc6813_rdcv_voltages(uint8_t ic_n, uint16_t cell_voltages[18][2], SPI_HandleTypeDef *hspi1);
void ltc6813_adax(uint8_t chg, SPI_HandleTypeDef *hspi1);
void ltc6813_rdaux(uint8_t ic_n, uint16_t gpio_voltages[9][2],SPI_HandleTypeDef *hspi1);
uint16_t pec15(uint8_t len,uint8_t data[],uint16_t crcTable[] );
uint16_t convert_voltage(uint8_t v_data[]);
void ltc6813_adstat( SPI_HandleTypeDef *hspi1);
void ltc6804_rdstat(SPI_HandleTypeDef *hspi1,uint16_t stat_voltages[18][2] );
void ltc6813_clrcell(SPI_HandleTypeDef *hspi1);
void ltc6813_clraux(SPI_HandleTypeDef *hspi1);
void ltc6813_DischargeCell_Enable(SPI_HandleTypeDef *hspi1);
#endif /* LTC6813_H_ */
