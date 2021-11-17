/**
 * @file		mainboard_config.h
 * @brief		This file contains configuration settings for the mainboard
 *
 * @date		Nov 13, 2021
 * @author		Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#pragma once

#include "main.h"
#include "tim.h"
#include "usart.h"
#include "pack/current.h"
#include "pack/voltage.h"
#include "energy/soc.h"
#include "mainboard_config.h"

#define VOLTS_READ_INTERVAL 20
#define TEMPS_READ_INTERVAL 200

void measures_init();
void measures_current();