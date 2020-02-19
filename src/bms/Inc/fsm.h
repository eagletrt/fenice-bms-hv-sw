/**
 * @file		fsm.h
 * @brief		This file contains the primitives to setup a FSM
 *
 * @date		Oct 24, 2019
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef FSM_H
#define FSM_H

#include "bal.h"
#include "error.h"
#include "pack.h"
#include "stm32f4xx_hal.h"

typedef struct {
	SPI_HandleTypeDef *hspi;

	PACK_T pack;

	ERROR_STATUS_T can_error;

	error_t error;
	uint8_t error_index;

	bal_conf_t balancing;

} state_global_data_t;

typedef enum {
	BMS_INIT,
	BMS_IDLE,
	BMS_PRECHARGE,
	BMS_ON,
	BMS_CHARGE,
	BMS_HALT,
	BMS_NUM_STATES
} BMS_STATE_T;

extern const char *bms_state_names[BMS_NUM_STATES];

typedef BMS_STATE_T state_func_t(state_global_data_t *data);
typedef void transition_func_t(state_global_data_t *data);

#endif