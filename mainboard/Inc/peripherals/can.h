/**
 * @file		can.h
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <fdcan.h>
#include <ids.h>
#include <schema_builder.h>

HAL_StatusTypeDef can_send(uint16_t id);