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

void imd_init();
uint32_t imd_get_freq();
uint32_t imd_get_period();
uint8_t imd_get_duty_cycle_percentage();