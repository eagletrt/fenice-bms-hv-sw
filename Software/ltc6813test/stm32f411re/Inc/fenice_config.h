/**
 * @file		fenice_config.h
 * @brief		This file contains configuration settings for Chimera Evoluzione
 *
 * @date		Oct 07, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef FENICE_CONFIG_H_
#define FENICE_CONFIG_H_
#include <stdbool.h>

// Pin definitions
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_6
#define SPI1_CS_GPIO_Port GPIOB

#define CHARGING 0

#define LTC6813_COUNT 1 /*!< Number of daisy chained LTCs */
#define LTC6813_CELL_COUNT 18
/*!< Number of cells a single IC controls. Refer to cell_distribution for
 * configuration */

#define LTC6813_REG_COUNT 6
/* Number of registers for a single IC. A, B, C, D */
#define LTC6813_REG_CELL_COUNT 3
/* Max number of cells handled by a register.
							   Refer to cell distribution */

// Total number of cells in series
#define PACK_MODULE_COUNT LTC6813_COUNT* LTC6813_CELL_COUNT
#define PACK_MAX_CURRENT 200

#define CELL_WARN_VOLTAGE 28000
#define CELL_MIN_VOLTAGE 25000
#define CELL_MAX_VOLTAGE 42250
#define CELL_MAX_TEMPERATURE 6000

#define PACK_MAX_VOLTAGE_THRESHOLD 500

#endif /* CHIMERA_CONFIG_H_ */
