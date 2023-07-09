/**
 * @file		mainboard_config.h
 * @brief		This file contains configuration settings for the mainboard
 *
 * @date		Nov 13, 2021
 * @author		Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#pragma once

#include "energy/soc.h"
#include "main.h"
#include "mainboard_config.h"
#include "pack/current.h"
#include "tim.h"
#include "usart.h"

#define MEASURE_BASE_INTERVAL_MS 5 // Interval of the timer in ms

typedef enum {
    MEASURE_INTERVAL_10MS  = 10 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_50MS  = 50 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_100MS = 100 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_500MS = 500 / MEASURE_BASE_INTERVAL_MS,
    MEASURE_INTERVAL_5S    = 5000 / MEASURE_BASE_INTERVAL_MS
} MEASURE_INTERVAL;

typedef uint8_t measures_flags_t;

void measures_init();
void measures_check_flags();
void _measures_handle_tim_oc_irq(TIM_HandleTypeDef *htim);