/**
 * @file		error.c
 * @brief		This file contains the functions to handle errors.
 *
 * @date		May 1, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include <error.h>
#include <stdlib.h>
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
ERROR_LIMITS_T timeout[ERROR_NUM_ERRORS] = {
	{LTC6813_PEC_TIMEOUT_COUNT, 0},	{0, CELL_UNDER_VOLTAGE_TIMEOUT_MS},
	{0, CELL_OVER_VOLTAGE_TIMEOUT_MS}, {0, CELL_OVER_TEMPERATURE_TIMEOUT_MS},
	{0, OVER_CURRENT_TIMEOUT_MS},	  {0, CAN_TIMEOUT_MS}};

/**
 * @brief	Initializes an error structure
 *
 * @param	error	The error structure to initialize
 */
void error_init(ERROR_STATUS_T *error) {
	error->type = ERROR_OK;
	error->count = 0;
	error->active = false;
	error->fatal = false;
	error->time_stamp = 0;
}

void er_node_init(er_node_t *node, void *ref, error_t type,
				  uint32_t timestamp) {
	error_init(&node->status);
	error_set(type, &node->status, timestamp);
	node->ref = ref;
	node->next = NULL;
}

bool error_add(er_node_t *head, void *ref, error_t type, uint32_t timestamp) {
	er_node_t *current = head;
	// TODO: Check first element empty

	while (current->next != NULL) {
		current = current->next;
	}

	current->next = malloc(sizeof(er_node_t));
	if (current->next == NULL) {
		return false;
	}

	er_node_init(current->next, ref, type, timestamp);

	return true;
}

void error_remove(void) {}

/**
 * @brief	Activates an error.
 *
 * @param	type	The error type
 * @param	er		The error structure to activate
 * @param	now		The current time
 */
void error_set(error_t type, ERROR_STATUS_T *er, uint32_t now) {
	// If the error is already enabled
	if (er->active) {
		// and it's the same error type
		if (er->type == type) {
			er->count++;
		}
	} else {
		er->type = type;
		er->active = true;
		er->time_stamp = now;
		er->count = 1;
	}
}

/**
 * @brief		Deactivates an error.
 *
 * @param		type	The type of error to deactivate
 * @param		er		The error structure to deactivate
 */
void error_unset(error_t type, ERROR_STATUS_T *er) {
	// Disable only if the types are the same. We don't want to disable
	// different errors
	if (er->type == type) {
		er->type = ERROR_OK;
		er->active = false;
		er->fatal = false;
	}
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
error_t error_check_fatal(ERROR_STATUS_T *error, uint32_t now) {
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
bool _error_check_count(ERROR_STATUS_T *error) {
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
bool _error_check_timeout(ERROR_STATUS_T *error, uint32_t now) {
	if (timeout[error->type].timeout) {
		if (now - error->time_stamp > timeout[error->type].timeout) {
			return true;
		}
	}
	return false;
}
