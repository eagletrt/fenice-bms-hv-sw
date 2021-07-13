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

#define ERROR_SET(e)   (errors = errors | (1 << e))
#define ERROR_UNSET(e) (errors = errors & (~(1 << e)))
#define ERROR_GET(e)   ((errors & (1 << e)) != 0)

typedef enum { ERROR_LTC_COMM, ERROR_TEMP_COMM } error_types;

extern uint16_t errors;

#endif