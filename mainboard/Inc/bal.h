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

// TODO: Start and stop balancing per cellboard

void bal_start();
void bal_stop();


voltage_t bal_get_threshold();
void bal_set_threshold(uint16_t thresh);

uint8_t bal_are_cells_off_status();

void _bal_handle_tim_oc_irq(TIM_HandleTypeDef *htim);

#endif
