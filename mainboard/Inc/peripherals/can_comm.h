/**
 * @file	can_comm.h
 * @brief	CAN bus serialization middleware
 *
 * @date	Mar 1, 2021
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include "../Primary/ids.h"
#include "../../Primary/c/Primary.h"
#include "../bms/ids.h"
#include "../../bms/c/bms.h"
#include "can.h"

void can_init();
void can_car_init();
void can_bms_init();
void can_tx_header_init();
HAL_StatusTypeDef can_send(uint16_t id);
HAL_StatusTypeDef can_bms_send(uint16_t id);