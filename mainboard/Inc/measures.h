/**
 * @file		mainboard_config.h
 * @brief		This file contains configuration settings for the mainboard
 *
 * @date		Nov 13, 2021
 * @author		Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#ifndef MEASURE_H
#define MEASURE_H

#include "energy/soc.h"
#include "main.h"
#include "mainboard_config.h"
#include "pack/current.h"
#include "tim.h"
#include "usart.h"

#define MEASURE_BASE_INTERVAL_MS 5 // Interval of the timer in ms

// Measure interval based on the base timer interval
typedef enum {
    MEASURE_INTERVAL_10MS  = 10 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_50MS  = 50 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_100MS = 100 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_500MS = 500 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_1S    = 1000 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_5S    = 5000 / MEASURE_BASE_INTERVAL_MS
} MEASURE_INTERVAL;

/** @brief Initialize all measures */
void measures_init();
/** @brief Run checks for each measure */
void measures_check_flags();
/**
 * @brief Timer callback function
 * 
 * @param htim The timer
 */
void _measures_handle_tim_oc_irq(TIM_HandleTypeDef *htim);

#endif // MEASURE_H
