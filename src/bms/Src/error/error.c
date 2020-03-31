/**
 * @file		error.c
 * @brief		This file contains the functions to handle errors.
 *
 * @date		May 1, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "error/error.h"

#include "error/error_list_ref.h"

/**
 * Reaction times by the rules:
 * 	- 500ms for voltage and current
 * 	- 1s for temperatures
 */
#define LTC6813_PEC_TIMEOUT_COUNT 0	 // 5000
#define CELL_UNDER_VOLTAGE_TIMEOUT_MS 500
#define CELL_OVER_VOLTAGE_TIMEOUT_MS 500
#define CELL_UNDER_TEMPERATURE_TIMEOUT_MS 1000
#define CELL_OVER_TEMPERATURE_TIMEOUT_MS 1000
#define OVER_CURRENT_TIMEOUT_MS 500	 // 400
#define CAN_TIMEOUT_MS 1000

/** @brief	Defines the timeout in count or time for each error type */
error_limits_t timeout[ERROR_NUM_ERRORS] = {
	{LTC6813_PEC_TIMEOUT_COUNT, 0}, {0, CELL_UNDER_VOLTAGE_TIMEOUT_MS}, {0, CELL_OVER_VOLTAGE_TIMEOUT_MS}, {0, CELL_OVER_TEMPERATURE_TIMEOUT_MS}, {0, OVER_CURRENT_TIMEOUT_MS}, {0, CAN_TIMEOUT_MS}};

node_t *er_list = NULL;

/**
 * @brief	Initializes an error structure
 *
 * @param	error			The error structure to initialize
 * @param type			The error type
 * @param	offset		The offset (index) of the error in error_data
 * @param	timestamp	The timestamp at which the error occurred
 */
void error_init(error_status_t *error, error_t type, uint8_t offset, uint32_t timestamp) {
	error->type = type;
	error->offset = offset;
	error->count = 0;
	error->fatal = false;
	error->active = true;
	error->time_stamp = timestamp;
}

/**
 * @brief	Activates an error
 * 
 * @details	
 *
 * @param	type			The error type
 * @param	offset		The offset (index) of the error in error_data
 * @param	timestamp	Current timestamp
 * 
 * @returns	whether the error has been set
 */
bool error_set(error_t type, uint8_t offset, uint32_t timestamp) {
	// Check if error exists
	if (error_list_ref_array[type][offset] == NULL) {
		error_status_t er;
		error_init(&er, type, offset, timestamp);

		error_list_ref_array[type][offset] = list_insert(&er_list, &er, sizeof(error_status_t));

		if (error_list_ref_array[type][offset] == NULL) {
			return false;
		}

		return true;
	}

	((error_status_t *)error_list_ref_array[type][offset]->data)->count++;
	return true;
}

/**
 * @brief		Deactivates an error
 *
 * @param		type		The type of error to deactivate
 * @param		offset	The offset of the error type
 * 
 * @returns	whether the error has been unset
 */
bool error_unset(error_t type, uint8_t offset) {
	// Check if error is fatal; in that case do not remove it, but deactivate it
	if (error_list_ref_array[type][offset] != NULL) {
		if (((error_status_t *)error_list_ref_array[type][offset]->data)->fatal) {
			((error_status_t *)error_list_ref_array[type][offset]->data)->active = false;
			return true;
		}

		list_remove(&er_list, error_list_ref_array[type][offset]);

		error_list_ref_array[type][offset] = NULL;

		return true;
	}

	return false;
}

/**
 * @returns	The number of currently running errors
 */
// TODO: Remove
uint8_t error_count() {
	return list_count(er_list);
}

/**
 * @returns the current array of errors
 */
// TODO: Make
uint16_t error_dump(error_status_t errors[]) {
	node_t *node = er_list;
	uint16_t count = 0;

	while (node != NULL) {
		errors[count++] = *(error_status_t *)node->data;
		node = node->next;
	}

	return count;
}

/**
 * @brief Checks every error for expiration
 * 
 * @returns	The expired error type, or ERROR_OK
 */
error_t error_verify(uint32_t now) {
	node_t *node = er_list;
	error_t ret = ERROR_OK;

	while (node != NULL && ret == ERROR_OK) {
		ret = error_check_fatal((error_status_t *)(node->data), now);

		node = node->next;
	}

	return ret;
}

/**
 * @brief		Checks if an error has become fatal.
 * @details	This function checks if the provided error structure has exceeded
 * 					one or more of its critical limits.
 *
 * @param		error	The error structure to check.
 * @param		now		The current time.
 *
 * @retval	The error return value.
 */
error_t error_check_fatal(error_status_t *error, uint32_t now) {
	if (_error_check_count(error) || _error_check_timeout(error, now)) {
		error->fatal = true;
		return error->type;
	}

	return ERROR_OK;
}

/**
 * @brief		Checks whether to trigger a count-based error.
 * @details	This will trigger the error if the number of occurrences exceeds the
 * 					count parameter of that error type.
 *
 * @param		error	The error structure to check
 *
 * @retval	true for error, false for OK
 */
bool _error_check_count(error_status_t *error) {
	if (timeout[error->type].count) {
		/** Compares the actual count to the timeout for this error type */
		if (error->count > timeout[error->type].count) {
			return true;
		}
	}

	return false;
}

/**
 * @brief		Checks whether to trigger a time-based error.
 * @details	This will trigger the error if the time elapsed between the first
 * 					occurrence of the error and the current time is more than
 * 					the timeout for that type of error.
 *
 * @param		error	The error struct to check
 * @param		now		The current time
 *
 * @retval	true for error, false for OK
 */
bool _error_check_timeout(error_status_t *error, uint32_t now) {
	if (timeout[error->type].timeout) {
		if (now - error->time_stamp > timeout[error->type].timeout) {
			return true;
		}
	}
	return false;
}
