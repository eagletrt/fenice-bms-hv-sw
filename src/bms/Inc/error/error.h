/**
 * @file		error.h
 * @brief		This file contains the functions to handle errors.
 *
 * @date		May 1, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef ERROR_H
#define ERROR_H

#include <inttypes.h>
#include <stdbool.h>
#include "error/error_def.h"

extern const char *error_names[ERROR_NUM_ERRORS];

bool _error_check_count(error_status_t *error);
bool _error_check_timeout(error_status_t *error, uint32_t time);

void error_init(error_status_t *error);

void error_set(error_t type, void *ref, uint8_t index, uint32_t now);
bool error_unset(error_t type, void *ref, uint8_t index);
error_t error_check_fatal(error_status_t *error, uint32_t now);

#endif /* ERROR_H_ */
