/**
 * @file error.h
 * @brief Error handling functions and structures
 * 
 * @date Jul 08, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef ERROR_H
#define ERROR_H

#include <inttypes.h>
#include "stm32f4xx_hal.h"
#include "micro-libs/error-utils/error_utils.h"


#define ERROR_TIMEOUT_INSTANT 0
#define ERROR_TIMEOUT_NEVER UINT32_MAX

#define ERROR_SET_INT(error, instance) ERROR_UTILS_SET_INT(&error_handler, error, instance)
#define ERROR_SET_STR(error, instance) ERROR_UTILS_SET_STR(&error_handler, error, instance)

#define ERROR_RESET_INT(error, instance) ERROR_UTILS_RESET_INT(&error_handler, error, instance)
#define ERROR_RESET_STR(error, instance) ERROR_UTILS_RESET_STR(&error_handler, error, instance)

#define ERROR_TOGGLE_CHECK_INT(condition, error, instance) \
    do {                                                   \
        if (condition) {                                   \
            ERROR_SET_INT(error, instance);                \
        }                                                  \
        else {                                             \
            ERROR_RESET_INT(error, instance);              \
        }                                                  \
    } while(0)
#define ERROR_TOGGLE_CHECK_STR(condition, error, instance) \
    do {                                                   \
        if (condition) {                                   \
            ERROR_SET_STR(error, instance);                \
        }                                                  \
        else {                                             \
            ERROR_RESET_STR(error, instance);              \
        }                                                  \
    } while(0)


/** @brief Error types */
typedef enum {
    ERROR_CELL_LOW_VOLTAGE,
    ERROR_CELL_UNDER_VOLTAGE,
    ERROR_CELL_OVER_VOLTAGE,
    ERROR_CELL_UNDER_TEMPERATURE,
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


// Do not use this structure directly
extern ErrorUtilsHandler error_handler;

// String instances names
extern const char * error_mainboard_instance;
extern const char * error_pack_instance;
extern const char * error_current_instance;
extern const char * error_bms_can_instance;
extern const char * error_car_can_instance;


/** @brief Initialize the error handler */
void error_init();
/**
 * @brief Get the timeout of an error in ms
 *
 * @param error The error type
 * @return uint32_t The timeout of the error
 */
uint32_t error_get_timeout_ms(uint32_t);

/** @brief Expire the first error */
void error_expire_errors();

/**
 * @brief Get the number of currently running errors
 * 
 * @return size_t The number of running errors
 */
size_t error_running_count();

/**
 * @brief Get the number of currently expired errors
 * 
 * @return size_t The number of expired errors
 */
size_t error_expired_count();

// TODO: Remove, for debug purpose only
size_t error_dump(ErrorUtilsRunningInstance * errs[397U]);


#endif // ERROR_H
