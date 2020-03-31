/**
 * @file		fsm.h
 * @brief		This file contains the primitives to setup a FSM
 *
 * @date		Oct 24, 2019
 *
 * @author	Simone Ruffini[simone.ruffini@tutanota.com]
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "common/fsm.h"

uint8_t fsm_transition(fsm_t *fsm, uint8_t future_state) {
	state_func_t *transition = fsm->state_table[fsm->current_state][future_state];

	if (transition) {
		return transition();
	}

	// No state function exists -> go ahead
	// TODO: add warning maybe?
	return fsm->current_state;
}

void fsm_run_state(fsm_t *fsm) {
	fsm->current_state = fsm_transition(fsm, fsm->current_state);
}

//void fsm_init(fsm_t *fsm) {
//}