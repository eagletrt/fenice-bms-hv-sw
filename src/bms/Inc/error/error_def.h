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
 * @brief	Error type definitions
 * 
 * @details	To add an error type you need to add it to this enum, then you need to create the error reference variable in error_data.h
 * 					and you need to link the error to the reference in the error_reference array.
 */
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

/** @details	Defines the acceptable thresholds over which an error becomes
 * 					critical. Can be a count based limit, a time based one
 * 					or both. 0 values are ignored
 */
typedef struct error_limits {
	uint16_t count;
	uint32_t timeout;
} error_limits_t;

/** @brief	Defines an error instance */
typedef struct error_status {
	error_t type;		 /* Defines the type of error */
	uint8_t offset;		 /* Identifies different instances of a type */
	bool fatal;			 /* True if the error is fatal */
	bool active;		 /* True if the error is currently active */
	uint32_t count;		 /* How many times the error has occurred */
	uint32_t time_stamp; /* Last time the error activated */
} error_status_t;

#endif