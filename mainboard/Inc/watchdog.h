#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdbool.h>
#include <inttypes.h>

/** @brief Initialize watchdogs */
void watchdog_init();

/**
 * @brief Check if any of the watchdogs has timed out
 * 
 * @return true If a watchdog has timed out
 * @return false Otherwise
 */
bool is_watchdog_timed_out();

/**
 * @brief Reset the watchdog timer
 * 
 * @param id The id
 */
void watchdog_reset(uint16_t id);

/**
 * @brief Watchdog routine used to check for timeout
 * @details This function should be called periodically
 * */
void watchdog_routine();

#endif // WATCHDOG_H