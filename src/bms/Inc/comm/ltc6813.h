/**
 * @file		ltc6813.h
 * @brief		This file contains the functions to communicate with the LTCs
 *
 * @date		Apr 11, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef LTC6813_H_
#define LTC6813_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stm32f4xx_hal.h>
#include "comm/ltc6813_utils.h"
#include "error.h"
#include "fenice_config.h"

void ltc6813_wakeup_idle(SPI_HandleTypeDef *hspi, bool apply_delay);

void _ltc6813_adcv(SPI_HandleTypeDef *hspi, bool DCP);
void _ltc6813_wrcfg(SPI_HandleTypeDef *hspi, bool start, bool parity);

void ltc6813_wrcomm_i2c(SPI_HandleTypeDef *hspi, uint8_t address, bool read,
						uint8_t data);
void ltc6813_stcomm_i2c(SPI_HandleTypeDef *hspi, uint8_t length);
void ltc6813_rdcomm_i2c(SPI_HandleTypeDef *hspi, uint8_t data[8]);

void ltc6813_set_balancing(SPI_HandleTypeDef *hspi, uint8_t *indexes, int dcto);
void ltc6813_wrcfg(SPI_HandleTypeDef *hspi, bool is_a,
				   uint8_t cfgr[LTC6813_COUNT][8]);

#endif /* LTC6813_H_ */
