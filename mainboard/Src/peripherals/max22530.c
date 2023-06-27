/**
 * @file max22530.c
 * @brief MAX22530AWE+ commmunication interface 
 * 
 * @date Mar 31, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "peripherals/max22530.h"

/**
 * @brief Enable chip select
 * @param handler The MAX22530 handler structure
 */
#define _MAX22530_CS_ENABLE(handler) HAL_GPIO_WritePin(handler->gpio, handler->pin, GPIO_PIN_RESET);
/**
 * @brief Disable chip select
 * @param handler The MAX22530 handler structure
 */
#define _MAX22530_CS_DISABLE(handler) HAL_GPIO_WritePin(handler->gpio, handler->pin, GPIO_PIN_SET);

/**
 * @brief Send a command to the MAX22530 via the SPI
 * 
 * @param handler The MAX22530 handler structure
 * @param cmd The command to send
 * @param len The length of the command in bytes
 * @return HAL_StatusTypeDef The status of the SPI communication
 */
HAL_StatusTypeDef _max22530_cmd_send(MAX22530_HandleTypeDef * handler, uint8_t cmd[], size_t len) {
    _MAX22530_CS_ENABLE(handler);
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(handler->spi, cmd, cmd, len, 10);
    _MAX22530_CS_DISABLE(handler);
    return status;
}
/**
 * @brief Write data to the address of the MAX22530
 * 
 * @param handler The MAX22530 handler structure
 * @param address The address to write the data to
 * @param data The data to write
 * @return HAL_StatusTypeDef The status of the SPI communication
 */
HAL_StatusTypeDef _max22530_cmd_write(MAX22530_HandleTypeDef * handler, uint8_t address, uint16_t data) {
    if (handler == NULL)
        return HAL_ERROR;
    
    const size_t cmd_size = 3;
    uint8_t cmd[cmd_size];
    
    cmd[0] = (address << 2) | (MAX22530_WR << 1);
    cmd[1] = (data & 0xFF00) >> 8;
    cmd[2] = data & 0x00FF;
    
    return _max22530_cmd_send(handler, cmd, cmd_size);
}
/**
 * @brief Read data from the address of the MAX22530
 * 
 * @param handler The MAX22530 handler structure
 * @param address The address to read the data from
 * @return int16_t The data read or -1 if there is a communication error
 */
int16_t _max22530_cmd_read(MAX22530_HandleTypeDef * handler, uint8_t address) {
    if (handler == NULL)
        return -1;
    
    const size_t cmd_size = 3;
    uint8_t cmd[cmd_size];

    // Fill unused data with 0xFF
    for (size_t i = 0; i < cmd_size; i++)
        cmd[i] = 0xFF;
    
    cmd[0] = (address << 2);
    
    if (_max22530_cmd_send(handler, cmd, cmd_size) != HAL_OK)
        return -1;

    uint16_t data = cmd[2] | ((uint16_t)cmd[1] << 8);
    return data;
}
/**
 * @brief Read channels data from the address of the MAX22530
 * 
 * @param handler The MAX22530 handler structure
 * @param address The address to read the data from
 * @param filtered If true the data are read from the filtered ADC
 * @return int16_t The data read or -1 if there is a communication error
 */
HAL_StatusTypeDef _max22530_cmd_burst(MAX22530_HandleTypeDef * handler, uint16_t data[MAX22530_CHANNEL_COUNT], bool filtered) {
    if (handler == NULL || data == NULL)
        return HAL_ERROR;
    
    const size_t cmd_size = 11;
    uint8_t cmd[cmd_size];

    // Fill unused data with 0xFF
    for (size_t i = 0; i < cmd_size; i++)
        cmd[i] = 0xFF;
    
    cmd[0] = ((MAX22530_CH1 + (filtered != 0) * MAX22530_FILTERED_OFFSET) << 2) | MAX22530_BURST;
    
    HAL_StatusTypeDef status;
    if ((status = _max22530_cmd_send(handler, cmd, cmd_size)) != HAL_OK)
        return status;

    for (size_t i = 0; i < MAX22530_CHANNEL_COUNT; i++)
        data[i] = cmd[i * 2 + 2] | (((uint16_t)cmd[i * 2 + 1] & 0x0F) << 8);

    return HAL_OK;
}


int8_t max22530_get_id(MAX22530_HandleTypeDef * handler) {
    uint16_t data = _max22530_cmd_read(handler, MAX22530_ID_REG);
    if (data < 0)
        return -1;
    return ((data & 0xFF00) >> 8);
}
int8_t max22530_get_por(MAX22530_HandleTypeDef * handler) {
    uint16_t data = _max22530_cmd_read(handler, MAX22530_ID_REG);
    if (data < 0)
        return -1;
    return ((data & 0b10000000) >> 7);
}
int8_t max22530_get_rev(MAX22530_HandleTypeDef * handler) {
    uint16_t data = _max22530_cmd_read(handler, MAX22530_ID_REG);
    if (data < 0)
        return -1;
    return (data & 0b01111111);
}
HAL_StatusTypeDef max22530_init(MAX22530_HandleTypeDef * handler,
    SPI_HandleTypeDef * spi,
    GPIO_TypeDef * gpio,
    uint16_t pin) {

    if (handler == NULL)
        return HAL_ERROR;

    handler->spi = spi;
    handler->gpio = gpio;
    handler->pin = pin;

    uint16_t data = 0;
    // cmd[1] = (1 << 15); // Enable spi CRC (ENCRC)

    return _max22530_cmd_write(handler, MAX22530_CONTROL_REG, data);
}
float max22530_read_channel(MAX22530_HandleTypeDef * handler, MAX22530_CH channel) {
    if (handler == NULL)
        return -1;

    uint8_t addr = ((channel + MAX22530_FILTERED_OFFSET) << 2); // Read filtered ADC register
    int16_t data = _max22530_cmd_read(handler, addr);

    if (data < 0)
        return -1;
    return MAX22530_CONV_VALUE_TO_VOLTAGE(data);
}
HAL_StatusTypeDef max22530_read_all_channels(MAX22530_HandleTypeDef * handler, float volts[MAX22530_CHANNEL_COUNT]) {
    if (handler == NULL)
        return HAL_ERROR;

    uint16_t data[MAX22530_CHANNEL_COUNT] = { 0 };

    // Read values
    HAL_StatusTypeDef status;
    if ((status = _max22530_cmd_burst(handler, data, true)) != HAL_OK)
        return status;

    // Convert and copy values
    for (size_t i = 0; i < MAX22530_CHANNEL_COUNT; i++)
        volts[i] = MAX22530_CONV_VALUE_TO_VOLTAGE(data[i]);
    return HAL_OK;
}
