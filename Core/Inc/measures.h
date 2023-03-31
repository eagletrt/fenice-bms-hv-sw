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

#define _50MS_INTERVAL  50
#define _200MS_INTERVAL 200
#define _500MS_INTERVAL 500
#define _5S_INTERVAL    5000

enum {
    _50MS_INTERVAL_FLAG  = 0b00000001,
    _200MS_INTERVAL_FLAG = 0b00000010,
    _500MS_INTERVAL_FLAG = 0b00000100,
    _5S_INTERVAL_FLAG    = 0b00001000
};

typedef uint8_t measures_flags_t;

void measures_init();
void measures_voltage_current_soc();
void measures_check_flags();
void _measures_handle_tim_oc_irq(TIM_HandleTypeDef *htim);