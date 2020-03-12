/**
 * @file		ltc6813.c
 * @brief		This file contains the functions to communicate with the LTCs
 *
 * @date		Oct 08, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "comm/ltc6813.h"

#include "main.h"

uint8_t GPIO_CONFIG;

void ltc6813_enable_cs(SPI_HandleTypeDef *spi, GPIO_TypeDef *gpio,
					   uint16_t pin) {
	HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);
	while (spi->State != HAL_SPI_STATE_READY)
		;
}

void ltc6813_disable_cs(SPI_HandleTypeDef *spi, GPIO_TypeDef *gpio,
						uint16_t pin) {
	while (spi->State != HAL_SPI_STATE_READY)
		;
	HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);
}

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

	ltc6813_enable_cs(spi, CS_LTC_GPIO_Port, CS_LTC_Pin);
	ltc6813_wakeup_idle(spi, false);

	HAL_SPI_Transmit(spi, cmd, 4, 100);

	ltc6813_disable_cs(spi, CS_LTC_GPIO_Port, CS_LTC_Pin);
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

	ltc6813_enable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);
	HAL_SPI_Transmit(hspi, wrcfg, 4, 100);
	HAL_SPI_Transmit(hspi, cfgr, 8, 100);
	ltc6813_disable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);

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

	ltc6813_enable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);

	for (uint8_t i = 0; i < LTC6813_COUNT; i++) {
		// set the configuration for the #i ltc on the chain
		// GPIO configs are equal for all ltcs
		cfgr[i][GPIO_CFGAR_POS] =
			GPIO_CONFIG + ((!GPIO_CFGAR_MASK) | cfgr[i][GPIO_CFGAR_POS]);
		HAL_SPI_Transmit(hspi, cfgr[i], 8, 100);
	}

	ltc6813_disable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);
}

void ltc6813_wrcomm_i2c(SPI_HandleTypeDef *hspi, uint8_t data[8]) {
	uint8_t cmd[4] = {0b00000111, 0b00100001};	// WRCOMM
	uint16_t cmd_pec = ltc6813_pec15(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	ltc6813_enable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Transmit(hspi, data, 8, 100);
	ltc6813_disable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);
}

bool ltc6813_rdcomm_i2c(SPI_HandleTypeDef *hspi, uint8_t data[8]) {
	uint8_t cmd[4] = {0b00000111, 0b00100010};	// RDCOMM

	uint16_t cmd_pec = ltc6813_pec15(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	ltc6813_enable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Receive(hspi, data, 8, 100);
	ltc6813_disable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);

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

	ltc6813_enable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);

	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	for (uint8_t i = 0; i < 3 * length; i++) {
		HAL_SPI_Transmit(hspi, (uint8_t *)0xFF, 1, 20);
	}

	ltc6813_disable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);
}

/**
 * @brief		Wakes up all the devices connected to the isoSPI bus
 *
 * @param		hspi	The SPI configuration structure
 */
void ltc6813_wakeup_idle(SPI_HandleTypeDef *hspi, bool apply_delay) {
	uint8_t data = 0xFF;

	ltc6813_enable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);

	HAL_SPI_Transmit(hspi, &data, 1, 1);

	ltc6813_disable_cs(hspi, CS_LTC_GPIO_Port, CS_LTC_Pin);
}

/**
 * @brief		This function is used to calculate the PEC value
 *
 * @param		len		Length of the data array
 * @param		data	Array of data
 */
uint16_t ltc6813_pec15(uint8_t len, uint8_t data[]) {
	uint16_t remainder, address;
	remainder = 16;	 // PEC seed
	for (int i = 0; i < len; i++) {
		// calculate PEC table address
		address = ((remainder >> 7) ^ data[i]) & 0xff;
		remainder = (remainder << 8) ^ crcTable[address];
	}
	// The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
	return (remainder * 2);
}
