/**
 * @file blink.h
 * @brief Blink a led as many times as the index of the current cellboard
 * 
 * @date Feb 21, 2024
 * 
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef BLINK_H
#define BLINK_H

#include "blinky.h"

/** @brief Initialize the blinking behaviour of the led */
void blink_init(void);

/**
 * @brief Get the led state at any given time
 * 
 * @param t The current time
 * @return BlinkyState The current led state
 */
BlinkyState blink_routine(uint32_t t);

#endif  // BLINK_H
