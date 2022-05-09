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
#include "can_comm.h"
#include "fsm.h"
#include "tim.h"

enum { BAL_OFF, BAL_COMPUTE, BAL_DISCHARGE, BAL_COOLDOWN, BAL_NUM_STATES };
enum { EV_BAL_STOP, EV_BAL_START, EV_BAL_COOLDOWN_START, EV_BAL_COOLDOWN_END, BAL_EV_NUM };

typedef struct {
    fsm fsm;
    bms_balancing_cells cells[LTC6813_COUNT];
    bms_balancing_status status[LTC6813_COUNT];
    uint32_t discharge_time;
    uint32_t cycle_length;
    voltage_t target;
} bal_fsm;

extern bal_fsm bal;

uint16_t bal_get_threshold();
void bal_set_threshold(uint16_t thresh);

void bal_fsm_init();
uint8_t bal_are_cells_off_status();

void _bal_handle_tim_oc_irq(TIM_HandleTypeDef *htim);

#endif
