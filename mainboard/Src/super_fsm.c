/**
 * @file		super_fsm.c
 * @brief		Super FSM: the simple FSM that handles sensor readings and BMS execution
 *
 * @date		Jun 27, 2021
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "super_fsm.h"

#include "bms_fsm.h"
#include "can_comm.h"
#include "current.h"
#include "energy.h"
#include "main.h"
#include "pack.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void super_bms_entry(fsm handle);
void super_bms(fsm handle, super_events event);
void super_measure_volts(fsm handle, super_events event);

void super_fsm_init() {
    super_fsm = fsm_init(SUPER_NUM_STATES, SUPER_EV_NUM, NULL, NULL);

    fsm_state state;
    state.run     = NULL;
    state.handler = super_bms;
    state.entry   = super_bms_entry;
    state.exit    = NULL;
    fsm_set_state(super_fsm, SUPER_BMS, &state);

    state.handler = super_measure_volts;
    state.entry   = NULL;
    state.exit    = NULL;
    fsm_set_state(super_fsm, SUPER_MEASURE_VOLTS, &state);

    HAL_TIM_OC_Start_IT(&htim_super, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&htim_super, TIM_CHANNEL_2);
    fsm_start(super_fsm);
}

void super_bms_entry(fsm handle) {
    fsm_trigger_event(handle, SUPER_EV_BMS);
}

void super_bms(fsm handle, super_events event) {
    switch (event) {
        case SUPER_EV_MEASURE_VOLTS:
            fsm_transition(handle, SUPER_MEASURE_VOLTS);
            break;
        case SUPER_EV_BMS:
            fsm_run(bms.fsm);
            fsm_trigger_event(handle, SUPER_EV_BMS);
            break;
        default:
            break;
    }
}

void super_measure_volts(fsm handle, super_events event) {
    pack_update_voltages(&SI8900_UART);

    energy_sample_current(current_read());

    can_send(ID_HV_CURRENT);

    fsm_transition(handle, SUPER_BMS);
}
