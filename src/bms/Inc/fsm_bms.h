/**
 * @file		fsm_bms.h
 * @brief		BMS's state machine
 *
 * @date		Mar 31, 2020
 * 
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef FSM_BMS_H
#define FSM_BMS_H

#include "common/fsm.h"

typedef enum {
	BMS_INIT,
	BMS_IDLE,
	BMS_PRECHARGE,
	BMS_ON,
	BMS_CHARGE,
	BMS_HALT,
	BMS_NUM_STATES
} bms_state_t;

extern fsm_t fsm_bms;

extern state_func_t *state_table[BMS_NUM_STATES][BMS_NUM_STATES];
extern char *bms_state_names[BMS_NUM_STATES];

void fsm_bms_init();

#endif