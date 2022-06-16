/**
 * @file    pack.h
 * @brief   This file contains the functions to manage the battery pack state
 *
 * @date    Apr 11, 2019
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include "error/error.h"
#include "main.h"

#include <inttypes.h>

#define TS_ON_VALUE  GPIO_PIN_SET
#define TS_OFF_VALUE GPIO_PIN_RESET

#define AIRN_OFF_VALUE GPIO_PIN_SET
#define AIRN_ON_VALUE  GPIO_PIN_RESET

#define AIRP_OFF_VALUE GPIO_PIN_SET
#define AIRP_ON_VALUE  GPIO_PIN_RESET

#define PRECHARGE_OFF_VALUE GPIO_PIN_SET
#define PRECHARGE_ON_VALUE  GPIO_PIN_RESET

#define BMS_FAULT_OFF_VALUE GPIO_PIN_SET
#define BMS_FAULT_ON_VALUE  GPIO_PIN_RESET

void pack_set_ts_on(uint8_t value);
void pack_set_airn_off(uint8_t value);
void pack_set_airp_off(uint8_t value);
void pack_set_precharge(uint8_t value);
void pack_set_fault(uint8_t value);
void pack_set_default_off();
