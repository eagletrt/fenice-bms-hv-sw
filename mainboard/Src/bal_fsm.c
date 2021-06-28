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
#include "config.h"
#include "fenice_config.h"
#include "ltc6813_utils.h"
#include "pack.h"
#include "spi.h"

#define CONF_VER  0x01
#define CONF_ADDR 0x01

typedef struct {
    uint8_t version;
    voltage_t threshold;
} bal_params;
bal_params bal_params_default = {CONF_VER, BAL_MAX_VOLTAGE_THRESHOLD};

bal_fsm bal;
config_t config;

void off_entry(fsm FSM);
event_result off_handler(fsm FSM, uint8_t event);
void compute_entry(fsm FSM);
void discharge_entry(fsm FSM);
event_result discharge_handler(fsm FSM, uint8_t event);
event_result cooldown_handler(fsm FSM, uint8_t event);

voltage_t bal_get_threshold() {
    return ((bal_params *)config_get(config))->threshold;
}

void bal_set_threshold(uint16_t thresh) {
    bal_params params = *(bal_params *)config_get(config);
    params.threshold  = thresh;

    config_set(config, &params);
    config_write(config);
}

void bal_fsm_init() {
    bal.cycle_length = BAL_CYCLE_LENGTH;
    bal.fsm          = fsm_init(BAL_NUM_STATES, BAL_EV_NUM);

    fsm_state state;
    state.handler = off_handler;
    state.entry   = off_entry;
    state.exit    = NULL;
    fsm_set_state(bal.fsm, BAL_OFF, &state);

    state.handler = NULL;
    state.entry   = compute_entry;
    state.exit    = NULL;
    fsm_set_state(bal.fsm, BAL_COMPUTE, &state);

    state.handler = discharge_handler;
    state.entry   = discharge_entry;
    state.exit    = NULL;
    fsm_set_state(bal.fsm, BAL_DISCHARGE, &state);

    state.handler = cooldown_handler;
    state.entry   = NULL;
    state.exit    = NULL;
    fsm_set_state(bal.fsm, BAL_COOLDOWN, &state);

    config_init(&config, CONF_ADDR, &bal_params_default, sizeof(bal_params));
}

void off_entry(fsm FSM) {
    uint16_t cells[PACK_CELL_COUNT] = {0};
    ltc6813_set_balancing(&LTC6813_PERIPHERAL, cells, 0);
}

event_result off_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_START:
            fsm_transition(FSM, BAL_COMPUTE);
            return EVENT_HANDLED;
            break;
    }
    return EVENT_UNKNOWN;
}

void compute_entry(fsm FSM) {
    if (bal_get_cells_to_discharge(pack_get_voltages(), PACK_CELL_COUNT, bal_get_threshold(), bal.cells) != 0) {
        fsm_transition(FSM, BAL_DISCHARGE);
        return;
    }
    cli_bms_debug("Non si può fare meglio di così.", 34);
    fsm_transition(FSM, BAL_OFF);
}

void discharge_entry(fsm FSM) {
    ltc6813_set_balancing(&LTC6813_PERIPHERAL, bal.cells, bal.cycle_length);
    bal.discharge_time = HAL_GetTick();
    cli_bms_debug("Discharging cells", 18);
}

event_result discharge_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_STOP:
            fsm_transition(FSM, BAL_OFF);
            return EVENT_HANDLED;
            break;
        case EV_BAL_CHECK_TIMER:
            if (bal.discharge_time - HAL_GetTick() >= bal.cycle_length) {
                fsm_transition(FSM, BAL_COOLDOWN);
            } else {
                fsm_catch_event(FSM, EV_BAL_CHECK_TIMER);
            }
            return EVENT_HANDLED;
            break;
    }
    return EVENT_UNKNOWN;
}

event_result cooldown_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_STOP:
            fsm_transition(FSM, BAL_OFF);
            return EVENT_HANDLED;
        case EV_BAL_CHECK_TIMER:
            if (bal.discharge_time - HAL_GetTick() >= bal.cycle_length + BAL_COOLDOWN_DELAY) {
                fsm_transition(FSM, BAL_COMPUTE);
            } else {
                fsm_catch_event(FSM, EV_BAL_CHECK_TIMER);
            }
            return EVENT_HANDLED;
            break;
    }
    return EVENT_UNKNOWN;
}
