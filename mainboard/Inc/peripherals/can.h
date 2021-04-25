/**
 * @file		can.h
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <fdcan.h>

#include "../lib/can/includes_generator/Primary/ids.h"
#include "../lib/can/naked_generator/Primary/c/Primary.h"

void can_init();
HAL_StatusTypeDef can_send(uint16_t id);