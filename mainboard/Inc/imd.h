/**
 * @file    current.h
 * @brief   Functions that handle current measurement
 *
 * @date    Dec 01, 2021
 *
 * @author  Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#pragma once

#include "main.h"
#include "mainboard_config.h"
#include "tim.h"

typedef enum { IMD_SC, IMD_NORMAL, IMD_UNDER_VOLTAGE, IMD_START_MEASURE, IMD_DEVICE_ERROR, IMD_HEARTH_FAULT } IMD_STATE;

void imd_init();
float imd_get_freq();
uint32_t imd_get_period();
float imd_get_duty_cycle_percentage();
IMD_STATE imd_get_state();