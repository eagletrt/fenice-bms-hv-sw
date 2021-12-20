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

void pack_init();

/**
 * @brief	Updates the pack's temperature stats
 * @details It updates *_temperature variables with the data of the pack
 */
void pack_update_temperature_stats();

void pack_set_ts_on();
void pack_set_airn_off(uint8_t value);
void pack_set_pc_on();
bool pack_set_ts_off();
void pack_set_pc_start();
void pack_set_precharge_end();
