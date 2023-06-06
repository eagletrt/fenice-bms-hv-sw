/**
 * @file	can_comm.h
 * @brief	CAN bus serialization middleware
 *
 * @date	Mar 1, 2021
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include "bms/bms_network.h"
#include "primary/primary_network.h"
#include "can.h"
#include "pack/voltage.h"

#define CAN_1MBIT_PRE 3
#define CAN_1MBIT_BS1 CAN_BS1_12TQ
#define CAN_1MBIT_BS2 CAN_BS2_2TQ

#define CAN_125KBIT_PRE 20
#define CAN_125KBIT_BS1 CAN_BS1_15TQ
#define CAN_125KBIT_BS2 CAN_BS2_2TQ

typedef enum { CAN_BITRATE_1MBIT, CAN_BITRATE_125KBIT } CAN_Bitrate;

#define CAN_SLAVE_START_FILTER_BANK 14

void can_car_init();
void can_bms_init();
void can_tx_header_init();
HAL_StatusTypeDef can_send(CAN_HandleTypeDef *hcan, uint8_t *buffer, CAN_TxHeaderTypeDef *header);
HAL_StatusTypeDef can_car_send(uint16_t id);
HAL_StatusTypeDef can_bms_send(uint16_t id);
void can_cellboards_check();
void CAN_change_bitrate(CAN_HandleTypeDef *hcan, CAN_Bitrate bitrate);