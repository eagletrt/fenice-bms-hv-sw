/**
 * @file		ltc6813_utils.c
 * @brief		This file contains utilities for improving LTC6813
 * 				communications
 *
 * @date		Nov 16, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "comm/ltc6813_utils.h"

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
 * @param		ltc		The array of LTC6813 configurations
 * @param		volts	The array of voltages
 * @param		error	The error return value
 */
uint8_t ltc6813_read_voltages(SPI_HandleTypeDef *spi, LTC6813_T ltc[],
							  uint16_t volts[], ERROR_STATUS_T volts_error[],
							  warning_t *warning, error_t *error) {
	uint8_t cmd[4];
	uint16_t cmd_pec;
	uint8_t data[8];

	cmd[0] = 0;  // Broadcast

	uint8_t count = 0;  // volts[] index
	for (uint8_t reg = 0; reg < LTC6813_REG_COUNT; reg++) {
		cmd[1] = (uint8_t)rdcv_cmd[reg];
		cmd_pec = ltc6813_pec15(2, cmd);
		cmd[2] = (uint8_t)(cmd_pec >> 8);
		cmd[3] = (uint8_t)(cmd_pec);

		ltc6813_wakeup_idle(spi, false);

		HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
		if (HAL_SPI_Transmit(spi, cmd, 4, 100) != HAL_OK) {
			// goto End;
		}
		HAL_SPI_Receive(spi, data, 8 * LTC6813_COUNT, 100);
		HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);

#if LTC6813_EMU > 0
		// Writes 3.6v to each cell

		uint8_t emu_i;
		for (emu_i = 0; emu_i < LTC6813_REG_CELL_COUNT * 2; emu_i++) {
			// 36000
			data[emu_i] = 0b10100000;
			data[++emu_i] = 0b10001100;
		}
		uint16_t emu_pec = _pec15(6, data);
		data[6] = (uint8_t)(emu_pec >> 8);
		data[7] = (uint8_t)emu_pec;
#endif

		if (ltc6813_pec15(6, data) == (uint16_t)(data[6] * 256 + data[7])) {
			error_unset(ERROR_LTC_PEC_ERROR, &ltc->error);

			// For every cell in the register
			for (uint8_t cell = 0; cell < LTC6813_REG_CELL_COUNT; cell++) {
				volts[count] = ltc6813_convert_voltage(&data[2 * cell]);

				ltc6813_check_voltage(volts[count], &volts_error[count],
									  warning, error);
				ER_CHK(error);
				count++;
			}
		} else {
			error_set(ERROR_LTC_PEC_ERROR, &ltc->error, HAL_GetTick());
		}

		*error = error_check_fatal(&ltc->error, HAL_GetTick());
		ER_CHK(error);
	}

End:;
	return count;
}

/**
 * @brief		This function is used to fetch the temperatures.
 * @details	The workings of this function are very similar to read_voltages,
 * 					the main difference to it is the presence of the even
 * 					parameter. Refer to the ltc6813_read_voltages comment for
 * 					the actual messages.
 *
 * @param		hspi		The SPI configuration structure
 * @param		even		indicates which set of cells is currently being
 * 									balanced: false for odd and true for even
 * cells
 * @param		ltc			The array of LTC6813 configurations
 * @param		temps		The array of temperatures
 * @param		error		The error return value
 */
uint8_t ltc6813_read_temperatures(SPI_HandleTypeDef *hspi, LTC6813_T *ltc,
								  uint16_t temps[],
								  ERROR_STATUS_T temps_error[],
								  error_t *error) {
	uint8_t cmd[4];
	uint16_t cmd_pec;
	uint8_t data[8];

	cmd[0] = (uint8_t)0x80 + ltc->address;

	uint8_t count = 0;
	for (uint8_t reg = 0; reg < LTC6813_REG_COUNT; reg++) {
		cmd[1] = (uint8_t)rdcv_cmd[reg];
		cmd_pec = ltc6813_pec15(2, cmd);
		cmd[2] = (uint8_t)(cmd_pec >> 8);
		cmd[3] = (uint8_t)(cmd_pec);

		ltc6813_wakeup_idle(hspi, false);

		HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
		HAL_Delay(1);
		HAL_SPI_Transmit(hspi, cmd, 4, 100);

		HAL_SPI_Receive(hspi, data, 8, 100);
		HAL_Delay(1);
		HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);

#if LTC6813_EMU > 0
		// Writes 0.9292v (18°C) to each sensor

		for (uint8_t emu_i = 0; emu_i < LTC6813_REG_CELL_COUNT * 2; emu_i++) {
			// 9292
			data[emu_i] = 0b00100100;
			data[++emu_i] = 0b01001100;
		}
		uint16_t emu_pec = _pec15(6, data);
		data[6] = (uint8_t)(emu_pec >> 8);
		data[7] = (uint8_t)emu_pec;
#endif

		if (ltc6813_pec15(6, data) == (uint16_t)(data[6] * 256 + data[7])) {
			error_unset(ERROR_LTC_PEC_ERROR, &ltc->error);

			// Counts the cell inside the register
			for (uint8_t cell = 0; cell < LTC6813_REG_CELL_COUNT; cell++) {
				uint16_t temp = ltc6813_convert_temp(
					ltc6813_convert_voltage(&data[2 * cell]));

				if (temp > 0) {
					temps[count] = temp;

					ltc6813_check_temperature(temps[count], &temps_error[count],
											  error);
					ER_CHK(error);
				}

				count++;
			}
		} else {
			error_set(ERROR_LTC_PEC_ERROR, &ltc->error, HAL_GetTick());
		}
	}

	*error = error_check_fatal(&ltc->error, HAL_GetTick());
	ER_CHK(error);  // In case of error, set the error and goto label End

End:;

	return count;
}

/**
 * @brief		Checks that voltage is between its thresholds.
 *
 * @param		volts		The voltage
 * @param		error		The error return code
 */
void ltc6813_check_voltage(uint16_t volts, ERROR_STATUS_T *volt_error,
						   warning_t *warning, error_t *error) {
	if (volts < CELL_WARN_VOLTAGE) {
		*warning = WARN_CELL_LOW_VOLTAGE;
	}

	if (volts < CELL_MIN_VOLTAGE) {
		error_set(ERROR_CELL_UNDER_VOLTAGE, volt_error, HAL_GetTick());
	} else {
		error_unset(ERROR_CELL_UNDER_VOLTAGE, volt_error);
	}

	if (volts > CELL_MAX_VOLTAGE) {
		error_set(ERROR_CELL_OVER_VOLTAGE, volt_error, HAL_GetTick());
	} else {
		error_unset(ERROR_CELL_OVER_VOLTAGE, volt_error);
	}

	*error = error_check_fatal(volt_error, HAL_GetTick());
	ER_CHK(error);

End:;
}

/**
 * @brief		Checks that temperature is between its thresholds.
 *
 * @param		temp		The temperature
 * @param		error		The error return code
 */
void ltc6813_check_temperature(uint16_t temps, ERROR_STATUS_T *temp_error,
							   error_t *error) {
	if (temps >= CELL_MAX_TEMPERATURE) {
		error_set(ERROR_CELL_OVER_TEMPERATURE, temp_error, HAL_GetTick());
	} else {
		error_unset(ERROR_CELL_OVER_TEMPERATURE, temp_error);
	}

	*error = error_check_fatal(temp_error, HAL_GetTick());
	ER_CHK(error);

End:;
}

void ltc6813_set_dcc(uint8_t indexes[], uint8_t cfgar[8], uint8_t cfgbr[8]) {
	for (uint8_t i = 0; i < PACK_MODULE_COUNT; i++) {
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

/**
 * @brief		This function is used to calculate the PEC value
 *
 * @param		len		Length of the data array
 * @param		data	Array of data
 */
uint16_t ltc6813_pec15(uint8_t len, uint8_t data[]) {
	uint16_t remainder, address;
	remainder = 16;  // PEC seed
	for (int i = 0; i < len; i++) {
		// calculate PEC table address
		address = ((remainder >> 7) ^ data[i]) & 0xff;
		remainder = (remainder << 8) ^ crcTable[address];
	}
	// The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
	return (remainder * 2);
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
 * @retval	Temperature [C° * 100]
 */
uint16_t ltc6813_convert_temp(uint16_t volt) {
	float voltf = volt * 0.0001;
	float temp;
	temp = -225.7 * voltf * voltf * voltf + 1310.6 * voltf * voltf -
		   2594.8 * voltf + 1767.8;
	return (uint16_t)(temp * 100);
}

/**
 * @brief		This function sets the GPIO configuration for the ltc
 *
 * @param		mask	first byte of CFGAR
 *
 */
