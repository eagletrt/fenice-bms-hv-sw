/**
 * @file		error.h
 * @brief		This file contains the functions to handle errors.
 *
 * @date		May 1, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef ERROR_H_
#define ERROR_H_

#include <inttypes.h>
#include <stdbool.h>

/**
 * @brief		Checks if an error has been triggered, if true it jumps to the
 * 					"End" label
 * @details	This macro checks if a function raised an error. You should call
 * 					this right after a function that can raise some type of
 * 					error.
 */
#define ER_CHK(ST_P)             \
	/*1*/ {                      \
		if (*ST_P != ERROR_OK) { \
			/*3*/ goto End;      \
		}                        \
	}

/** @brief		Sets the error type and jumps to the "End" label
 *	@details	This macro should be called inside a function that can generate
 *						an error. It's used to set the error variable and to
 *						break execution of the function
 */
#define ER_RAISE(ST_P, ST) \
	/*1*/ {                \
		*ST_P = ST;        \
		/*3*/ goto End;    \
	}

typedef enum error {
	ERROR_OK,
	ERROR_LTC_PEC_ERROR,

	ERROR_CELL_UNDER_VOLTAGE,
	ERROR_CELL_OVER_VOLTAGE,
	ERROR_CELL_OVER_TEMPERATURE,

	ERROR_OVER_CURRENT,
	ERROR_CAN,

	ERROR_NUM_ERRORS
} error_t;

typedef enum {
	WARN_CELL_LOW_VOLTAGE,
	WARN_CELL_DROPPING,
	WARN_PRECHARGE_FAIL,

	WARN_NUM_WARNINGS,
	WARN_OK
} warning_t;

extern const char *error_names[ERROR_NUM_ERRORS];

/** @brief	Defines the acceptable thresholds over which an error becomes
 * 					critical. Can be a count based limit, a time based one
 * or both. 0 values are ignored
 */
typedef struct ERROR_LIMITS_T {
	uint16_t count;
	uint32_t timeout;
} ERROR_LIMITS_T;

/** @brief	Defines an error instance */
typedef struct ERROR_STATUS {
	error_t type;		 /*!< Defines the type of error */
	bool active;		 /*!< True if the error is currently happening */
	bool fatal;			 /*!< True if the error is fatal */
	uint16_t count;		 /*!< How many times the error has occurred */
	uint32_t time_stamp; /*!< Last time the error activated */
} ERROR_STATUS_T;

/** @brief	tuple of value-error_status. Used to store values that can trigger
 * 					an error
 */
typedef struct ER_INT16 {
	int16_t value;
	ERROR_STATUS_T error;
} ER_INT16_T;

typedef struct er_node {
	void *ref;
	ERROR_STATUS_T status;
	struct er_node *next;
} er_node_t;

bool _error_check_count(ERROR_STATUS_T *error);
bool _error_check_timeout(ERROR_STATUS_T *error, uint32_t time);

void error_init(ERROR_STATUS_T *error);

bool error_add(er_node_t *head, void *ref, error_t type, uint32_t time_stamp);

void error_set(error_t type, ERROR_STATUS_T *error, uint32_t time_stamp);
void error_unset(error_t type, ERROR_STATUS_T *error);
error_t error_check_fatal(ERROR_STATUS_T *error, uint32_t now);

#endif /* ERROR_H_ */
