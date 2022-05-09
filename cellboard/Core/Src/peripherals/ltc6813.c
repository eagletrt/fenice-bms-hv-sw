/**
 * @file		ltc6813.c
 * @brief		This file contains the functions to communicate with the LTCs
 *
 * @date		Oct 08, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "peripherals/ltc6813.h"

#include "main.h"

void ltc6813_enable_cs(SPI_HandleTypeDef *spi) {
    HAL_GPIO_WritePin(LTC_CS_GPIO_Port, LTC_CS_Pin, GPIO_PIN_RESET);
}

void ltc6813_disable_cs(SPI_HandleTypeDef *spi) {
    HAL_GPIO_WritePin(LTC_CS_GPIO_Port, LTC_CS_Pin, GPIO_PIN_SET);
}

/**
 * @brief	Starts the LTC6813 ADC voltage conversion
 * @details	According to the datasheet, this command should take 2,335µs.
 * 					ADCV Command syntax:
 *
 * 					0     CMD0    7     CMD1      15 PEC  31
 * 					|- - - - - - -|- - - - - - - -|- ... -|
 * 					0 0 0 0 0 0 1 1 0 1 1 X 0 0 0 0
 * 				    - - - - -     - -     -
 * 					 Address     Mode    DCP
 *
 * @param	spi		The spi configuration structure
 */
void ltc6813_adcv(SPI_HandleTypeDef *spi) {
    uint8_t cmd[4];
    uint16_t cmd_pec;
    cmd[0]  = (uint8_t)0b00000011;
    cmd[1]  = (uint8_t)0b01100000;
    cmd_pec = ltc6813_pec15(2, cmd);
    cmd[2]  = (uint8_t)(cmd_pec >> 8);
    cmd[3]  = (uint8_t)(cmd_pec);

    ltc6813_wakeup_idle(spi);

    ltc6813_enable_cs(spi);
    HAL_SPI_Transmit(spi, cmd, 4, 100);
    ltc6813_disable_cs(spi);
}

/**
 * @brief	Polls ADC Convertion
 * @details	PLADC Command syntax:
 *
 * 					0     CMD0    7     CMD1      15 PEC  31
 * 					|- - - - - - -|- - - - - - - -|- ... -|
 * 					0 0 0 0 0 1 1 1 0 0 0 1 0 1 0 0
 * 				    - - - - -
 * 					 Address
 *
 * @param	spi		The spi configuration structure
 */
HAL_StatusTypeDef ltc6813_poll_convertion(SPI_HandleTypeDef *hspi, uint32_t timeout) {
    uint8_t cmd[4];
    uint16_t pec;
    uint32_t tick;
    cmd[0] = 0b00000111;
    cmd[1] = 0b00010100;
    pec    = ltc6813_pec15(2, cmd);
    cmd[2] = (uint8_t)(pec >> 8);
    cmd[3] = (uint8_t)(pec);

    ltc6813_wakeup_idle(hspi);
    ltc6813_enable_cs(hspi);
    HAL_SPI_Transmit(hspi, cmd, 4, 100);

    tick             = HAL_GetTick();
    *(uint32_t *)cmd = 0;
    do {
        HAL_SPI_Receive(hspi, cmd, 4, 10);
    } while (!(*(uint32_t *)cmd) &&
             (HAL_GetTick() - tick < timeout));  //SDO is pulled down until the convertion is finished,
                                                 //so exit from this cicle when the received data is no more 0

    ltc6813_disable_cs(hspi);

    return *(uint32_t *)cmd ? HAL_OK : HAL_TIMEOUT;
}

void ltc6813_wrcfg(SPI_HandleTypeDef *hspi, wrcfg_register reg, uint8_t cfgr[8]) {
    uint8_t cmd[4] = {0};

    if (reg == WRCFGA) {
        cmd[1] = 1;
    } else if (reg == WRCFGB) {
        // WRCFGB
        cmd[1] = 0b00100100;
    }

    uint16_t cmd_pec = ltc6813_pec15(2, cmd);
    cmd[2]           = (uint8_t)(cmd_pec >> 8);
    cmd[3]           = (uint8_t)(cmd_pec);

    ltc6813_enable_cs(hspi);
    HAL_SPI_Transmit(hspi, cmd, 4, 100);

    // set the configuration for the #i ltc on the chain
    // GPIO configs are equal for all ltcs
    //cfgr[GPIO_CFGAR_POS] = GPIO_CONFIG + ((!GPIO_CFGAR_MASK) | cfgr[GPIO_CFGAR_POS]);
    HAL_SPI_Transmit(hspi, cfgr, 8, 100);
    ltc6813_disable_cs(hspi);
}

/**
 * @brief		Wakes up all the devices connected to the isoSPI bus
 *
 * @param		hspi	The SPI configuration structure
 */
void ltc6813_wakeup_idle(SPI_HandleTypeDef *hspi) {
    uint8_t data = 0xFF;

    ltc6813_enable_cs(hspi);

    HAL_SPI_Transmit(hspi, &data, 1, 1);

    ltc6813_disable_cs(hspi);
}

uint16_t ltc6813_pec15(uint8_t len, uint8_t data[]) {
    uint16_t remainder, address;
    remainder = 16;  // PEC seed
    for (int i = 0; i < len; i++) {
        // calculate PEC table address
        address   = ((remainder >> 7) ^ data[i]) & 0xff;
        remainder = (remainder << 8) ^ crcTable[address];
    }
    // The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
    return (remainder * 2);
}
