/**
 * @file		error.c
 * @brief		This file contains the functions to handle errors.
 *
 * @date		May 1, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include <stddef.h>

#include "error/error.h"
#include "error-utils/error_utils.h"
#include "bms_config.h"


ERROR_UTILS_HandleTypeDef error_handle;
ERROR_UTILS_ErrorTypeDef errors[ERROR_COUNT];

// TODO: Set error instances count
ERROR_UTILS_InstanceTypeDef cell_low_voltage_inst[1];
ERROR_UTILS_InstanceTypeDef cell_under_voltage_inst[1];
ERROR_UTILS_InstanceTypeDef cell_over_voltage_inst[1];
ERROR_UTILS_InstanceTypeDef cell_over_temperature_inst[1];
ERROR_UTILS_InstanceTypeDef cell_high_temperature_inst[1];
ERROR_UTILS_InstanceTypeDef over_current_inst[1];
ERROR_UTILS_InstanceTypeDef can_inst[1];
ERROR_UTILS_InstanceTypeDef int_voltage_mismatch_inst[1];
ERROR_UTILS_InstanceTypeDef cellboard_comm_inst[1];
ERROR_UTILS_InstanceTypeDef cellboard_internal_inst[1];
ERROR_UTILS_InstanceTypeDef feedback_inst[1];
ERROR_UTILS_InstanceTypeDef feedback_circuitry_inst[1];
ERROR_UTILS_InstanceTypeDef eeprom_comm_inst[1];
ERROR_UTILS_InstanceTypeDef eeprom_write_inst[1];

ERROR_UTILS_InstanceTypeDef * instances[] = {
    [ERROR_CELL_LOW_VOLTAGE]      = cell_low_voltage_inst,
    [ERROR_CELL_UNDER_VOLTAGE]    = cell_under_voltage_inst,
    [ERROR_CELL_OVER_VOLTAGE]     = cell_over_voltage_inst,
    [ERROR_CELL_OVER_TEMPERATURE] = cell_over_temperature_inst,
    [ERROR_CELL_HIGH_TEMPERATURE] = cell_high_temperature_inst,
    [ERROR_OVER_CURRENT]          = over_current_inst,
    [ERROR_CAN]                   = can_inst,
    [ERROR_INT_VOLTAGE_MISMATCH]  = int_voltage_mismatch_inst,
    [ERROR_CELLBOARD_COMM]        = cellboard_comm_inst,
    [ERROR_CELLBOARD_INTERNAL]    = cellboard_internal_inst,
    [ERROR_FEEDBACK]              = feedback_inst,
    [ERROR_FEEDBACK_CIRCUITRY]    = feedback_circuitry_inst,
    [ERROR_EEPROM_COMM]           = eeprom_comm_inst,
    [ERROR_EEPROM_WRITE]          = eeprom_write_inst
};

// TODO: Init error instances
HAL_StatusTypeDef error_init() {
    // for (size_t i = 0; i < ERROR_COUNT; i++) {
    //     errors[i].toggle_callback = NULL;
    //     errors[i].expiry_callback = NULL;
    //     errors[i].expiry_delay_ms = error_timeout[i];
    //     errors[i].instances = instances[i];
    //     errors[i].instances_length = ;
    // }

    return error_utils_init(&error_handle,
        &HTIM_ERR,
        errors,
        ERROR_COUNT,
        NULL,
        NULL);
}


HAL_StatusTypeDef error_set(ERROR_TYPE type, size_t instance) {
    return error_utils_error_set(&error_handle, type, instance);
}
HAL_StatusTypeDef error_reset(ERROR_TYPE type, size_t instance) {
    return error_utils_error_reset(&error_handle, type, instance);
}

size_t error_count() {
    return error_utils_get_count(&error_handle);
}
