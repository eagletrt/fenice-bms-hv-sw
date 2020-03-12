/**
 * @file		ltc6813_utils.c
 * @brief		This file contains utilities for improving LTC6813
 * 				communications
 *
 * @date		Nov 16, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "comm/ltc6813_utils.h"

#include <math.h>

#include "main.h"

/**
 * @brief		Polls all the registers of the LTC6813 and updates the cell
 * array
 * @details	It executes multiple rdcv requests to the LTCs and saves the values
 * 					in the voltage variable of the CELL_Ts.
 *
 * 					1     CMD0    8     CMD1      16      32
 * 					|- - - - - - -|- - - - - - - -|- ... -|
 * 					1 0 0 0 0 0 0 0 0 0 0 0 X X X X  PEC
 * 					 Address |             |  Reg  |
 * 					  (BRD)
 *
 * @param		spi		The SPI configuration structure
 * @param		volts	The array of voltages
 */
uint8_t ltc6813_read_voltages(SPI_HandleTypeDef *spi, uint16_t volts[]) {
	uint8_t cmd[4];
	uint16_t cmd_pec;
	uint8_t data[8 * LTC6813_COUNT];

	cmd[0] = 0;	 // Broadcast

	uint8_t count = 0;	// counts the cells
	for (uint8_t reg = 0; reg < LTC6813_REG_COUNT; reg++) {
		cmd[1] = (uint8_t)rdcv_cmd[reg];
		cmd_pec = ltc6813_pec15(2, cmd);
		cmd[2] = (uint8_t)(cmd_pec >> 8);
		cmd[3] = (uint8_t)(cmd_pec);

		ltc6813_wakeup_idle(spi, false);
		ltc6813_enable_cs(spi, CS_LTC_GPIO_Port, CS_LTC_Pin);

		if (HAL_SPI_Transmit(spi, cmd, 4, 100) != HAL_OK) {
			// goto End;
		}
		HAL_SPI_Receive(spi, data, 8 * LTC6813_COUNT, 100);

		ltc6813_disable_cs(spi, CS_LTC_GPIO_Port, CS_LTC_Pin);

#if LTC6813_EMU == 1
		// Writes 3.6v to each cell

		uint8_t emu_i;
		for (emu_i = 0; emu_i < LTC6813_REG_CELL_COUNT * 2; emu_i++) {
			// 36000
			data[emu_i] = 0b10100000;
			data[++emu_i] = 0b10001100;
		}
		uint16_t emu_pec = ltc6813_pec15(6, data);
		data[6] = (uint8_t)(emu_pec >> 8);
		data[7] = (uint8_t)emu_pec;
#endif

		for (uint8_t ltc = 0; ltc < LTC6813_COUNT; ltc++) {
			if (ltc6813_pec15(6, data) == (uint16_t)(data[6] * 256 + data[7])) {
				error_unset(ERROR_LTC_PEC_ERROR, ltc);

				// For every cell in the register
				for (uint8_t cell = 0; cell < LTC6813_REG_CELL_COUNT; cell++) {
					// offset by register (3 slots) + offset by ltc (18 slots) + cell
					uint16_t index = (reg * LTC6813_REG_CELL_COUNT) + (ltc * LTC6813_CELL_COUNT) + cell;

					// (size of value) * cell
					volts[index] = ltc6813_convert_voltage(&data[2 * cell]);

					ltc6813_check_voltage(volts, index);
					count++;
				}
			} else {
				error_set(ERROR_LTC_PEC_ERROR, ltc, HAL_GetTick());
			}
		}
	}

	return count;
}

void ltc6813_temp_set_register(SPI_HandleTypeDef *hspi, uint8_t address,
							   uint8_t reg) {
	uint8_t comm[8] = {0};

	comm[0] = I2C_START | (address >> 3);
	comm[1] = (address << 5) | (0 << 4) | I2C_MASTER_ACK;

	comm[2] = I2C_BLANK | (reg >> 4);
	comm[3] = (reg << 4) | I2C_MASTER_ACK;

	comm[4] = I2C_START | (address >> 3);
	comm[5] = (address << 5) | (1 << 4) | I2C_MASTER_ACK;

	uint16_t pec = ltc6813_pec15(6, comm);
	comm[6] = (uint8_t)(pec >> 8);
	comm[7] = (uint8_t)(pec);

	ltc6813_wrcomm_i2c(hspi, comm);
}

void ltc6813_temp_get(SPI_HandleTypeDef *hspi, uint8_t address) {
	uint8_t comm[8] = {0};

	comm[0] = I2C_BLANK | (0xFF >> 4);
	comm[1] = (0xF << 4) | I2C_MASTER_ACK;

	comm[2] = I2C_BLANK | (0xFF >> 4);
	comm[3] = (uint8_t)(0xFF << 4) | I2C_MASTER_ACK;

	comm[4] = I2C_BLANK | (0xFF >> 4);
	comm[5] = (uint8_t)(0xFF << 4) | I2C_MASTER_NACK_STOP;

	uint16_t pec = ltc6813_pec15(6, comm);
	comm[6] = (uint8_t)(pec >> 8);
	comm[7] = (uint8_t)(pec);

	// HAL_Delay(1);
	ltc6813_wrcomm_i2c(hspi, comm);
}

/**
 * @brief		This function is used to fetch the temperatures.
 *
 * @param		hspi		The SPI configuration structure
 * @param		ltc			The array of LTC6813 configurations
 * @param		temps		The array of temperatures
 */
void ltc6813_read_temperatures(SPI_HandleTypeDef *hspi, uint8_t max[2],
							   uint8_t min[2]) {
	uint8_t recv[8 * LTC6813_COUNT] = {0};

	ltc6813_wakeup_idle(hspi, false);

	ltc6813_temp_set_register(hspi, LTC6813_TEMP_ADDRESS, 0xFF);
	ltc6813_stcomm_i2c(hspi, 3);

	///// read
	ltc6813_temp_get(hspi, LTC6813_TEMP_ADDRESS);
	ltc6813_stcomm_i2c(hspi, 3);

	ltc6813_rdcomm_i2c(hspi, recv);

	for (uint8_t ltc = 0; ltc < LTC6813_COUNT; ltc++) {
		// For each ltc we skip 8 bits (6 data + 2 pec);
		uint8_t base_index = ltc * (LTC6813_COUNT + 2);

		uint8_t d0 = (recv[0 + base_index] << 4) | (recv[1 + base_index] >> 4);
		uint8_t d1 = (recv[2 + base_index] << 4) | (recv[3 + base_index] >> 4);
		uint8_t d2 = (recv[4 + base_index] << 4) | (recv[5 + base_index] >> 4);

		max[ltc * 2] = d0 >> 2;
		max[1 + ltc * 2] = (d0 & 0b00000011) << 4 | d1 >> 4;

		min[ltc * 2] = (d1 & 0b00001111) << 2 | d2 >> 6;
		min[1 + ltc * 2] = d2 & 0b00111111;
	}
}

void ltc6813_read_all_temps(SPI_HandleTypeDef *hspi, uint8_t *temps) {
	uint8_t count = 0;
	ltc6813_wakeup_idle(hspi, false);

	for (uint8_t sens = 0; sens < ceil((float)TEMP_SENSOR_COUNT / 4); sens++) {
		uint8_t recv[8 * LTC6813_COUNT] = {0};

		ltc6813_temp_set_register(hspi, LTC6813_TEMP_ADDRESS, sens);
		ltc6813_stcomm_i2c(hspi, 3);

		///// read
		ltc6813_temp_get(hspi, LTC6813_TEMP_ADDRESS);
		ltc6813_stcomm_i2c(hspi, 3);

		ltc6813_rdcomm_i2c(hspi, recv);

		for (uint8_t ltc = 0; ltc < LTC6813_COUNT; ltc++) {
			uint8_t base_index = ltc * (LTC6813_COUNT + 2);

			uint8_t d0 = (recv[0 + base_index] << 4) | (recv[1 + base_index] >> 4);
			uint8_t d1 = (recv[2 + base_index] << 4) | (recv[3 + base_index] >> 4);
			uint8_t d2 = (recv[4 + base_index] << 4) | (recv[5 + base_index] >> 4);

			temps[count++] = d0 >> 2;
			temps[count++] = (d0 & 0b00000011) << 4 | d1 >> 4;
			temps[count++] = (d1 & 0b00001111) << 2 | d2 >> 6;
			temps[count++] = d2 & 0b00111111;
		}
	}
}

/**
 * @brief		Checks that voltage is between its thresholds.
 *
 * @param		volts		The voltage
 * @param		error		The error return code
 */
void ltc6813_check_voltage(uint16_t *volts, uint8_t index) {
	if (volts[index] < CELL_MIN_VOLTAGE) {
		error_set(ERROR_CELL_UNDER_VOLTAGE, index, HAL_GetTick());
	} else {
		error_unset(ERROR_CELL_UNDER_VOLTAGE, index);
	}

	if (volts[index] > CELL_MAX_VOLTAGE) {
		error_set(ERROR_CELL_OVER_VOLTAGE, index, HAL_GetTick());
	} else {
		error_unset(ERROR_CELL_OVER_VOLTAGE, index);
	}
}

/**
 * @brief		Checks that temperature is between its thresholds.
 *
 * @param		temp		The temperature
 * @param		error		The error return code
 */
void ltc6813_check_temperature(uint16_t *temps, uint8_t index) {
	if (temps[index] >= CELL_MAX_TEMPERATURE) {
		error_set(ERROR_CELL_OVER_TEMPERATURE, index, HAL_GetTick());
	} else {
		error_unset(ERROR_CELL_OVER_TEMPERATURE, index);
	}
}

void ltc6813_set_dcc(uint8_t indexes[], uint8_t cfgar[8], uint8_t cfgbr[8]) {
	for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
		if (indexes[i] < 8) {
			cfgar[4] += dcc[indexes[i]];
		} else if (indexes[i] >= 8 && indexes[i] < 12) {
			cfgar[5] += dcc[indexes[i]];
		} else if (indexes[i] >= 12 && indexes[i] < 16) {
			cfgbr[0] += dcc[indexes[i]];
		} else if (indexes[i] >= 16 && indexes[i] < 18) {
			cfgbr[1] += dcc[indexes[i]];
		}

		uint16_t pec = ltc6813_pec15(6, cfgar);
		cfgar[6] = (uint8_t)(pec >> 8);
		cfgar[7] = (uint8_t)(pec);

		pec = ltc6813_pec15(6, cfgbr);
		cfgbr[6] = (uint8_t)(pec >> 8);
		cfgbr[7] = (uint8_t)(pec);
	}
}

void ltc6813_set_balancing(SPI_HandleTypeDef *hspi, uint8_t *indexes,
						   int dcto) {
	uint8_t cfgar[LTC6813_COUNT][8] = {0};
	uint8_t cfgbr[LTC6813_COUNT][8] = {0};

	for (uint8_t i = 0; i < LTC6813_COUNT; i++) {
		// cfgbr[i][1] += 0b00001000; // set DTMEN
		cfgar[i][0] += 0b00000010;	// set DTEN
		cfgar[i][5] += dcto << 4;	// Set timer

		// For each LTC we set the correct cfgr
		ltc6813_set_dcc(indexes, cfgar[i], cfgbr[i]);
	}
	ltc6813_wakeup_idle(hspi, true);

	ltc6813_wrcfg(hspi, true, cfgar);
	ltc6813_wrcfg(hspi, false, cfgbr);
}

/**
 * @brief	This function is used to convert the 2 byte raw data from the
 * 				LTC68xx to a 16 bit unsigned integer
 *
 * @param 	v_data	Raw data bytes
 *
 * @retval	Voltage [mV]
 */
uint16_t ltc6813_convert_voltage(uint8_t v_data[]) {
	return v_data[0] + (v_data[1] << 8);
}

/**
 * @brief		This function converts a voltage data from the zener sensor
 * 					to a temperature
 *
 * @param		volt	Voltage [mV]
 *
 * @retval	Temperature [C°]
 */
uint8_t ltc6813_convert_temp(uint8_t temp) { return temp; }