/**
 * @file		fsm_bms.c
 * @brief		BMS's state machine
 *
 * @date		Mar 25, 2020
 * 
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include "fsm_bms.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli_bms.h"
#include "error.h"
#include "main.h"
#include "pack.h"
#include "peripherals/can.h"
#include "tim.h"

//------------------------------Declarations------------------------------------------
uint16_t do_init(fsm *FSM);
uint16_t do_ts_off(fsm *FSM);
uint16_t do_idle(fsm *FSM);
uint16_t do_precharge_start(fsm *FSM);
uint16_t do_precharge(fsm *FSM);
uint16_t do_precharge_end(fsm *FSM);
uint16_t do_run(fsm *FSM);
uint16_t do_charge(fsm *FSM);
uint16_t do_to_halt(fsm *FSM);
uint16_t do_halt(fsm *FSM);

fsm fsm_bms;

uint32_t timer_precharge = 0;

void fsm_bms_init() {
	fsm_init(&fsm_bms, BMS_NUM_STATES);

	fsm_bms.state_table[BMS_INIT] = &do_init;
	fsm_bms.state_table[BMS_SET_TS_OFF] = &do_ts_off;
	fsm_bms.state_table[BMS_IDLE] = &do_idle;
	fsm_bms.state_table[BMS_PRECHARGE_START] = &do_precharge_start;
	fsm_bms.state_table[BMS_PRECHARGE] = &do_precharge;
	fsm_bms.state_table[BMS_PRECHARGE_END] = &do_precharge_end;
	fsm_bms.state_table[BMS_RUN] = &do_run;
	fsm_bms.state_table[BMS_CHARGE] = &do_charge;
	fsm_bms.state_table[BMS_TO_HALT] = &do_to_halt;
	fsm_bms.state_table[BMS_HALT] = &do_halt;

	// state_function *bms_states_tab[BMS_NUM_STATES][BMS_NUM_STATES] = {
	// 	{do_init, to_idle, to_precharge, NULL, NULL, to_halt},	   // from init
	// 	{NULL, do_idle, to_precharge, NULL, NULL, to_halt},		   // from idle
	// 	{NULL, to_idle, do_precharge, to_on, to_charge, to_halt},  // from precharge
	// 	{NULL, to_idle, NULL, do_on, NULL, to_halt},			   // from on
	// 	{NULL, to_idle, NULL, NULL, do_charge, to_halt},		   // from charge
	// 	{NULL, NULL, NULL, NULL, NULL, do_halt}};				   // from halt

	// char *bms_states_names[BMS_NUM_STATES] = {[BMS_INIT] = "init",
	// 										  [BMS_IDLE] = "idle",
	// 										  [BMS_PRECHARGE] = "pre-charge",
	// 										  [BMS_RUN] = "on",
	// 										  [BMS_CHARGE] = "charge",
	// 										  [BMS_HALT] = "halt"};

	// fsm_bms.state_table = (state_function ***)malloc(BMS_NUM_STATES * sizeof(state_function **));
	// for (uint8_t i = 0; i < BMS_NUM_STATES; i++) {
	// 	fsm_bms.state_table[i] = (state_function **)malloc(BMS_NUM_STATES * sizeof(state_function *));
	// 	for (uint8_t j = 0; j < BMS_NUM_STATES; j++) {
	// 		fsm_bms.state_table[i][j] = bms_states_tab[i][j];
	// 	}
	// }

	// fsm_bms.state_names = (char **)malloc(sizeof(char **) * BMS_NUM_STATES);
	// for (uint8_t i = 0; i < BMS_NUM_STATES; i++) {
	// 	fsm_bms.state_names[i] = (char *)malloc(sizeof(bms_states_names[i]));
	// 	strcpy(fsm_bms.state_names[i], bms_states_names[i]);
	// }
}

/**
 * @brief Does the staccah-staccah on the fsm
 * 
 * @details Called by the HAL timer interrupt, this function immediately executes do_to_halt and puts the fsm into HALT state
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim == &htim_Err) {  //htim_Err is a #define to htim2 (barely a substition) so tim.h must be included
		HAL_TIM_Base_Stop_IT(htim);

		fsm_handle_event(&fsm_bms, BMS_TO_HALT);
		fsm_run(&fsm_bms);
	}
}

/*
	Event Handlers
*/
void fsm_bms_ts_off_handler() {
	cli_bms_debug("FSM: got TS OFF event", 22);
	switch (fsm_get_state(&fsm_bms)) {
		case BMS_PRECHARGE_START:
		case BMS_PRECHARGE:
		case BMS_PRECHARGE_END:
		case BMS_RUN:
		case BMS_CHARGE:
			fsm_handle_event(&fsm_bms, BMS_SET_TS_OFF);
			break;
		default:
			can_send(ID_TS_STATUS);
			break;
	}
}

void fsm_bms_ts_on_handler() {
	cli_bms_debug("CAN: got TS ON event", 22);
	switch (fsm_get_state(&fsm_bms)) {
		case BMS_IDLE:
			fsm_handle_event(&fsm_bms, BMS_PRECHARGE_START);
			break;
		default:
			can_send(ID_TS_STATUS);
			break;
	}
}

uint16_t do_init(fsm *FSM) {
	pack_init();
	return BMS_IDLE;
}

uint16_t do_ts_off(fsm *FSM) {
	pack_set_ts_off();

	can_send(ID_TS_STATUS);
	return BMS_IDLE;
}

uint16_t do_idle(fsm *FSM) {
	// Zzz

	return BMS_IDLE;
}

uint16_t do_precharge_start(fsm *FSM) {
	// Precharge
	pack_set_pc_start();
	timer_precharge = HAL_GetTick();

	return BMS_PRECHARGE;
}

uint16_t do_precharge(fsm *FSM) {
	bms_states return_state = BMS_PRECHARGE;

	if (HAL_GetTick() - timer_precharge < PRECHARGE_TIMEOUT) {
		if (pack_get_bus_voltage() >= pack_get_int_voltage() * PRECHARGE_VOLTAGE_THRESHOLD) {
			return BMS_PRECHARGE_END;
		}
	} else {
		// If the precharge takes too long, we shut down and start from idle
		return_state = BMS_SET_TS_OFF;
	}

	return return_state;
}

uint16_t do_precharge_end(fsm *FSM) {
	pack_set_precharge_end();

	if (HAL_GPIO_ReadPin(CHARGE_GPIO_Port, CHARGE_Pin)) {
		return BMS_CHARGE;
	}
	return BMS_RUN;
}

uint16_t do_run(fsm *FSM) {
	return BMS_RUN;
}

uint16_t do_charge(fsm *FSM) {
	return BMS_CHARGE;
}

uint16_t do_to_halt(fsm *FSM) {
	// bms_set_ts_off(&data->bms);
	// bms_set_fault(&data->bms);
	pack_set_ts_off();

	// can_send_error(&hcan, data->error, data->error_index, &data->pack);
	return BMS_HALT;
}

uint16_t do_halt(fsm *FSM) { return BMS_HALT; }
