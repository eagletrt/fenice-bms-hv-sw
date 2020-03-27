/**
 * @file		fsm.h
 * @brief		This file contains the primitives to setup a FSM
 *
 * @date		Oct 24, 2019
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini[simone.ruffini@tutanota.com]
 */

#ifndef FSM_H
#define FSM_H

#include <inttypes.h>

typedef uint8_t state_func_t();

typedef struct fsm {
	uint8_t current_state;
	state_func_t ***state_table;
	char **state_names;
} fsm_t;

uint8_t fsm_transition(fsm_t *fsm, uint8_t future_state);
void fsm_run_state(fsm_t *fsm);
void fsm_init(fsm_t *fsm, state_func_t ***state_table, char **state_names, uint8_t initial_state);

#endif