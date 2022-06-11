/**
 * @file		error.h
 * @brief		Small error management lib
 *
 * @date		Jul 07, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef ERROR_H
#define ERROR_H

#include <inttypes.h>

#define ERROR_SET(e)   (errors = errors | (1 << (e)))
#define ERROR_UNSET(e) (errors = errors & (~(1 << (e))))
#define ERROR_GET(e)   ((errors & (1 << (e))) != 0)

typedef enum {
    ERROR_CAN = 0,
    ERROR_LTC_COMM,
    ERROR_TEMP_COMM_0,
    ERROR_TEMP_COMM_1,
    ERROR_TEMP_COMM_2,
    ERROR_TEMP_COMM_3,
    ERROR_TEMP_COMM_4,
    ERROR_TEMP_COMM_5,
    ERROR_OPEN_WIRE
} error_types;

typedef uint16_t error_t;

extern error_t errors;

#endif