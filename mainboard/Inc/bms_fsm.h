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

#include "blink.h"
#include "fsm.h"

#include <stdbool.h>
typedef enum { BMS_IDLE = 0, BMS_TS_ON, BMS_AIRN_CLOSE, BMS_AIRN_STATUS, BMS_PRECHARGE, BMS_ON, BMS_FAULT, BMS_NUM_STATES } bms_states;
typedef enum {
    BMS_EV_FAULT = 0,
    BMS_EV_TS_OFF,
    BMS_EV_TS_ON,
    BMS_EV_FB_CHECK,
    BMS_EV_FB_TIMEOUT,
    BMS_EV_PRECHARGE_CHECK,
    BMS_EV_PRECHARGE_TIMEOUT,
    BMS_EV_NO_ERRORS,
    BMS_EV_NUM
} bms_events;

typedef struct {
    fsm fsm;
    blink_t led;
    bool handcart_connected;
} bms_fsm;

extern bms_fsm bms;

void bms_fsm_init();

void _bms_handle_tim_oc_irq(TIM_HandleTypeDef *htim);
#endif