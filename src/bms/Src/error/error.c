/**
 * @file		error.c
 * @brief		This file contains the functions to handle errors.
 *
 * @date		May 1, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "error/error.h"
#include <stdlib.h>
#include "error/list.h"

/**
 * Reaction times by the rules:
 * 	- 500ms for voltage and current
 * 	- 1s for temperatures
 */
#define LTC6813_PEC_TIMEOUT_COUNT 0  // 5000
#define CELL_UNDER_VOLTAGE_TIMEOUT_MS 500
#define CELL_OVER_VOLTAGE_TIMEOUT_MS 500
#define CELL_UNDER_TEMPERATURE_TIMEOUT_MS 1000
#define CELL_OVER_TEMPERATURE_TIMEOUT_MS 1000
#define OVER_CURRENT_TIMEOUT_MS 500  // 400
#define CAN_TIMEOUT_MS 1000

/** @brief	Defines the timeout in count or time for each error type */
error_limits_t timeout[ERROR_NUM_ERRORS] = {
	{LTC6813_PEC_TIMEOUT_COUNT, 0},	{0, CELL_UNDER_VOLTAGE_TIMEOUT_MS},
	{0, CELL_OVER_VOLTAGE_TIMEOUT_MS}, {0, CELL_OVER_TEMPERATURE_TIMEOUT_MS},
	{0, OVER_CURRENT_TIMEOUT_MS},	  {0, CAN_TIMEOUT_MS}};

er_node_t *er_list;

/**
 * @brief	Initializes an error structure
 *
 * @param	error	The error structure to initialize
 */
void error_init(error_status_t *error) {
	error->type = ERROR_OK;
	error->count = 0;
	error->active = false;
	error->fatal = false;
	error->time_stamp = 0;
}

/**
 * @brief	Activates an error.
 *
 * @param	type	The error type
 * @param	er		The error structure to activate
 * @param	now		The current time
 */
void error_set(error_t type, void *ref, uint8_t index, uint32_t now) {
	er_node_t *node = list_find(er_list, type, ref, index);

	if (node == NULL) {
		error_status_t er;
		error_init(&er);
		er_node_init(node, ref, index, type);
		node->status = er;

		list_add(er_list, node);
	}

	// If the error is already enabled
	if (node->status.active) {
		node->status.count++;
	} else {
		node->status.type = type;
		node->status.active = true;
		node->status.time_stamp = now;
		node->status.count = 1;
	}
}

/**
 * @brief		Deactivates an error.
 *
 * @param		type	The type of error to deactivate
 * @param		er		The error structure to deactivate
 */
bool error_unset(error_t type, void *ref, uint8_t index) {
	er_node_t *node = list_find(er_list, type, ref, index);

	if (node != NULL) {
		// TODO: Optimize
		return list_remove(&er_list, node);
	}

	return false;
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
	if (error->active) {
		if (_error_check_count(error) || _error_check_timeout(error, now)) {
			error->fatal = true;
			return error->type;
		}
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
