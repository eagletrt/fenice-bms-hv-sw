/**
 * @file		fsm_bms.c
 * @brief		BMS's state machine
 *
 * @date		Mar 25, 2020
 * 
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include <stdio.h>

//#include "pack_data.h"
#include <stdlib.h>
#include <string.h>

#include "fsm_bms.h"

//------------------------------Declarations------------------------------------------
bms_state_t do_state_init();
bms_state_t do_state_idle();
bms_state_t do_state_charge();
bms_state_t do_state_precharge();
bms_state_t do_state_on();
bms_state_t do_state_halt();
bms_state_t to_idle();
bms_state_t to_precharge();
bms_state_t to_charge();
bms_state_t to_on();
bms_state_t to_halt();

fsm_t fsm_bms;

void fsm_bms_init() {
	state_func_t *bms_state_table[BMS_NUM_STATES][BMS_NUM_STATES] = {
		{do_state_init, to_idle, to_precharge, NULL, NULL, to_halt},	 // from init
		{NULL, do_state_idle, to_precharge, NULL, NULL, to_halt},		 // from idle
		{NULL, to_idle, do_state_precharge, to_on, to_charge, to_halt},	 // from precharge
		{NULL, to_idle, NULL, do_state_on, NULL, to_halt},				 // from on
		{NULL, to_idle, NULL, NULL, do_state_charge, to_halt},			 // from charge
		{NULL, NULL, NULL, NULL, NULL, do_state_halt}};					 // from halt

	char *bms_state_names[BMS_NUM_STATES] = {[BMS_INIT] = "init",
											 [BMS_IDLE] = "idle",
											 [BMS_PRECHARGE] = "pre-charge",
											 [BMS_ON] = "on",
											 [BMS_CHARGE] = "charge",
											 [BMS_HALT] = "halt"};

	fsm_bms.state_table = (state_func_t ***)malloc(BMS_NUM_STATES * sizeof(state_func_t **));
	for (uint8_t i = 0; i < BMS_NUM_STATES; i++) {
		fsm_bms.state_table[i] = (state_func_t **)malloc(BMS_NUM_STATES * sizeof(state_func_t *));
		for (uint8_t j = 0; j < BMS_NUM_STATES; j++) {
			fsm_bms.state_table[i][j] = bms_state_table[i][j];
		}
	}

	fsm_bms.state_names = (char **)malloc(sizeof(char **) * BMS_NUM_STATES);
	for (uint8_t i = 0; i < BMS_NUM_STATES; i++) {
		fsm_bms.state_names[i] = (char *)malloc(sizeof(bms_state_names[i]));
		strcpy(fsm_bms.state_names[i], bms_state_names[i]);
	}

	fsm_bms.current_state = BMS_INIT;
}

bms_state_t do_state_init() {
	//pack_init(&pack);
	return BMS_IDLE;
}

bms_state_t to_idle() {
	// bms_set_ts_off(&data->bms);
	// HAL_CAN_ConfigFilter(&hcan, &CAN_FILTER_NORMAL);
	// can_send(&hcan, CAN_ID_BMS, CAN_MSG_TS_OFF, 8);
	return BMS_IDLE;
}

bms_state_t do_state_idle() {
	// if (data->can_rx.StdId == CAN_ID_ECU) {
	// 	if (data->can_rx.Data[0] == CAN_IN_TS_ON) {
	// 		// TS On
	// 		if (data->can_rx.Data[1] == 0x01) {
	// 			// Charge command
	// 			data->bms.precharge_bypass = true;
	// 		}
	// 		return BMS_PRECHARGE;
	// 	}
	// }

	return BMS_IDLE;
}

bms_state_t to_precharge() {
	// Precharge
	// bms_precharge_start(&data->bms);
	// timer_precharge = HAL_GetTick();

	return BMS_PRECHARGE;
}

bms_state_t do_state_precharge() {	// Check for incoming voltage
	// if (data->can_rx.StdId == CAN_ID_IN_INVERTER_L) {
	// 	if (data->can_rx.Data[0] == CAN_IN_BUS_VOLTAGE) {
	// 		uint16_t bus_voltage = 0;

	// 		bus_voltage = data->can_rx.Data[2] << 8;
	// 		bus_voltage += data->can_rx.Data[1];
	// 		bus_voltage /= 31.99;

	// 		if (bus_voltage >= data->pack.total_voltage / 10000 * 0.95) {
	// 			bms_precharge_end(&data->bms);
	// 			return BMS_ON;
	// 		}
	// 	}
	// }

	// switch (bms_precharge_check(&(data)->bms)) {
	// 	case PRECHARGE_SUCCESS:
	// 		// Used when bypassing precharge

	// 		return BMS_CHARGE;
	// 		break;

	// 	case PRECHARGE_FAILURE:
	// 		// Precharge timed out

	// 		can_send_warning(&hcan, WARN_PRECHARGE_FAIL, 0);

	// 		return BMS_IDLE;
	// 		break;

	// 	case PRECHARGE_WAITING:
	// 		// If precharge is still running, send the bus voltage request

	// 		if (HAL_GetTick() - timer_precharge >= 20) {
	// 			timer_precharge = HAL_GetTick();

	// 			can_send(&hcan, CAN_ID_OUT_INVERTER_L, CAN_MSG_BUS_VOLTAGE, 8);
	// 		}
	// 		break;
	// }
	return BMS_PRECHARGE;
}

bms_state_t to_charge() {
	// send ready to charge message (TBD)

	return BMS_CHARGE;
}

bms_state_t do_state_charge() {
	// if (data->can_rx.StdId == CAN_ID_ECU) {
	// 	if (data->can_rx.Data[0] == CAN_IN_TS_OFF) {
	// 		return BMS_IDLE;
	// 	}
	// }

	return BMS_CHARGE;
}

bms_state_t to_on() {
	// bms_precharge_end(&data->bms);
	// HAL_CAN_ConfigFilter(&hcan, &CAN_FILTER_NORMAL);
	// can_send(&hcan, CAN_ID_BMS, CAN_MSG_TS_ON, 8);

	return BMS_ON;
}

bms_state_t do_state_on() {
	// if (data->can_rx.StdId == CAN_ID_ECU) {
	// 	if (data->can_rx.Data[0] == CAN_IN_TS_OFF) {
	// 		return BMS_IDLE;
	// 	}
	// }

	return BMS_ON;
}

bms_state_t to_halt() {
	// bms_set_ts_off(&data->bms);
	// bms_set_fault(&data->bms);

	// can_send_error(&hcan, data->error, data->error_index, &data->pack);
	return BMS_HALT;
}

bms_state_t do_state_halt() { return BMS_HALT; }
