/**
 * @file		bal_fsm.h
 * @brief		This file contains the balancing functions
 *
 * @date		May 09, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef BAL_FSM_H
#define BAL_FSM_H

#include "bal.h"
#include "fsm.h"

enum {
	BAL_OFF,
	BAL_COMPUTING,
	BAL_DISCHARGING,
	BAL_NUM_STATES
};

extern fsm bal_fsm;

void bal_fsm_init();

#endif
