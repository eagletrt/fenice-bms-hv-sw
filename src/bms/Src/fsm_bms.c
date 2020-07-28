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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "main.h"
#include "pack.h"

//------------------------------Declarations------------------------------------------
bms_states do_init(fsm *FSM);
bms_states do_ts_off(fsm *FSM);
bms_states do_idle(fsm *FSM);
bms_states do_precharge_start(fsm *FSM);
bms_states do_precharge(fsm *FSM);
bms_states do_precharge_end(fsm *FSM);
bms_states do_run(fsm *FSM);
bms_states do_charge(fsm *FSM);
bms_states do_to_halt(fsm *FSM);
bms_states do_halt(fsm *FSM);

fsm fsm_bms;

uint32_t timer_precharge = 0;

void fsm_bms_init() {
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

	fsm_bms.current_state = BMS_INIT;
}

bms_states do_init(fsm *FSM) {
	pack_init();
	return BMS_IDLE;
}

bms_states do_ts_off(fsm *FSM) {
	pack_set_ts_off();

	// can_send(&hcan, CAN_ID_BMS, CAN_MSG_TS_OFF, 8);
	return BMS_IDLE;
}

bms_states do_idle(fsm *FSM) {
	// Check CAN
	//if (data->can_rx.StdId == CAN_ID_ECU) {
	//	if (data->can_rx.Data[0] == CAN_IN_TS_ON) {
	// 	TS On message received
	//		if (pack_feedback_check(FEEDBACK_IDLE_TS_ON_TRIGGER_MASK,FEEDBACK_IDLE_TS_ON_TRIGGER_VALUE,ERROR_FEEDBACK_HARD)) {
	//			return BMS_PRECHARGE;
	//		}
	//	}
	//}

	// Do state-specific stuff

	// Return
	return BMS_IDLE;
}

bms_states do_precharge_start(fsm *FSM) {
	// Precharge
	// bms_precharge_start(&data->bms);
	pack_set_pc_start();
	timer_precharge = HAL_GetTick();

	return BMS_PRECHARGE;
}

bms_states do_precharge(fsm *FSM) {
	bms_states return_state = BMS_PRECHARGE;

	if (HAL_GetTick() - timer_precharge < PRECHARGE_TIMEOUT) {
		if (pd_get_bus_voltage() >= pd_get_adc_voltage() * PRECHARGE_VOLTAGE_THRESHOLD) {
			pack_set_precharge_end();
			return_state = BMS_RUN;

			if (HAL_GPIO_ReadPin(CHARGE_GPIO_Port, CHARGE_Pin)) {
				return_state = BMS_CHARGE;
			}
		}
	} else {
		return_state = BMS_HALT;
	}
	return (error_verify(HAL_GetTick()) == ERROR_OK) ? return_state : BMS_HALT;

	/*
	if (pd_get_bus_voltage() >= pd_get_adc_voltage() * PRECHARGE_VOLTAGE_THRESHOLD) {
		pack_set_precharge_end();

		if (HAL_GPIO_ReadPin(CHARGE_GPIO_Port, CHARGE_Pin)) {
			return_state = BMS_CHARGE;
		}
		return BMS_RUN;
	}

	if (HAL_GetTick() - timer_precharge >= PRECHARGE_TIMEOUT) {
		// TODO: set some sort of error
		return BMS_IDLE;

		return BMS_CHARGE;
	}
	*/
}

bms_states do_precharge_end(fsm *FSM) {
	// bms_precharge_end(&data->bms);
	// HAL_CAN_ConfigFilter(&hcan, &CAN_FILTER_NORMAL);
	// can_send(&hcan, CAN_ID_BMS, CAN_MSG_TS_ON, 8);

	return BMS_RUN;
}

bms_states do_run(fsm *FSM) {
	// if (data->can_rx.StdId == CAN_ID_ECU) {
	// 	if (data->can_rx.Data[0] == CAN_IN_TS_OFF) {
	// 		return BMS_IDLE;
	// 	}
	// }

	return BMS_RUN;
}

bms_states do_to_halt(fsm *FSM) {
	// bms_set_ts_off(&data->bms);
	// bms_set_fault(&data->bms);

	// can_send_error(&hcan, data->error, data->error_index, &data->pack);
	return BMS_HALT;
}

bms_states do_halt(fsm *FSM) { return BMS_HALT; }
