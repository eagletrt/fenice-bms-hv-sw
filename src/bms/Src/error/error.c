/**
 * @file		error.c
 * @brief		This file contains the functions to handle errors.
 *
 * @date		May 1, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "error/error.h"

#include <stdlib.h>

#include "error/error_list_ref.h"
#include "tim.h"

/**
 * Reaction times by the rules:
 * 	- 500ms for voltages and current
 * 	- 1s for temperatures
 */
const error_timeout error_timeouts[ERROR_NUM_ERRORS] = {
	[ERROR_LTC_PEC_ERROR] = SOFT,
	[ERROR_CELL_UNDER_VOLTAGE] = SHORT,
	[ERROR_CELL_OVER_VOLTAGE] = SHORT,
	[ERROR_OVER_CURRENT] = SHORT,
	[ERROR_CAN] = REGULAR,
	[ERROR_ADC_INIT] = SOFT,
	[ERROR_ADC_TIMEOUT] = SOFT,
	[ERROR_FEEDBACK_HARD] = INSTANT,
	[ERROR_FEEDBACK_SOFT] = SOFT};

llist er_list = NULL;

/**
 * @returns The time left before the error becomes fatal
 */
uint32_t get_timeout_delta(error_t *error) {
	uint32_t delta = error->timestamp + error_timeouts[error->id] - HAL_GetTick();
	return delta >= 0 ? delta : 0;
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
	if (get_timeout_delta((error_t *)a) < get_timeout_delta((error_t *)b))
		return 1;
	if (get_timeout_delta((error_t *)a) == get_timeout_delta((error_t *)b))
		return 0;
	return -1;
}

bool error_set_timer(error_t *error) {
	HAL_TIM_Base_Stop_IT(&htim_Err);

	if (error != NULL && error->state == ERROR_ACTIVE) {
		// Set counter period register to the delta
		htim_Err.Instance->ARR = get_timeout_delta(error) - 1;
		HAL_TIM_Base_Start_IT(&htim_Err);

		return true;
	}

	return false;
}

void error_init() {
	er_list = llist_init(error_compare, error_equals);
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
	error->id = id;
	error->offset = offset;
	error->state = ERROR_ACTIVE;
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

		//ERROR_GET_REF(id, offset) = (llist_node)error;
		(*error_list_ref_array_element(id, offset)) = (llist_node)error;
		//error_list_ref_array[id][offset] = (llist_node)error;
		// Re-set timer if first in list
		if (error_equals(llist_get_head(er_list), error)) {
			error_set_timer(error);
		}
	}

	return true;
}

/**
 * @brief		Deactivates an error
 *
 * @param		id		The id of error to deactivate
 * @param		offset	The offset of the error id
 * 
 * @returns	whether the error has been unset
 */
bool error_unset(error_id id, uint8_t offset) {
	if (*error_list_ref_array_element(id, offset) != NULL) {
		error_t *error = (error_t *)(*error_list_ref_array_element(id, offset));

		// Check if error is fatal; in that case do not remove it, but deactivate it
		if (error->state == ERROR_FATAL) {
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

/**
 * @returns	The number of currently running errors
 */
// TODO: Remove
uint8_t error_count() {
	return llist_size(er_list);
}

/**
 * @returns	An array of running errors
 */
size_t error_dump(error_t errors[]) {
	llist_export(er_list, (llist_node *)errors);
	return llist_size(er_list);
}
