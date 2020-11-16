/**
 * @file		fsm_bms.h
 * @brief		BMS's state machine
 *
 * @date		Mar 31, 2020
 * 
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef FSM_BMS_H
#define FSM_BMS_H

#include <fsm.h>

typedef enum {
	BMS_INIT,
	BMS_SET_TS_OFF,
	BMS_IDLE,
	BMS_PRECHARGE_START,
	BMS_PRECHARGE,
	BMS_PRECHARGE_END,
	BMS_RUN,
	BMS_CHARGE,
	BMS_TO_HALT,
	BMS_HALT,
	BMS_NUM_STATES
} bms_states;

extern fsm fsm_bms;

void fsm_bms_init();

#endif