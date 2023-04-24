/**
 * @file error.h
 * @brief This file contains the functions to handle errors.
 *
 * @date May 1, 2019
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Simone Ruffini [simone.ruffini@tutanota.com]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef ERROR_H
#define ERROR_H

#include <inttypes.h>

#define ERROR_TIMEOUT_SOFT UINT32_MAX

/** @brief Set error if the condition is true, otherwise reset */
#define ERROR_TOGGLE(condition, type, instance) (condition) ? error_set((type), (instance)) : error_reset((type), (instance))


// TODO: Change old error types
/** @brief Error type definitions */
typedef enum {
    ERROR_CELL_LOW_VOLTAGE,
    ERROR_CELL_UNDER_VOLTAGE,
    ERROR_CELL_OVER_VOLTAGE,
    ERROR_CELL_OVER_TEMPERATURE,
    ERROR_CELL_HIGH_TEMPERATURE,

    ERROR_OVER_CURRENT,
    ERROR_CAN,

    ERROR_INT_VOLTAGE_MISMATCH,

    ERROR_CELLBOARD_COMM,
    ERROR_CELLBOARD_INTERNAL,

    ERROR_FEEDBACK,
    ERROR_FEEDBACK_CIRCUITRY,

    ERROR_EEPROM_COMM,
    ERROR_EEPROM_WRITE,

    ERROR_COUNT
} __attribute__((__packed__)) ERROR_TYPE;

/**
 * @brief Timeout of each error type in ms 
 * @details Reaction times by the rules:
 *     - 500ms for voltages and current
 *     - 1s for temperatures
 */
const uint32_t error_timeouts[ERROR_COUNT] = {
    [ERROR_CELL_LOW_VOLTAGE]      = ERROR_TIMEOUT_SOFT,
    [ERROR_CELL_UNDER_VOLTAGE]    = 450,
    [ERROR_CELL_OVER_VOLTAGE]     = 450,
    [ERROR_CELL_HIGH_TEMPERATURE] = ERROR_TIMEOUT_SOFT,
    [ERROR_CELL_OVER_TEMPERATURE] = 750,
    [ERROR_OVER_CURRENT]          = 450,
    [ERROR_CAN]                   = ERROR_TIMEOUT_SOFT,
    [ERROR_INT_VOLTAGE_MISMATCH]  = ERROR_TIMEOUT_SOFT,
    [ERROR_CELLBOARD_COMM]        = 250,
    [ERROR_CELLBOARD_INTERNAL]    = ERROR_TIMEOUT_SOFT,
    [ERROR_FEEDBACK]              = ERROR_TIMEOUT_SOFT,
    [ERROR_FEEDBACK_CIRCUITRY]    = ERROR_TIMEOUT_SOFT,
    [ERROR_EEPROM_COMM]           = ERROR_TIMEOUT_SOFT,
    [ERROR_EEPROM_WRITE]          = ERROR_TIMEOUT_SOFT
};

/**
 * @brief Initialize error handler
 * 
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef error_init();
/**
 * @brief Set an instance of an error
 * 
 * @param type The type of the error
 * @param instance The instance of the error type
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef error_set(ERROR_TYPE type, size_t instance);
/**
 * @brief Reset an instance of an error
 * 
 * @param type The type of the error
 * @param instance The instance of the error type
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef error_reset(ERROR_TYPE type, size_t instance);

/**
 * @brief Get the number of running error instances
 * 
 * @return size_t The number of errors
 */
size_t error_count();

#endif /* ERROR_H_ */
