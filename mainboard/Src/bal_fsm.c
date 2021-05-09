/**
 * @file		bal_fsm.c
 * @brief		This file contains the balancing functions
 *
 * @date		May 09, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "bal_fsm.h"

#include "cli_bms.h"
#include "fenice_config.h"
#include "ltc6813_utils.h"
#include "pack.h"
#include "spi.h"

fsm bal_fsm;

uint32_t discharge_start = 0;
uint16_t indexes[PACK_CELL_COUNT];

uint16_t do_off(fsm* FSM);
uint16_t do_compute(fsm* FSM);
uint16_t do_discharge(fsm* FSM);

void bal_fsm_init() {
	bal.slot_time = BAL_CYCLE_LENGTH;
	bal.threshold = BAL_MAX_VOLTAGE_THRESHOLD;

	fsm_init(&bal_fsm, BAL_NUM_STATES);

	bal_fsm.state_table[BAL_OFF][BAL_OFF] = &do_off;
	bal_fsm.state_table[BAL_OFF][BAL_COMPUTING] = &do_compute;
	bal_fsm.state_table[BAL_COMPUTING][BAL_DISCHARGING] = &do_discharge;
	bal_fsm.state_table[BAL_COMPUTING][BAL_OFF] = &do_off;
	bal_fsm.state_table[BAL_DISCHARGING][BAL_DISCHARGING] = &do_discharge;
	bal_fsm.state_table[BAL_DISCHARGING][BAL_OFF] = &do_off;
	bal_fsm.state_table[BAL_DISCHARGING][BAL_COMPUTING] = &do_compute;
}

uint16_t do_off(fsm* FSM) {
	return BAL_OFF;
}

uint16_t do_compute(fsm* FSM) {
	size_t len = bal_compute_indexes(pack_get_voltages(), bal.threshold, indexes);

	if (len > 0) {
		ltc6813_set_balancing(&LTC6813_PERIPHERAL, indexes, bal.slot_time);
		discharge_start = HAL_GetTick();
		cli_bms_debug("Discharging cells", 18);
		return BAL_DISCHARGING;
	}
	cli_bms_debug("Nothing to balance", 19);
	return BAL_OFF;
}

uint16_t do_discharge(fsm* FSM) {
	if (discharge_start - HAL_GetTick() >= bal.slot_time) {
		return BAL_COMPUTING;
	}
	return BAL_DISCHARGING;
}
