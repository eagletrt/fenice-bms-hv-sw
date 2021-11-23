/**
 * @file		bal_fsm.c
 * @brief		This file contains the balancing functions
 *
 * @date		May 09, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "bal_fsm.h"

#include "can_comm.h"
#include "cli_bms.h"
#include "config.h"
#include "fenice_config.h"
#include "pack/pack.h"
#include "spi.h"

#include <string.h>

#define CONF_VER  0x01
#define CONF_ADDR 0x01

typedef struct {
    voltage_t threshold;
} bal_params;
bal_params bal_params_default = {BAL_MAX_VOLTAGE_THRESHOLD};

bal_fsm bal;
config_t config;

void off_entry(fsm FSM);
void off_handler(fsm FSM, uint8_t event);
void compute_entry(fsm FSM);
void discharge_entry(fsm FSM);
void discharge_exit(fsm FSM);
void discharge_handler(fsm FSM, uint8_t event);
void cooldown_entry(fsm FSM);
void cooldown_handler(fsm FSM, uint8_t event);
void cooldown_exit(fsm FSM);

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
    memset(bal.cells, 0, sizeof(bal.cells));
    memset(bal.status, bms_balancing_status_OFF, sizeof(bal.status));

    bal.cycle_length = TIM_MS_TO_TICKS(&HTIM_BAL, BAL_CYCLE_LENGTH);
    bal.fsm          = fsm_init(BAL_NUM_STATES, BAL_EV_NUM, NULL, NULL);

    fsm_state state;
    state.run     = NULL;
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
    state.exit    = discharge_exit;
    fsm_set_state(bal.fsm, BAL_DISCHARGE, &state);

    state.handler = cooldown_handler;
    state.entry   = cooldown_entry;
    state.exit    = cooldown_exit;
    fsm_set_state(bal.fsm, BAL_COOLDOWN, &state);

    __HAL_TIM_SetCounter(&HTIM_BAL, 0U);
    HAL_TIM_Base_Start_IT(&HTIM_BAL);

    config_init(&config, CONF_ADDR, CONF_VER, &bal_params_default, sizeof(bal_params));
}

void off_entry(fsm FSM) {
    HAL_TIM_OC_Stop_IT(&HTIM_BAL, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&HTIM_BAL, TIM_CHANNEL_2);
    cli_bms_debug("disabling balancing", 20);
	memset(bal.cells, 0, sizeof(bms_balancing_cells) * LTC6813_COUNT);
    can_bms_send(ID_BALANCING);
}

void off_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_START:
            fsm_transition(FSM, BAL_COMPUTE);
            break;
    }
}

void compute_entry(fsm FSM) {
    if ( bal_get_cells_to_discharge(voltage_get_cells(), PACK_CELL_COUNT, bal_get_threshold(), bal.cells, LTC6813_COUNT) != 0) {
        fsm_transition(FSM, BAL_DISCHARGE);
        return;
    }
    cli_bms_debug("Non si può fare meglio di così.", 34);
    can_bms_send(ID_BALANCING);
    fsm_transition(FSM, BAL_OFF);
}

void discharge_entry(fsm FSM) {
    /*
    fsm_trigger_event(FSM, EV_BAL_CHECK_TIMER);
    bal.discharge_time = HAL_GetTick();
    */

    HAL_TIM_Base_Stop_IT(&HTIM_BAL);
    __HAL_TIM_SET_COMPARE(&HTIM_BAL, TIM_CHANNEL_1, __HAL_TIM_GET_COUNTER(&HTIM_BAL) + bal.cycle_length);
    HAL_TIM_Base_Start_IT(&HTIM_BAL);
    HAL_TIM_OC_Start_IT(&HTIM_BAL, TIM_CHANNEL_1);

    can_bms_send(ID_BALANCING);
    cli_bms_debug("Discharging cells", 18);
}

void discharge_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_STOP:
            fsm_transition(FSM, BAL_OFF);
            break;
        case EV_BAL_COOLDOWN_START:
            if(bal_are_cells_off_status()){
                fsm_transition(FSM, BAL_COOLDOWN);
            } else {
                fsm_trigger_event(FSM, EV_BAL_COOLDOWN_START);
            }
            break;
    }
}

void discharge_exit(fsm FSM) {
    HAL_TIM_OC_Stop_IT(&HTIM_BAL, TIM_CHANNEL_1);
}

void cooldown_entry(fsm FSM) {

    HAL_TIM_Base_Stop_IT(&HTIM_BAL);
    __HAL_TIM_SET_COMPARE(&HTIM_BAL, TIM_CHANNEL_2, __HAL_TIM_GET_COUNTER(&HTIM_BAL) + TIM_MS_TO_TICKS(&HTIM_BAL, BAL_COOLDOWN_DELAY));
    __HAL_TIM_CLEAR_FLAG(&HTIM_BAL, TIM_IT_CC2); //clears existing interrupts on channel 1
    HAL_TIM_Base_Start_IT(&HTIM_BAL);
    HAL_TIM_OC_Start_IT(&HTIM_BAL, TIM_CHANNEL_2);

    cli_bms_debug("Cooldown cells", 15);
}

void cooldown_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_STOP:
            fsm_transition(FSM, BAL_OFF);
        case EV_BAL_COOLDOWN_END:
            fsm_transition(FSM, BAL_COMPUTE);
            break;
    }
}

void cooldown_exit(fsm FSM) {
    HAL_TIM_OC_Stop_IT(&HTIM_BAL, TIM_CHANNEL_2);
}

uint8_t bal_are_cells_off_status() {
    bms_balancing_status off[LTC6813_COUNT] = { bms_balancing_status_OFF };
    return memcmp(bal.status, off, sizeof(bal.status)) == 0;
}