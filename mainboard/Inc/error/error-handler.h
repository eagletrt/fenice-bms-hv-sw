/*******************************************************************************
 * Critical error handler library generator
 * Generated by error_gen ruby gem, for more information see:
 * https://github.com/eagletrt/micro-utils/tree/master/error-handler-generator
 *
 * Error_gen version 1.3.4
 * Generation date: 2024-06-04 16:25:13 +0200
 * Generated from: errors.json
 * The error handler contains:
 *     - 15 error groups
 *     - 66 total error instances
 ******************************************************************************/

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Total number of error instances
#define ERROR_INSTANCE_COUNT 66

/**
 * @brief Set or reset an instance of an error based on a condition
 * @details If the condition is true the error is set, otherwise it is reset
 *
 * @param condition A boolean expression
 * @param group The group to which the error belongs
 * @param instance The instance of the error
 * @param The current time (in ms)
 */
#define ERROR_TOGGLE_IF(condition, group, instance, timestamp) \
    ((condition) ? error_set(group, instance, timestamp) : error_reset(group, instance))

/** @brief Type of the error that categorize a group of instances */
typedef enum {
    ERROR_CELL_UNDER_VOLTAGE,
    ERROR_CELL_OVER_VOLTAGE,
    ERROR_CELL_UNDER_TEMPERATURE,
    ERROR_CELL_OVER_TEMPERATURE,
    ERROR_OVER_CURRENT,
    ERROR_CAN,
    ERROR_INT_VOLTAGE_MISMATCH,
    ERROR_CELLBOARD_COMM,
    ERROR_CELLBOARD_INTERNAL,
    ERROR_CONNECTOR_DISCONNECTED,
    ERROR_FANS_DISCONNECTED,
    ERROR_FEEDBACK,
    ERROR_FEEDBACK_CIRCUITRY,
    ERROR_EEPROM_COMM,
    ERROR_EEPROM_WRITE,
    ERROR_COUNT
} ErrorGroup;

// Single error instance type definition
typedef uint16_t ErrorInstance;

/**
 * @brief Error type definition
 *
 * @param group The group to which the error belongs
 * @param timestamp The time when the error was set (in ms)
 * @param is_running True if the error is set, false otherwise
 * @param is_expired True if the error has expired, false otherwise
 */
typedef struct {
    ErrorGroup group;
    uint32_t timestamp;
    bool is_running;
    bool is_expired;
} Error;

/**
 * @brief Initialize the internal error handler structures
 * @details A critical section is defined as a block of code where, if an interrupt
 * happens, undefined behaviour with the modified data within the block can happen
 *
 * @param cs_enter A pointer to a function that should manage a critical section
 * @param cs_exit A pointer to a function that shuold manage a critical section
 */
void error_init(void (* cs_enter)(void), void (* cs_exit)(void));

/**
 * @brief Get the number of errors that has been set but they still have to expire
 * 
 * @param size_t The number of running errors
 */
size_t error_get_running(void);

/**
 * @brief Get the number of expired errors
 * 
 * @param size_t The number of expired errors
 */
size_t error_get_expired(void);

/**
 * @brief Get the number of running error of a specific group
 *
 * @param group The error group
 * @return uint16_t The number of running errors
 */
uint16_t error_get_group_running(ErrorGroup group);

/**
 * @brief Get the number of expired error of a specific group
 *
 * @param group The error group
 * @return uint16_t The number of running errors
 */
uint16_t error_get_group_expired(ErrorGroup group);

/**
 * @brief Get a copy of all the errors that are currently running
 * @attention This function can be quite expensive in terms of time
 * and should be used wisely, do not call to often
 * @attention This function calls the critical section handler functions
 * @details The out array should be able to contain all the instances
 *
 * @param out A pointer to an array of errors where the data is copied into
 * @return size_t The number of copied errors
 */
size_t error_dump_running(Error * out);

/**
 * @brief Get a copy of all the errors that are expired
 * @attention This function can be quite expensive in terms of time
 * and should be used wisely, do not call to often
 * @attention This function calls the critical section handler functions
 * @details The out array should be able to contain all the instances
 *
 * @param out A pointer to an array of errors where the data is copied into
 * @return size_t The number of copied errors
 */
size_t error_dump_expired(Error * out);

/**
 * @brief Get all the groups in which at least one error is running
 * 
 * @param out A pointer to an array of groups where the data is copied into
 * @return size_t The number of copied groups
 */
size_t error_dump_running_groups(ErrorGroup * out);

/**
 * @brief Get all the groups in which at least one error is expired
 * 
 * @param out A pointer to an array of groups where the data is copied into
 * @return size_t The number of copied groups
 */
size_t error_dump_expired_groups(ErrorGroup * out);

/**
 * @brief Set an error which will expire after a certain amount of time (the timeout)
 * 
 * @param group The group to which the error belongs
 * @param instance The instance of the error
 * @param The current time (in ms)
 */
void error_set(ErrorGroup group, ErrorInstance instance, uint32_t timestamp);

/**
 * @brief Reset an error to avoid its expiration
 *
 * @param group The group to which the error belongs
 * @param instance The instance of the error
 */
void error_reset(ErrorGroup group, ErrorInstance instance);

/** @brief Set the error as expired */
void error_expire(void);

/**
 * @brief Routine that updates the internal error states
 * @attention This function should not be called inside interrupts callback
 * or other threads
 * @details This function should be called periodically
 */
void error_routine(void);

/**
 * @brief Update the timer that should expire the error after a certain amount of time
 * @attention This function have to be defined by the user
 * @details This function is called internally when an error is set, reset or expired
 *
 * @param timestamp The time in which the error was set (in ms)
 * @param timeout The time that should elapse after the timestamp to expire the error (in ms)
 */
void error_update_timer_callback(uint32_t timestamp, uint16_t timeout);

/**
 * @brief Stop the timer that should expire the errors
 * @attention This function have to be defined by the user
 * @details This function is called internally when an error is reset or expired
 */
void error_stop_timer_callback(void);

#endif  // ERROR_HANDLER_H

