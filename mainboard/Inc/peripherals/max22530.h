/**
 * @file max22530.h
 * @brief MAX22530AWE+ communication interface
 *
 * @date Mar 31, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include <stm32f4xx_hal.h>
#include <stdbool.h>
#include <inttypes.h>

#define MAX22530_VREF 1.8f  // Reference voltage
/** @brief Convert 12 bit value to a voltage */
#define MAX22530_CONV_VALUE_TO_VOLTAGE(x) ((x) * (MAX22530_VREF / 4095))

#define MAX22530_ID_REG      0X00 // Address of the product id
#define MAX22530_CONTROL_REG 0x14 // Address of the control register

#define MAX22530_VTS_CHANNEL   MAX22530_CH1
#define MAX22530_VBATT_CHANNEL MAX22530_CH2
#define MAX22530_SHUNT_CHANNEL MAX22530_CH3
#define MAX22530_TSN_CHANNEL   MAX22530_CH4

#define MAX22530_FILTERED_OFFSET 0x04 // Address offset for the filtered ADC registers

#define MAX22530_BURST 1 // Burst mode

#define MAX22530_CHANNEL_COUNT 4 // Number of ADC channels


/** @brief Address of the ADC registers */
typedef enum {
    MAX22530_CH1 = 0x01,
    MAX22530_CH2,
    MAX22530_CH3,
    MAX22530_CH4,
} MAX22530_CH;
/** @brief MAX22530 operations */
typedef enum {
    MAX22530_RD = 0,
    MAX22530_WR
} MAX22530_OP;
/** @brief MAX22530 handler structure */
typedef struct {
    SPI_HandleTypeDef * spi;
    GPIO_TypeDef * gpio;
    uint16_t pin;
} MAX22530_HandleTypeDef;


/**
 * @brief Set initial configuration of the MAX22530
 * @attention By default CRC is disabled so every command is 24 bit long instead of 36 bit,
 * but it can be enabled through spi in the control register of the MAX22530
 *
 * @param handler The MAX22530 handler structure
 * @param spi The SPI configuration structure
 * @param gpio The GPIO port
 * @param pin The GPIO pin
 * @return HAL_StatusTypeDef The status of the SPI communication
 */
HAL_StatusTypeDef max22530_init(MAX22530_HandleTypeDef * handler,
    SPI_HandleTypeDef * spi,
    GPIO_TypeDef * gpio,
    uint16_t pin);
/**
 * @brief Read the device ID
 * 
 * @param handler The MAX22530 handler structure
 * @return int8_t The ID or -1 if there is a communication error
 */
int8_t max22530_get_id(MAX22530_HandleTypeDef * handler);
/**
 * @brief Read the device power on reset
 * 
 * @param handler The MAX22530 handler structure
 * @return int8_t The power on reset or -1 if there is a communication error
 */
int8_t max22530_get_por(MAX22530_HandleTypeDef * handler);
/**
 * @brief Read the device revision control of die
 * 
 * @param handler The MAX22530 handler structure
 * @return int8_t The revision control of die or -1 if there is a communication error
 */
int8_t max22530_get_rev(MAX22530_HandleTypeDef * handler);
/**
 * @brief Read data from a channel
 * 
 * @param handler The MAX22530 handler structure
 * @param channel ADC channel to select
 * @return float The returned value or -1 if there is an error during communication
 */
float max22530_read_channel(MAX22530_HandleTypeDef * handler, MAX22530_CH channel);
/**
 * @brief Read data from all the channels
 * 
 * @param handler The MAX22530 handler structure
 * @param volts Array where the data is stored
 * @return HAL_StatusTypeDef The status of the SPI communication
 */
HAL_StatusTypeDef max22530_read_all_channels(MAX22530_HandleTypeDef * handler, float volts[MAX22530_CHANNEL_COUNT]);
