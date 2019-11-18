/**
 * @file		ltc6813.h
 * @brief		This file contains the functions to communicate with the LTCs
 *
 * @date		Apr 11, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef LTC6813_H_
#define LTC6813_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stm32f4xx_hal.h>
#include "comm/ltc6813_utils.h"
#include "error.h"
#include "fenice_config.h"

/** @brief Basic definition of a LTC6813 */
// TODO: Remove this. Only maintain the error
typedef struct {
	uint8_t address;  //!< The isoSPI bus address
	const bool
		*cell_distribution;  //!< distribution of cells across the registers
	ERROR_STATUS_T error;	//!< Error status for the LTC
} LTC6813_T;

/**
 * @brief rdcv command registers
 * @details As defined in the LTC6813 datasheet, theese are the bits to access
 * 			the six different registers of the LTC
 */
static const uint8_t rdcv_cmd[LTC6813_REG_COUNT] = {
	0b0100,  // A
	0b0110,  // B
	0b1000,  // C
	0b1010,  // D
	0b1001,  // E
	0b1011,  // F
};

// Time to discharge the cell
static const uint16_t dcto[16] = {
	0x00,  // disabled
	0x01,  // 0,5 min
	0x02,  // 1
	0x03,  // 2
	0x04,  // 3-
	0x05,  // 4
	0x06,  // 5
	0x07,  // 10
	0x08,  // 15

	0x09,  // 20
	0x0a,  // 30
	0x0b,  // 40
	0x0c,  // 60

	0x0d,  // 75
	0x0e,  // 90
	0x0f   // 120
};

enum ltc6813_i2c_ctrl {
	I2C_READ = 1,
	I2C_WRITE = 0,
	I2C_START = 0b01100000,
	I2C_STOP = 0b00010000,
	I2C_BLANK = 0b00000000,
	I2C_NO_TRANSMIT = 0b01110000,
	I2C_MASTER_ACK = 0b00000000,
	I2C_MASTER_NACK = 0b00001000,
	I2C_MASTER_NACK_STOP = 0b00001001
};

uint16_t _convert_voltage(uint8_t v_data[]);

uint16_t _convert_temp(uint16_t volt);

void _wakeup_idle(SPI_HandleTypeDef *hspi, bool apply_delay);

void _ltc6813_adcv(SPI_HandleTypeDef *hspi, bool DCP);
void _ltc6813_wrcfg(SPI_HandleTypeDef *hspi, bool start, bool parity);

uint8_t ltc6813_read_voltages(SPI_HandleTypeDef *spi, LTC6813_T *ltc,
							  uint16_t volts[], ERROR_STATUS_T volts_error[],
							  WARNING_T *warning, ERROR_T *error);
uint8_t ltc6813_read_temperatures(SPI_HandleTypeDef *hspi, LTC6813_T *ltc,
								  uint16_t temps[],
								  ERROR_STATUS_T temps_error[], ERROR_T *error);

void ltc6813_wrcomm_i2c(SPI_HandleTypeDef *hspi, uint8_t address, bool read,
						uint8_t data);
void ltc6813_stcomm_i2c(SPI_HandleTypeDef *hspi);
void ltc6813_rdcomm_i2c(SPI_HandleTypeDef *hspi, uint8_t data[8]);

void ltc6813_set_balancing(SPI_HandleTypeDef *hspi, uint8_t *indexes, int dcto);
void ltc6813_wrcfg(SPI_HandleTypeDef *hspi, bool is_a,
				   uint8_t cfgr[LTC6813_COUNT][8]);

#endif /* LTC6813_H_ */
