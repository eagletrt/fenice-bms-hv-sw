/**
 * @file max22530.c
 * @brief MAX22530AWE+ commmunication interface 
 * 
 * @date Mar 31, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "peripherals/max22530.h"
#include "bms-monitor/ltc6811/ltc6811.h"

void max22530_cs_enable(GPIO_TypeDef * port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}
void max22530_cs_disable(GPIO_TypeDef * port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

// TODO: Enable CRC?
void max22530_init(SPI_HandleTypeDef * spi, GPIO_TypeDef * port, uint16_t pin) {
    uint8_t cmd[3] = { 0 };
    cmd[0] = (MAX22530_CONTROL_REG << 2) | (MAX22530_WR << 1); // Write on the control register
    // cmd[1] = (1 << 15); // Enable spi CRC (ENCRC)

    max22530_cs_enable(port, pin);
    HAL_SPI_Transmit(spi, cmd, sizeof(cmd), 100);
    max22530_cs_disable(port, pin);
}

// TODO: Check received data size
float max22530_read_channel(SPI_HandleTypeDef * spi,
    GPIO_TypeDef * port,
    uint16_t pin,
    MAX22530_CH channel) {
    uint8_t cmd[3] = { 0 };
    cmd[0] = ((channel + MAX22530_FILTERED_OFFSET) << 2); // Read filtered ADC register

    max22530_cs_enable(port, pin);
    HAL_SPI_TransmitReceive(spi, cmd, cmd, sizeof(cmd), 100);
    max22530_cs_disable(port, pin);

    // Get 12 bit ADC value
    uint16_t val = (*(uint16_t *)(cmd + 1)) & 0x0FFF;

    return MAX22530_CONV_VALUE_TO_VOLTAGE(val);
}

HAL_StatusTypeDef max22530_read_channels(SPI_HandleTypeDef *spi,
    GPIO_TypeDef * port,
    uint16_t pin,
    MAX22530_CH * channels,
    uint8_t ch_number,
    float * data_out) {
    if (ch_number > 4)
        return HAL_ERROR;

    uint8_t cmd[11] = { 0 };
    cmd[0] = ((MAX22530_CH1 + MAX22530_FILTERED_OFFSET) << 2) | MAX22530_BURST; // Read filtered ADC register in burst mode

    max22530_cs_enable(port, pin);
    HAL_SPI_TransmitReceive(spi, cmd, cmd, sizeof(cmd), 100);
    max22530_cs_disable(port, pin);

    for (uint8_t i = 0; i < ch_number; ++i) {
        MAX22530_CH ch = channels[i];
        // Get 12 bit ADC value
        uint16_t val = ( *(uint16_t *)(cmd + (ch * 2)) ) & 0x0FFF;
        data_out[i] = MAX22530_CONV_VALUE_TO_VOLTAGE(val);
    }

    return HAL_OK;
}