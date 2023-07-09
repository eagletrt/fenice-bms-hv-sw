/**
 * @file error.h
 * @brief Error handling unctions and structures
 * 
 * @date Jul 08, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef ERROR_H
#define ERROR_H

#include <inttypes.h>
#include "stm32f4xx_hal.h"

#define ERROR_TIMEOUT_INSTANT 0
#define ERROR_TIMEOUT_NEVER UINT32_MAX

#define ERROR_TOGGLE_CHECK(condition, index, offset) \
    if ((condition)) \
        error_set((index), (offset)); \
    else \
        error_reset((index), (offset));

/** @brief Error types */
typedef enum {
    ERROR_CELL_LOW_VOLTAGE,
    ERROR_CELL_UNDER_VOLTAGE,
    ERROR_CELL_OVER_VOLTAGE,
    ERROR_CELL_OVER_TEMPERATURE,
    ERROR_CELL_HIGH_TEMPERATURE,
    ERROR_OVER_CURRENT,

    ERROR_CAN_COMM,

    ERROR_VOLTAGE_MISMATCH,

    ERROR_CELLBOARD_COMM,
    ERROR_CELLBOARD_INTERNAL,

    ERROR_CONNECTOR_DISCONNECTED,
    ERROR_FANS_DISCONNECTED,
    ERROR_FEEDBACK,
    ERROR_FEEDBACK_CIRCUITRY,

    ERROR_EEPROM_COMM,
    ERROR_EEPROM_WRITE,

    ERROR_COUNT
} __attribute__((__packed__)) ErrorId;

/** @brief Timeout for each error type */
const uint32_t error_timeout[ERROR_COUNT] = {
    [ERROR_CELL_LOW_VOLTAGE]       = ERROR_TIMEOUT_NEVER,
    [ERROR_CELL_UNDER_VOLTAGE]     = 450,
    [ERROR_CELL_OVER_VOLTAGE]      = 450,
    [ERROR_CELL_HIGH_TEMPERATURE]  = ERROR_TIMEOUT_NEVER,
    [ERROR_CELL_OVER_TEMPERATURE]  = 750,
    [ERROR_OVER_CURRENT]           = 450,
    [ERROR_CAN_COMM]               = ERROR_TIMEOUT_NEVER,
    [ERROR_VOLTAGE_MISMATCH]       = ERROR_TIMEOUT_NEVER,
    [ERROR_CELLBOARD_COMM]         = 250,
    [ERROR_CELLBOARD_INTERNAL]     = ERROR_TIMEOUT_NEVER,
    [ERROR_CONNECTOR_DISCONNECTED] = 250,
    [ERROR_FANS_DISCONNECTED]      = ERROR_TIMEOUT_NEVER,
    [ERROR_FEEDBACK]               = ERROR_TIMEOUT_NEVER,
    [ERROR_FEEDBACK_CIRCUITRY]     = ERROR_TIMEOUT_NEVER,
    [ERROR_EEPROM_COMM]            = ERROR_TIMEOUT_NEVER,
    [ERROR_EEPROM_WRITE]           = ERROR_TIMEOUT_NEVER
};

/**
 * @brief Initialize the error handler
 * 
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef error_init();
/**
 * @brief Set an error given an index and an offset
 * 
 * @param index The type of the error
 * @param offset The instance of the error type
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef error_set(size_t index, size_t offset);
/**
 * @brief Reset an error given an index and an offset
 * 
 * @param index The type of the error
 * @param offset The instance of the error type
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef error_reset(size_t index, size_t offset);
/**
 * @brief Get the number of currently running errors
 * 
 * @return size_t The number of errors
 */
size_t error_count();
/**
 * @brief Callback function that should be called when the timer elapses
 * 
 * @param tim The timer that has elapsed
 * @return HAL_StatusTypeDef The reult of the operation
 */
HAL_StatusTypeDef error_timer_elapsed_callback(TIM_HandleTypeDef * tim);

#endif // ERROR_H