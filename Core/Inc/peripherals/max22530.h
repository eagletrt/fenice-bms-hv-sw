/**
 * @file max22530.h
 * @brief MAX22530AWE+ communication interface
 *
 * @date Mar 31, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include <stdbool.h>
#include <inttypes.h>

#define MAX22530_VREF 3.3f
#define MAX22530_CONV_VALUE_TO_VOLTAGE(x) ((x) * (MAX22530_VREF / 4095))

#define MAX22530_CONTROL_REG 0x14 // Address of the control register

#define MAX22530_VTS_CHANNEL   MAX22530_CH1
#define MAX22530_VBATT_CHANNEL MAX22530_CH2
#define MAX22530_SHUNT_CHANNEL MAX22530_CH3
#define MAX22530_TSN_CHANNEL   MAX22530_CH4

#define MAX22530_FILTERED_OFFSET 0x04 // Address offset for the filtered ADC registers

#define MAX22530_BURST 1 // Burst mode

/** @brief Address of the ADC registers */
typedef enum {
    MAX22530_CH1 = 0x01,
    MAX22530_CH2,
    MAX22530_CH3,
    MAX22530_CH4
} MAX22530_CH;
/** @brief MAX22530 operations */
typedef enum {
    MAX22530_RD = 0,
    MAX22530_WR
} MAX22530_OP;

/**
 * @brief Enable chip select
 * 
 * @param spi The spi configuration structure
 * @param port The GPIO port
 * @param pin The GPIO pin
 */
void max22530_cs_enable(GPIO_TypeDef * port, uint16_t pin);
/**
 * @brief Disable chip select
 * 
 * @param spi The spi configuration structure
 * @param port The GPIO port
 * @param pin The GPIO pin
 */
void max22530_cs_disable(GPIO_TypeDef * port, uint16_t pin);

/**
 * @brief Set initial configuration of the MAX22530
 * @details By default CRC is disabled so every command is 24 bit long instead of 36 bit,
 * but it can be enabled through spi in the control register of the MAX22530
 *
 * @param spi The spi configuration structure
 * @param port The GPIO port
 * @param pin The GPIO pin
 */
void max22530_init(SPI_HandleTypeDef * spi, GPIO_TypeDef * port, uint16_t pin);
/**
 * @brief Read data from a channel
 * @details Command structure:
 *
 * 					31   HEADER   23     DATA      7   CRC   0
 * 					|- - - - - - -|- - - ... - - - | - ... - |
 * 					X X X X X X 0 0                 (Optional)
 *                  - - - - - -
 * 					  Address
 * 
 * @param spi The spi configuration structure
 * @param port The GPIO port
 * @param pin The GPIO pin
 * @param channel ADC channel to select
 * @return float The returned value
 */
float max22530_read_channel(SPI_HandleTypeDef * spi,
    GPIO_TypeDef * port,
    uint16_t pin,
    MAX22530_CH channel);
/**
 * @brief Read data from multiple channels
 * @details Command structure:
 *
 *        95   HEADER   87     71      55      39       23          7   CRC   0
 *        |- - - - - - -|- ... -|- ... -|- ... -|- ... -|- - ... - -| - ... - |
 *        0 0 0 1 0 1 0 1  ADC1    ADC2    ADC3    ADC4   Interrupt  (Optional)
 *        - - - - - -                                      Status
 *          Address
 *          (FADC)
 * 
 * @param spi The spi configuration structure
 * @param port The GPIO port
 * @param pin The GPIO pin
 * @param channels ADC channels to select
 * @param ch_number Number of selected channels
 * @param data_out Array where the data is stored
 * @return HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef max22530_read_channels(SPI_HandleTypeDef * spi,
    GPIO_TypeDef * port,
    uint16_t pin,
    MAX22530_CH * channels,
    uint8_t ch_number,
    float * data_out);