/**
 * @file		bms_fsm.c
 * @brief		BMS's state machine
 *
 * @date		Mar 25, 2020
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include "bms_fsm.h"

#include "cli_bms.h"
#include "error.h"
#include "main.h"
#include "pack.h"
#include "peripherals/can.h"
#include "tim.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

//------------------------------Declarations------------------------------------------
void idle_entry(fsm FSM);
event_result idle_handler(fsm FSM, uint8_t event);
void idle_exit(fsm FSM);
void precharge_entry(fsm FSM);
event_result precharge_handler(fsm FSM, uint8_t event);
void precharge_exit(fsm FSM);
void on_entry(fsm FSM);
event_result on_handler(fsm FSM, uint8_t event);
void bms_on_exit(fsm FSM);
void halt_entry(fsm FSM);
event_result halt_handler(fsm FSM, uint8_t event);

bms_fsm bms;

void bms_fsm_init() {
    bms.fsm = fsm_init(BMS_NUM_STATES, BMS_EV_NUM);

    fsm_state state;
    state.handler = idle_handler;
    state.entry   = idle_entry;
    state.exit    = idle_exit;
    fsm_set_state(bms.fsm, BMS_IDLE, &state);

    state.handler = precharge_handler;
    state.entry   = precharge_entry;
    state.exit    = precharge_exit;
    fsm_set_state(bms.fsm, BMS_PRECHARGE, &state);

    state.handler = on_handler;
    state.entry   = on_entry;
    state.exit    = bms_on_exit;
    fsm_set_state(bms.fsm, BMS_ON, &state);

    state.handler = halt_handler;
    state.entry   = halt_entry;
    state.exit    = NULL;
    fsm_set_state(bms.fsm, BMS_HALT, &state);

    HAL_TIM_Base_Start_IT(&htim_bms);
    fsm_start(bms.fsm);
}

void idle_entry(fsm FSM) {
    pack_set_ts_off();

    can_send(ID_TS_STATUS);
}

event_result idle_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_ON:
            fsm_transition(FSM, BMS_PRECHARGE);
            return EVENT_HANDLED;
            break;
        case BMS_EV_HALT:
            fsm_transition(FSM, BMS_HALT);
            return EVENT_HANDLED;
            break;
    }
    return EVENT_UNKNOWN;
}

void idle_exit(fsm FSM) {
}

void precharge_entry(fsm FSM) {
    // Precharge
    pack_set_pc_start();

    uint32_t cnt = __HAL_TIM_GET_COUNTER(&htim_bms);
    __HAL_TIM_SET_COMPARE(&htim_bms, TIM_CHANNEL_1, (cnt + PRECHARGE_CHECK_INTERVAL * 10));
    __HAL_TIM_SET_COMPARE(&htim_bms, TIM_CHANNEL_2, (cnt + PRECHARGE_TIMEOUT * 10));

    HAL_TIM_OC_Start_IT(&htim_bms, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&htim_bms, TIM_CHANNEL_2);

    cli_bms_debug("Precharge entryed", 18);
}

event_result precharge_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_PRECHARGE_TIMEOUT:
            cli_bms_debug("Precharge timeout", 18);
            // send timeout warning
        case BMS_EV_TS_OFF:
            fsm_transition(FSM, BMS_IDLE);
            return EVENT_HANDLED;
            break;

        case BMS_EV_PRECHARGE_CHECK:
            char c[5] = {'\0'};
            itoa(pack_get_bus_voltage() / pack_get_int_voltage() * PRECHARGE_VOLTAGE_THRESHOLD, c, 10);
            cli_bms_debug(c, 5);

            if (pack_get_bus_voltage() >= pack_get_int_voltage() * PRECHARGE_VOLTAGE_THRESHOLD) {
                fsm_transition(bms.fsm, BMS_ON);
                cli_bms_debug("Precharge ok", 18);
            }
            return EVENT_HANDLED;
            break;
        case BMS_EV_HALT:
            fsm_transition(FSM, BMS_HALT);
            return EVENT_HANDLED;
            break;
    }
    return EVENT_UNKNOWN;
}

void precharge_exit(fsm FSM) {
    HAL_TIM_OC_Stop_IT(&htim_bms, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&htim_bms, TIM_CHANNEL_2);
}

void on_entry(fsm FSM) {
    pack_set_precharge_end();
    if (HAL_GPIO_ReadPin(CHARGE_GPIO_Port, CHARGE_Pin)) {
        //entry charge fsm
    }
}

event_result on_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_OFF:
            fsm_transition(FSM, BMS_IDLE);
            return EVENT_HANDLED;
            break;
        case BMS_EV_HALT:
            fsm_transition(FSM, BMS_HALT);
            return EVENT_HANDLED;
            break;
    }
    return EVENT_UNKNOWN;
}

void bms_on_exit(fsm FSM) {
}

void halt_entry(fsm FSM) {
    pack_set_ts_off();
    // bms_set_fault(&data->bms);

    // can_send_error(&hcan, data->error, data->error_index, &data->pack);
    cli_bms_debug("HALT", 5);
}

event_result halt_handler(fsm FSM, uint8_t event) {
    return EVENT_HANDLED;
}
