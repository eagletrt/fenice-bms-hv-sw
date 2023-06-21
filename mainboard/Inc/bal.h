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

enum { BAL_OFF = 0, BAL_COMPUTE, BAL_DISCHARGE, BAL_COOLDOWN, BAL_NUM_STATES };
enum { EV_BAL_STOP = 0, EV_BAL_START, EV_BAL_COOLDOWN_START, EV_BAL_COOLDOWN_END, BAL_EV_NUM };


// TODO: Start and stop balancing per cellboard

/*
void bal_start();
void bal_stop();


voltage_t bal_get_threshold();
void bal_set_threshold(uint16_t thresh);

uint8_t bal_are_cells_off_status();

void _bal_handle_tim_oc_irq(TIM_HandleTypeDef *htim);
*/

#endif