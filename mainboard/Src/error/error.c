/**
 * @file		error.c
 * @brief		This file contains the functions to handle errors.
 *
 * @date		May 1, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "error/error.h"

#include "error/error_list_ref.h"
#include "mainboard_config.h"
#include "tim.h"

#include <stdlib.h>
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
/**
 * Reaction times by the rules:
 * 	- 500ms for voltages and current
 * 	- 1s for temperatures
 */
const error_timeout error_timeouts[ERROR_NUM_ERRORS] = {
    [ERROR_CELL_LOW_VOLTAGE]      = SOFT,
    [ERROR_CELL_UNDER_VOLTAGE]    = 500,
    [ERROR_CELL_OVER_VOLTAGE]     = 500,
    [ERROR_CELL_HIGH_TEMPERATURE] = SOFT,
    [ERROR_CELL_OVER_TEMPERATURE] = 1000,
    [ERROR_OVER_CURRENT]          = 500,
    [ERROR_CAN]                   = 1000,
    [ERROR_ADC_INIT]              = SOFT,
    [ERROR_ADC_TIMEOUT]           = SOFT,
    [ERROR_INT_VOLTAGE_MISMATCH]  = SOFT,
    [ERROR_CELLBOARD_COMM]        = 500,
    [ERROR_CELLBOARD_INTERNAL]    = 500,
    [ERROR_FEEDBACK]              = 500,
    [ERROR_EEPROM_COMM]           = SOFT,
    [ERROR_EEPROM_WRITE]          = SOFT};

llist er_list = NULL;

/**
 * @returns The time left before the error becomes fatal
 */
uint32_t get_timeout_delta(error_t *error) {
    uint32_t delta = error->timestamp + error_timeouts[error->id] - HAL_GetTick();
    return delta <= error_timeouts[error->id] ? delta : 0;
}

bool error_equals(llist_node a, llist_node b) {
    return ((error_t *)a)->id == ((error_t *)b)->id && ((error_t *)a)->offset == ((error_t *)b)->offset;
}

/**
 * @brief Compares the timeout delta between two errors
 * 
 * @returns 1 if a < b. 0 if a == b. -1 otherwise
 */
int8_t error_compare(llist_node a, llist_node b) {
    // TODO: check if error is soft
    if (get_timeout_delta((error_t *)a) < get_timeout_delta((error_t *)b))
        return 1;
    if (get_timeout_delta((error_t *)a) == get_timeout_delta((error_t *)b))
        return 0;
    return -1;
}

bool error_set_timer(error_t *error) {
    HAL_TIM_Base_Stop_IT(&HTIM_ERR);
    HAL_TIM_OC_Stop_IT(&HTIM_ERR, TIM_CHANNEL_1);
    HAL_GPIO_WritePin(GPIO1_GPIO_Port, GPIO1_Pin, GPIO_PIN_RESET);

    if (error != NULL && error->state == STATE_WARNING && error_timeouts[error->id] < SOFT) {
        // Set counter period register to the delta
        HAL_TIM_Base_Start_IT(&HTIM_ERR);
        HAL_TIM_OC_Start_IT(&HTIM_ERR, TIM_CHANNEL_1);

        volatile uint16_t delta = (get_timeout_delta(error) * 10U);
        uint16_t pulse          = HAL_TIM_ReadCapturedValue(&HTIM_ERR, TIM_CHANNEL_1);
        __HAL_TIM_SET_COMPARE(&HTIM_ERR, TIM_CHANNEL_1, pulse + delta);
        HAL_GPIO_WritePin(GPIO1_GPIO_Port, GPIO1_Pin, GPIO_PIN_SET);

        return true;
    } else {
    }

    return false;
}

void error_init() {
    er_list = llist_init(error_compare, error_equals);
    HAL_TIM_Base_Stop_IT(&HTIM_ERR);
}

/**
 * @brief	Initializes an error structure
 *
 * @param	error			The error structure to initialize
 * @param id				The error id
 * @param	offset		The offset (index) of the error in error_data
 * @param	timestamp	The timestamp at which the error occurred
 */
void error_init_error(error_t *error, error_id id, uint8_t offset, uint32_t timestamp) {
    error->id        = id;
    error->offset    = offset;
    error->state     = STATE_WARNING;
    error->timestamp = timestamp;
}

/**
 * @brief	Activates an error
 * 
 * @details	
 *
 * @param	id			The error id
 * @param	offset		The offset (index) of the error in error_data
 * @param	timestamp	Current timestamp
 * 
 * @returns	whether the error has been set
 */
bool error_set(error_id id, uint8_t offset, uint32_t timestamp) {
    // Check if error exists
    if (*error_list_ref_array_element(id, offset) == NULL) {
        error_t *error = malloc(sizeof(error_t));

        if (error == NULL) {
            return false;
        }

        error_init_error(error, id, offset, timestamp);

        if (llist_insert_priority(er_list, (llist_node)error) != LLIST_SUCCESS) {
            return false;
        }

        (*error_list_ref_array_element(id, offset)) = (llist_node)error;

        // Re-set timer if first in list
        if (error_equals(llist_get_head(er_list), error)) {
            error_set_timer(error);
        }
    }

    return true;
}

error_t *error_get_top() {
    return (error_t *)llist_get_head(er_list);
}
bool error_set_fatal(error_t *error) {
    if (error != NULL && error->state != STATE_FATAL) {
        error->state = STATE_FATAL;
        return true;
    }
    return false;
}

/**
 * @brief		Deactivates an error
 *
 * @param		id		The id of error to deactivate
 * @param		offset	The offset of the error id
 * 
 * @returns	whether the error has been unset
 */
bool error_reset(error_id id, uint8_t offset) {
    if (*error_list_ref_array_element(id, offset) != NULL) {
        error_t *error = (error_t *)(*error_list_ref_array_element(id, offset));

        // Check if error is fatal; in that case do not remove it
        if (error->state == STATE_FATAL) {
            //error->state = ERROR_NOT_ACTIVE;
            return true;
        }

        // If we are removing the first error, re-set the timer to the second error
        if (error_equals(llist_get_head(er_list), (llist_node)error)) {
            error_t *tmp = NULL;

            // No need to check llist_get output because passing NULL to error_set_timer is legal
            llist_get(er_list, 1, (llist_node *)&tmp);
            error_set_timer(tmp);
        }

        if (llist_remove_by_node(er_list, (llist_node)error) != LLIST_SUCCESS) {
            return false;
        }

        //ERROR_GET_REF(id, offset) = NULL;
        (*error_list_ref_array_element(id, offset)) = NULL;
        //error_list_ref_array[id][offset] = NULL;

        return true;
    }

    return false;
}

size_t error_get_fatal() {
    size_t count = error_count();
    error_t errors[ERROR_NUM_ERRORS];
    error_dump(errors);

    size_t fatal = 0;
    for (size_t i = 0; i < count; i++) {
        if (errors[i].state == STATE_FATAL) {
            fatal++;
        }
    }

    return fatal;
}

/**
 * @returns	The number of currently running errors
 */
size_t error_count() {
    return llist_size(er_list);
}

/**
 * @returns	An array of running errors
 */
void error_dump(error_t errors[]) {
    llist_export(er_list, (void *)errors, sizeof(error_t));
}
