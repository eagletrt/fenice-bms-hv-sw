/**
 * @file		can.h
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <../../lib/can/flatbuf-generator/Primary/flatcc/schema_builder.h>
#include <../../lib/can/includes-generator/Primary/ids.h>
#include <fdcan.h>

HAL_StatusTypeDef can_send(uint16_t id);