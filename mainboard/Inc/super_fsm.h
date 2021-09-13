/**
 * @file	super_fsm.h
 * @brief	Super FSM: the simple FSM that handles sensor readings and BMS execution
 *
 * @date	Jun 27, 2021
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef SUPER_FSM_H
#define SUPER_FSM_H

#include "fsm.h"

#include <stdbool.h>

#define VOLTS_READ_INTERVAL 20
#define TEMPS_READ_INTERVAL 200

typedef enum { SUPER_BMS, SUPER_MEASURE_VOLTS, SUPER_NUM_STATES } super_states;
typedef enum { SUPER_EV_MEASURE_VOLTS, SUPER_EV_BMS, SUPER_EV_NUM } super_events;

extern fsm super_fsm;

void super_fsm_init();
#endif