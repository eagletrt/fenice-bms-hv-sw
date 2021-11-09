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
#include "pack/voltage.h"



#define CAN_WAIT(C)                                                                                     \
    {                                                                                                   \
        while (HAL_CAN_GetTxMailboxesFreeLevel(C) == 0);                                                \
    } 

void can_init();
void can_car_init();
void can_bms_init();
void can_tx_header_init();
HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header);
HAL_StatusTypeDef can_car_send(uint16_t id);
HAL_StatusTypeDef can_bms_send(uint16_t id);