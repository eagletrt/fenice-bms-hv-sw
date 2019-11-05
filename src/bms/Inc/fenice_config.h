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

#define CS_LTC_Pin GPIO_PIN_4
#define CS_LTC_GPIO_Port GPIOA

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

#define BAL_MAX_VOLTAGE_THRESHOLD 100
#define BAL_CYCLE_LENGTH 120000

#endif /* CHIMERA_CONFIG_H_ */
