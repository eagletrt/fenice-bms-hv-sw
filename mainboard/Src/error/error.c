/**
 * @file error.c
 * @brief Error handling functions and structures
 * 
 * @date Jul 08, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "error/error.h"

#include "micro-libs/error-utils/error_utils.h"
#include "../../fenice_config.h"
#include "mainboard_config.h"

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

ERROR_UTILS_InstanceTypeDef error_instance_cell_low_voltage[1];
ERROR_UTILS_InstanceTypeDef error_instance_cell_under_voltage[1];
ERROR_UTILS_InstanceTypeDef error_instance_cell_over_voltage[1];
ERROR_UTILS_InstanceTypeDef error_instance_cell_high_temperature[1];
ERROR_UTILS_InstanceTypeDef error_instance_cell_over_temperature[1];
ERROR_UTILS_InstanceTypeDef error_instance_over_current[1];
ERROR_UTILS_InstanceTypeDef error_instance_can_comm[1];
ERROR_UTILS_InstanceTypeDef error_instance_voltage_mismatch[1];
ERROR_UTILS_InstanceTypeDef error_instance_cellboard_comm[CELLBOARD_COUNT];
ERROR_UTILS_InstanceTypeDef error_instance_cellboard_internal[CELLBOARD_COUNT];
ERROR_UTILS_InstanceTypeDef error_instance_connector_disconnected[1];
ERROR_UTILS_InstanceTypeDef error_instance_fans_disconnected[1];
ERROR_UTILS_InstanceTypeDef error_instance_feedback[FEEDBACK_N];
ERROR_UTILS_InstanceTypeDef error_instance_feedback_circuitry[FEEDBACK_N];
ERROR_UTILS_InstanceTypeDef error_instance_eeprom_comm[1];
ERROR_UTILS_InstanceTypeDef error_instance_eeprom_write[1];

ERROR_UTILS_ErrorTypeDef error_types[ERROR_COUNT] = {
    [ERROR_CELL_LOW_VOLTAGE] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_CELL_LOW_VOLTAGE],
        .instances = error_instance_cell_low_voltage,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_CELL_UNDER_VOLTAGE] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_CELL_UNDER_VOLTAGE],
        .instances = error_instance_cell_under_voltage,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_CELL_OVER_VOLTAGE] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_CELL_OVER_VOLTAGE],
        .instances = error_instance_cell_over_voltage,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_CELL_HIGH_TEMPERATURE] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_CELL_HIGH_TEMPERATURE],
        .instances = error_instance_cell_high_temperature,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_CELL_OVER_TEMPERATURE] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_CELL_OVER_TEMPERATURE],
        .instances = error_instance_cell_over_temperature,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_OVER_CURRENT] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_OVER_CURRENT],
        .instances = error_instance_over_current,
        .instances_length = 1,
        .toggle_callback = NULL,
    },
    [ERROR_CAN_COMM] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_CAN_COMM],
        .instances = error_instance_can_comm,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_VOLTAGE_MISMATCH] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_VOLTAGE_MISMATCH],
        .instances = error_instance_voltage_mismatch,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_CELLBOARD_COMM] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_CELLBOARD_COMM],
        .instances = error_instance_cellboard_comm,
        .instances_length = CELLBOARD_COUNT,
        .toggle_callback = NULL
    },
    [ERROR_CELLBOARD_INTERNAL] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_CELLBOARD_INTERNAL],
        .instances = error_instance_cellboard_internal,
        .instances_length = CELLBOARD_COUNT,
        .toggle_callback = NULL
    },
    [ERROR_CONNECTOR_DISCONNECTED] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_CONNECTOR_DISCONNECTED],
        .instances = error_instance_connector_disconnected,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_FANS_DISCONNECTED] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_FANS_DISCONNECTED],
        .instances = error_instance_fans_disconnected,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_FEEDBACK] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_FEEDBACK],
        .instances = error_instance_feedback,
        .instances_length = FEEDBACK_N,
        .toggle_callback = NULL
    },
    [ERROR_FEEDBACK_CIRCUITRY] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_FEEDBACK_CIRCUITRY],
        .instances = error_instance_feedback_circuitry,
        .instances_length = FEEDBACK_N,
        .toggle_callback = NULL
    },
    [ERROR_EEPROM_COMM] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_EEPROM_COMM],
        .instances = error_instance_eeprom_comm,
        .instances_length = 1,
        .toggle_callback = NULL
    },
    [ERROR_EEPROM_WRITE] = {
        .expiry_callback = NULL,
        .expiry_delay_ms = error_timeout[ERROR_EEPROM_WRITE],
        .instances = error_instance_eeprom_write,
        .instances_length = 1,
        .toggle_callback = NULL
    }
};

ERROR_UTILS_HandleTypeDef error_handler;

HAL_StatusTypeDef error_init() {
    return error_utils_init(&error_handler, &HTIM_ERR, error_types, ERROR_COUNT, NULL, NULL);
}
HAL_StatusTypeDef error_set(size_t index, size_t offset) {
    return error_utils_error_set(&error_handler, index, offset);
}
HAL_StatusTypeDef error_reset(size_t index, size_t offset) {
    return error_utils_error_reset(&error_handler, index, offset);
}
size_t error_running_count() {
    return error_utils_get_running_count(&error_handler);
}
size_t error_expired_count() {
    return error_utils_get_expired_count(&error_handler);
}
HAL_StatusTypeDef error_reset_all_expired() {
    return error_utils_reset_all_expired(&error_handler);
}
HAL_StatusTypeDef error_timer_elapsed_callback(TIM_HandleTypeDef * tim) {
    return ERROR_UTILS_TimerElapsedCallback(&error_handler, tim);
}