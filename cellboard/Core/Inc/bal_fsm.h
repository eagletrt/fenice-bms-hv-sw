/**
 * @file		bal_fsm.h
 * @brief		This file contains the balancing functions
 *
 * @date		Jul 07, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef BAL_FSM_H
#define BAL_FSM_H

//#include "bal.h"
#include "can_comms.h"
#include "cellboard_config.h"
#include "fsm.h"
#include "tim.h"
#include "bms/bms_network.h"

#include <inttypes.h>

enum { BAL_OFF = 0, BAL_DISCHARGE, BAL_NUM_STATES };
enum { EV_BAL_STOP = 0, EV_BAL_START, EV_BAL_CHECK_TIMER, BAL_EV_NUM };

typedef struct {
    fsm fsm;

    uint8_t is_s_pin_high;
    bms_balancing_converted_t cells;
    uint32_t discharge_time;
    uint32_t cycle_length;
} bal_fsm;

extern bal_fsm bal;

void bal_fsm_init();
void bal_timers_handler(TIM_HandleTypeDef *htim, fsm handle);
void bal_oc_timer_handler(TIM_HandleTypeDef *htim);
uint8_t bal_is_cells_empty();

#endif