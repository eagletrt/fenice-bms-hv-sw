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
#include "pack/voltage.h"
#include "tim.h"
#include "usart.h"

#define _BASE_INTERVAL  10U
#define _10MS_INTERVAL  10U
#define _50MS_INTERVAL  50U
#define _100MS_INTERVAL 100U
#define _500MS_INTERVAL 500U
#define _1S_INTERVAL    1000U
#define _5S_INTERVAL    5000U

enum {
    _10MS_INTERVAL_FLAG  = 1,
    _50MS_INTERVAL_FLAG  = 2,
    _100MS_INTERVAL_FLAG = 4,
    _500MS_INTERVAL_FLAG = 8,
    _1S_INTERVAL_FLAG    = 16,
    _5S_INTERVAL_FLAG    = 32
};

typedef uint8_t timebase_flags_t;

void timebase_init();
void timebase_voltage_current_soc();
void timebase_check_flags();
void _timebase_handle_tim_elapsed_handler(TIM_HandleTypeDef *htim);