/**
 * @file measurements.h
 * @brief Functions to ensure that all the measurements and error checks
 * happens periodically for a given interval
 * 
 * @date Jul 24, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef MEASURES_H
#define MEASURES_H

#include "stm32l4xx_hal.h"

#define MEASURE_BASE_INTERVAL_MS 5 // Interval of the timer in ms

// Measure interval based on the base timer interval
typedef enum {
    MEASURE_INTERVAL_10MS  = 10 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_200MS = 200 / MEASURE_BASE_INTERVAL_MS
} MEASURE_INTERVAL;

/** @brief Initialize all measures */
void measures_init();
/** @brief Run checks for each measure */
void measures_check_flags();
/**
 * @brief Timer callback function
 * 
 * @param htim The timer that starts the measurement
 */
void _measures_handle_tim_oc_irq(TIM_HandleTypeDef * htim);

#endif // MEASURES_H