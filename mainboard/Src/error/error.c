/**
 * @file error.c
 * @brief Error handling functions and structures
 * 
 * @date Jul 08, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "error/error.h"

#include "../../fenice_config.h"
#include "mainboard_config.h"
#include "cmsis_compiler.h"

#define ERROR_BUFFER_SIZE 397U

// String instances names
const char * error_mainboard_instance = "MAINBOARD";
const char * error_pack_instance = "PACK";
const char * error_current_instance = "CURRENT";
const char * error_bms_can_instance = "BMS_CAN";
const char * error_car_can_instance = "CAR_CAN";

// Timer priority mask register
uint32_t rPriMask = 0;

/** @brief Error handler instance */
ErrorUtilsHandler error_handler;

/** @brief Array of running errors used by the error-utils library */
ErrorUtilsRunningInstance errors[ERROR_BUFFER_SIZE];
/** @brief Array of references to running errors used by the error-utils library */
ErrorUtilsRunningInstance * expiring[ERROR_BUFFER_SIZE];

/** @brief Timeout for each error type */
const uint32_t error_timeout[ERROR_COUNT] = {
    [ERROR_CELL_LOW_VOLTAGE]       = ERROR_TIMEOUT_NEVER,
    [ERROR_CELL_UNDER_VOLTAGE]     = 450,
    [ERROR_CELL_OVER_VOLTAGE]      = 450,
    [ERROR_CELL_UNDER_TEMPERATURE] = 750,
    [ERROR_CELL_HIGH_TEMPERATURE]  = ERROR_TIMEOUT_NEVER,
    [ERROR_CELL_OVER_TEMPERATURE]  = 750,
    [ERROR_OVER_CURRENT]           = 450,
    [ERROR_CAN_COMM]               = ERROR_TIMEOUT_NEVER,
    [ERROR_VOLTAGE_MISMATCH]       = 250,
    [ERROR_CELLBOARD_COMM]         = 250,
    [ERROR_CELLBOARD_INTERNAL]     = 750,
    [ERROR_CONNECTOR_DISCONNECTED] = 250,
    [ERROR_FANS_DISCONNECTED]      = 750,
    [ERROR_FEEDBACK]               = ERROR_TIMEOUT_NEVER,
    [ERROR_FEEDBACK_CIRCUITRY]     = ERROR_TIMEOUT_NEVER,
    [ERROR_EEPROM_COMM]            = ERROR_TIMEOUT_NEVER,
    [ERROR_EEPROM_WRITE]           = ERROR_TIMEOUT_NEVER
};

/**
 * @brief Get the timestamp in ms
 * @details Used by the error-utils library
 * 
 * @return uint32_t The timestamp in ms
 */
uint32_t _error_get_timestamp_ms(void) {
    return HAL_GetTick();
}
/** @brief Stop the timer capture compare register and clear interrupts */
void _error_expire_stop() {
        // Stop and reset error timer
    HAL_TIM_OC_Stop_IT(&HTIM_ERR, TIM_CHANNEL_1);
    __HAL_TIM_CLEAR_FLAG(&HTIM_ERR, TIM_IT_CC1);
}
/**
 * @brief Update the timer capture compare register with a new values
 * @details Used by the error-utils library
 * 
 * @param timestamp The timestamp at which the timer should have started
 * @param timeout How much time the timer should wait until it expire
 */
void _error_expire_update(uint32_t timestamp, uint32_t timeout) {
    if (timeout == ERROR_TIMEOUT_NEVER)
        return;

    _error_expire_stop();

    // Set timer capture compare register
    int32_t delta = (int32_t)(timestamp + timeout) - (int32_t)HAL_GetTick();
    uint16_t cnt = __HAL_TIM_GET_COUNTER(&HTIM_ERR);
    __HAL_TIM_SET_COMPARE(&HTIM_ERR, TIM_CHANNEL_1, cnt + TIM_MS_TO_TICKS(&HTIM_ERR, delta));
    
    HAL_TIM_OC_Start_IT(&HTIM_ERR, TIM_CHANNEL_1);
}
/** @brief Enter a critical section */
void _error_cs_enter(void) {
    rPriMask = __get_PRIMASK();
    __set_PRIMASK(1);
}
/** @brief Exit from a critical section */
void _error_cs_exit(void) {
    __set_PRIMASK(rPriMask);
}

void error_init() {
    error_utils_init(
        &error_handler,
        errors,
        expiring,
        ERROR_BUFFER_SIZE,
        _error_get_timestamp_ms,
        error_get_timeout_ms,
        _error_expire_update,
        _error_expire_stop,
        _error_cs_enter,
        _error_cs_exit
    );
}

uint32_t error_get_timeout_ms(uint32_t error) {
    return error_timeout[error];
}

void error_expire_errors() {
    // Stop and reset error timer
    _error_expire_stop();

    error_utils_expire_errors(&error_handler);
}

size_t error_running_count() {
    return error_utils_running_count(&error_handler);
}

size_t error_expired_count() {
    return error_utils_expired_count(&error_handler);
}

// TODO: Remove, for debug purposes only
size_t error_dump(ErrorUtilsRunningInstance * errs[ERROR_BUFFER_SIZE]) {
    size_t tail = 0;
    for (size_t i = 0; i < ERROR_BUFFER_SIZE; ++i) {
        if (errors[i].error != UINT32_MAX)
            errs[tail++] = errors + i;
    }
    return tail;
}
