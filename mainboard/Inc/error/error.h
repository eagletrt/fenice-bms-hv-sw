/**
 * @file error.h
 * @brief Implementation of the error handler required functions
 *
 * @date Mar 21, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

/**
 * @brief Enter a critical section where interrupts can cause problems
 * @details For more info refer to the docs
 */
void error_cs_enter(void);

/**
 * @brief Exit a critical section where interrupts can cause problems
 * @details For more info refer to the docs
 */
void error_cs_exit(void);

/**
 * @brief Notify the handler library that an error has expired
 */
void error_elapsed(void);

#endif /* ERROR_H_ */

