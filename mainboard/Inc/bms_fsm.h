/**
 * @file		bms_fsm.h
 * @brief		BMS's state machine
 *
 * @date		Mar 31, 2020
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef BMS_FSM_H
#define BMS_FSM_H

#include "fsm.h"

#include <stdbool.h>
typedef enum { BMS_IDLE, BMS_PRECHARGE, BMS_ON, BMS_HALT, BMS_NUM_STATES } bms_states;
typedef enum {
    BMS_EV_HALT,
    BMS_EV_TS_OFF,
    BMS_EV_TS_ON,
    BMS_EV_VOLT_MEASURE,
    BMS_EV_TEMP_MEASURE,
    BMS_EV_PRECHARGE_CHECK,
    BMS_EV_PRECHARGE_TIMEOUT,
    BMS_EV_NO_ERRORS,
    BMS_EV_NUM
} bms_events;

typedef struct {
    fsm fsm;
} bms_fsm;

extern bms_fsm bms;

void bms_fsm_init();
#endif