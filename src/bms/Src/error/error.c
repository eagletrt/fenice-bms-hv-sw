/**
 * @file		error.c
 * @brief		This file contains the functions to handle errors.
 *
 * @date		May 1, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "error/error.h"

#include <stdlib.h>

#include "error/error_data.h"
#include "error/list.h"

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

er_node_t *er_list = NULL;

/**
 * @brief	Initializes an error structure
 *
 * @param	error	The error structure to initialize
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
 */
bool error_set(error_t type, uint8_t offset, uint32_t timestamp) {
	// Check if error exists
	if (error_reference[type][offset] == NULL) {
		error_status_t *er = (error_status_t *)malloc(sizeof(error_status_t));

		error_init(er, type, offset, timestamp);

		error_reference[type][offset] = list_insert(&er_list, er);

		return true;
	}

	error_reference[type][offset]->status.count++;
	return false;
}

/**
 * @brief		Deactivates an error.
 *
 * @param		type	The type of error to deactivate
 * @param		er		The error structure to deactivate
 */
bool error_unset(error_t type, uint8_t offset) {
	// Check if error is fatal; in that case do not remove it, but deactivate it
	if (error_reference[type][offset] != NULL) {
		if (error_reference[type][offset]->status.fatal) {
			error_reference[type][offset]->status.active = false;
			return true;
		}

		list_remove(error_reference[type][offset]);

		error_reference[type][offset] = NULL;

		return true;
	}

	return false;
}

uint8_t error_count() {
	er_node_t *node = er_list;
	uint8_t count = 0;

	while (node != NULL) {
		count++;
		node = node->next;
	}

	return count;
}

uint16_t error_dump(error_status_t errors[]) {
	er_node_t *node = er_list;
	uint16_t count = 0;

	while (node != NULL) {
		errors[count++] = node->status;
		node = node->next;
	}

	return count;
}

error_t error_verify(uint32_t now) {
	er_node_t *node = er_list;
	error_t ret = ERROR_OK;

	while (node != NULL && ret == ERROR_OK) {
		ret = error_check_fatal(&(node->status), now);

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
