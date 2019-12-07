/**
 * @file		error_def.h
 * @brief		This file contains the common definitions for error management
 *
 * @date		Dec 7, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef ERROR_DEF_H
#define ERROR_DEF_H

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

typedef enum warning {
	WARN_CELL_LOW_VOLTAGE,
	WARN_CELL_DROPPING,
	WARN_PRECHARGE_FAIL,

	WARN_NUM_WARNINGS,
	WARN_OK
} warning_t;

/** @brief	Defines the acceptable thresholds over which an error becomes
 * 					critical. Can be a count based limit, a time based one
 * or both. 0 values are ignored
 */
typedef struct error_limits {
	uint16_t count;
	uint32_t timeout;
} error_limits_t;

/** @brief	Defines an error instance */
typedef struct error_status {
	error_t type;		 /*!< Defines the type of error */
	bool active;		 /*!< True if the error is currently happening */
	bool fatal;			 /*!< True if the error is fatal */
	uint16_t count;		 /*!< How many times the error has occurred */
	uint32_t time_stamp; /*!< Last time the error activated */
} error_status_t;

#endif