/**
 * @file		can.h
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <fdcan.h>

#include "../../lib/can/flatbuf-generator/Primary/c/schema_builder.h"
#include "../../lib/can/includes-generator/Primary/ids.h"

void can_init();
HAL_StatusTypeDef can_send(uint16_t id);