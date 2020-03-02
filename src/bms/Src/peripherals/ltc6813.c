/**
 * @file		ltc6813.c
 * @brief		This file contains the functions to communicate with the LTCs
 *
 * @date		Oct 08, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "peripherals/ltc6813.h"

// Set to 1 to emulate the LTC daisy chain
#define LTC6813_EMU 1

uint8_t GPIO_CONFIG;

/**
 * @brief		Starts the LTC6813 ADC voltage conversion
 * @details	According to the datasheet, this command should take 2,335µs.
 * 					ADCV Command syntax:
 *
 * 					1     CMD0    8     CMD1      16      32
 * 					|- - - - - - -|- - - - - - - -|- ... -|
 * 					0 0 0 0 0 0 1 1 0 1 1 X 0 0 0 0  PEC
 * 					 Address |    | |     |
 * 					  (BRD)      Speed   DCP
 *
 * @param		spi		The spi configuration structure
 * @param		dcp		false to read voltages; true to read temperatures
 */
void _ltc6813_adcv(SPI_HandleTypeDef *spi, bool dcp) {
	uint8_t cmd[4];
	uint16_t cmd_pec;
	cmd[0] = (uint8_t)0b00000011;
	cmd[1] = (uint8_t)0b01100000 + dcp * 0b00010000;
	cmd_pec = ltc6813_pec15(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	ltc6813_wakeup_idle(spi, false);
	// HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(spi, cmd, 4, 100);
	// HAL_Delay(1);
	// HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);
}

/**
 * @brief		Enable or disable the temperature measurement through
 *balancing
 * @details	Since it's not possible to read the temperatures from adiacent
 * 					cells at the same time, We split the measurement into
 *two times, read odd cells, and then even ones. To write configuration you
 *have to send 2 consecutive commands:
 *
 *					WRCFG:
 * 					1     CMD0    8     CMD1      16      32
 * 					|- - - - - - -|- - - - - - - -|- ... -|
 * 					0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1  PEC
 *
 *					CFGR:
 * 					1             8               16
 * 					|- - - - - - -|- - - - - - - -|
 * 					0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
 * 					17            24              32
 * 					|- - - - - - -|- - - - - - - -|
 * 					0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
 *					33            40              48      64
 *					|- - - - - - -|- - - - - - - -|- ... -|
 *					1 0 0 1 0 1 0 1 0 0 0 0 0 0 1 0  PEC	<- For odd cells
 *					              or
 *					0 1 0 0 1 0 1 0 0 0 0 0 0 0 0 1  PEC	<- For even
 *cells
 *
 * @param		hspi			The SPI configuration structure
 * @param		start_bal	whether to start temperature measurement
 * @param		even			Indicates whether we're reading odd or even
 *cells
 */
// TODO: remove this function
void _ltc6813_wrcfg(SPI_HandleTypeDef *hspi, bool start_bal, bool even) {
	uint8_t wrcfg[4];
	uint8_t cfgr[8];

	uint16_t cmd_pec;

	wrcfg[0] = 0x00;
	wrcfg[1] = 0x01;
	cmd_pec = ltc6813_pec15(2, wrcfg);
	wrcfg[2] = (uint8_t)(cmd_pec >> 8);
	wrcfg[3] = (uint8_t)(cmd_pec);

	cfgr[0] = 0x00;
	cfgr[1] = 0x00;
	cfgr[2] = 0x00;
	cfgr[3] = 0x00;

	if (start_bal) {
		if (even) {
			// Command to balance cells (in order) 8,5,3,1 and 10
			cfgr[4] = 0b10010101;
			cfgr[5] = 0b00000010;
		} else {
			// Command to balance cells (in order) 7,4,2 and 9
			cfgr[4] = 0b01001010;
			// First 4 bits are for DCT0 and should remain 0
			cfgr[5] = 0b00000001;
		}
	} else {
		cfgr[4] = 0x00;
		cfgr[5] = 0x00;
	}
	cmd_pec = ltc6813_pec15(6, cfgr);
	cfgr[6] = (uint8_t)(cmd_pec >> 8);
	cfgr[7] = (uint8_t)(cmd_pec);

	ltc6813_wakeup_idle(hspi, true);

	// HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, wrcfg, 4, 100);
	// HAL_Delay(1);
	HAL_SPI_Transmit(hspi, cfgr, 8, 100);
	// HAL_Delay(1);
	// HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);

	// TODO: remove this
	_ltc6813_adcv(hspi, start_bal);
}

void ltc6813_wrcfg(SPI_HandleTypeDef *hspi, bool is_a,
				   uint8_t cfgr[LTC6813_COUNT][8]) {
	uint8_t cmd[4] = {0};

	if (is_a) {
		// WRCFGA
		cmd[1] = 1;
	} else {
		// WRCFGB
		cmd[1] = 0b00100100;
	}

	uint16_t cmd_pec = ltc6813_pec15(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	// HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	// HAL_Delay(1);

	for (uint8_t i = 0; i < LTC6813_COUNT; i++) {
		// set the configuration for the #i ltc on the chain
		// GPIO configs are equal for all ltcs
		cfgr[i][GPIO_CFGAR_POS] =
			GPIO_CONFIG + ((!GPIO_CFGAR_MASK) | cfgr[i][GPIO_CFGAR_POS]);
		HAL_SPI_Transmit(hspi, cfgr[i], 8, 100);
	}

	// HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);
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

void ltc6813_wrcomm_i2c_w(SPI_HandleTypeDef *hspi, uint8_t address,
						  uint8_t *data) {
	uint8_t cmd[4] = {0b00000111, 0b00100001};	// WRCOMM

	uint16_t cmd_pec = ltc6813_pec15(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	uint8_t comm[8] = {0};

	comm[0] = I2C_START | (address >> 3);
	comm[1] = (address << 5) | (0 << 4) | I2C_MASTER_ACK;

	comm[2] = I2C_BLANK | (data[0] >> 4);
	comm[3] = (data[0] << 4) | I2C_MASTER_ACK;

	comm[4] = I2C_START | (address >> 3);
	comm[5] = (address << 5) | (1 << 4) | I2C_MASTER_ACK;

	uint16_t pec = ltc6813_pec15(6, comm);
	comm[6] = (uint8_t)(pec >> 8);
	comm[7] = (uint8_t)(pec);

	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
	while (hspi->State != HAL_SPI_STATE_READY)
		;
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Transmit(hspi, comm, 8, 100);
	while (hspi->State == HAL_SPI_STATE_BUSY)
		;

	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);
}

void ltc6813_wrcomm_i2c_r(SPI_HandleTypeDef *hspi, uint8_t address) {
	uint8_t cmd[4] = {0b00000111, 0b00100001};	// WRCOMM

	uint16_t cmd_pec = ltc6813_pec15(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	uint8_t comm[8] = {0};

	comm[0] = I2C_BLANK | (0xF >> 4);
	comm[1] = (0xF << 4) | I2C_MASTER_ACK;

	comm[2] = I2C_BLANK | (0xF >> 4);
	comm[3] = (uint8_t)(0xF << 4) | I2C_MASTER_ACK;

	comm[4] = I2C_BLANK | (0xF >> 4);
	comm[5] = (uint8_t)(0xF << 4) | I2C_MASTER_NACK_STOP;

	uint16_t pec = ltc6813_pec15(6, comm);
	comm[6] = (uint8_t)(pec >> 8);
	comm[7] = (uint8_t)(pec);

	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
	while (hspi->State != HAL_SPI_STATE_READY)
		;
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Transmit(hspi, comm, 8, 100);
	while (hspi->State == HAL_SPI_STATE_BUSY)
		;
	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);
	// HAL_Delay(1);
	// HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);
}

bool ltc6813_rdcomm_i2c(SPI_HandleTypeDef *hspi, uint8_t data[8]) {
	uint8_t cmd[4] = {0b00000111, 0b00100010};	// RDCOMM

	uint16_t cmd_pec = ltc6813_pec15(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
	while (hspi->State != HAL_SPI_STATE_READY)
		;
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Receive(hspi, data, 8, 100);

	while (hspi->State == HAL_SPI_STATE_BUSY)
		;
	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);

	if (ltc6813_pec15(6, data) == (uint16_t)(data[6] * 256 + data[7])) {
		return true;
	}
	return false;
}

void ltc6813_stcomm_i2c(SPI_HandleTypeDef *hspi, uint8_t length) {
	uint8_t cmd[4] = {0b00000111, 0b00100011};	// STCOMM

	uint16_t cmd_pec = ltc6813_pec15(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
	while (hspi->State != HAL_SPI_STATE_READY)
		;
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	for (uint8_t i = 0; i < 3 * length; i++) {
		HAL_SPI_Transmit(hspi, (uint8_t *)0xFF, 1, 20);
	}
	while (hspi->State == HAL_SPI_STATE_BUSY)
		;

	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);
	/*HAL_Delay(1);
	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);*/
}

/**
 * @brief		Wakes up all the devices connected to the isoSPI bus
 *
 * @param		hspi	The SPI configuration structure
 */
void ltc6813_wakeup_idle(SPI_HandleTypeDef *hspi, bool apply_delay) {
	uint8_t data = 0xFF;
	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, &data, 1, 1);
	if (apply_delay) {
		HAL_Delay(1);
	}
	HAL_GPIO_WritePin(CS_LTC_GPIO_Port, CS_LTC_Pin, GPIO_PIN_SET);
}
