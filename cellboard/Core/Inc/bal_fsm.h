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
#include "cellboard_config.h"
#include "fsm.h"
#include "tim.h"

#include <inttypes.h>

enum { BAL_OFF = 0, BAL_COMPUTE, BAL_DISCHARGE, BAL_NUM_STATES };
enum { EV_BAL_STOP = 0, EV_BAL_START, EV_BAL_CHECK_TIMER, BAL_EV_NUM };

typedef struct {
    fsm fsm;

    uint16_t cells[PACK_CELL_COUNT];
    size_t cells_length;
    uint32_t discharge_time;
    uint32_t cycle_length;
} bal_fsm;

extern bal_fsm bal;

void bal_fsm_init();
void bal_timers_handler(TIM_HandleTypeDef* htim, fsm handle);

#endif