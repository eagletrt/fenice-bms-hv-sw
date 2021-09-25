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
#include "mainboard_config.h"
#include "pack.h"
#include "peripherals/can_comm.h"
#include "tim.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

//------------------------------Declarations------------------------------------------
void bms_set_led_blinker();
void bms_blink_led();

void _idle_entry(fsm FSM);
void _idle_handler(fsm FSM, uint8_t event);
void _idle_exit(fsm FSM);
void _precharge_entry(fsm FSM);
void _precharge_handler(fsm FSM, uint8_t event);
void _precharge_exit(fsm FSM);
void _on_entry(fsm FSM);
void _on_handler(fsm FSM, uint8_t event);
void _on_exit(fsm FSM);
void _halt_entry(fsm FSM);
void _halt_run(fsm FSM);
void _halt_handler(fsm FSM, uint8_t event);

bms_fsm bms;

void bms_fsm_init() {
    bms.fsm = fsm_init(BMS_NUM_STATES, BMS_EV_NUM, &bms_blink_led, NULL);

    fsm_state state;
    state.run     = NULL;
    state.handler = _idle_handler;
    state.entry   = _idle_entry;
    state.exit    = _idle_exit;
    fsm_set_state(bms.fsm, BMS_IDLE, &state);

    state.handler = _precharge_handler;
    state.entry   = _precharge_entry;
    state.exit    = _precharge_exit;
    fsm_set_state(bms.fsm, BMS_PRECHARGE, &state);

    state.handler = _on_handler;
    state.entry   = _on_entry;
    state.exit    = _on_exit;
    fsm_set_state(bms.fsm, BMS_ON, &state);

    state.handler = _halt_handler;
    state.entry   = _halt_entry;
    state.run     = _halt_run;
    state.exit    = NULL;
    fsm_set_state(bms.fsm, BMS_HALT, &state);

    HAL_TIM_Base_Start_IT(&htim_bms);
    fsm_start(bms.fsm);

    bms.led.port = STATE_LED_GPIO;
    bms.led.pin  = STATE_LED_PIN;
    bms.led.time = HAL_GetTick();
}

void bms_set_led_blinker() {
    uint8_t pattern_count                    = 0;
    uint8_t state_count                      = 0;
    uint32_t state                           = fsm_get_state(bms.fsm);
    uint16_t pattern[(BMS_NUM_STATES)*2 + 1] = {0};

    pattern[pattern_count++] = 200;  // Big off
    while (state_count < state) {
        pattern[pattern_count++] = 200;  // On
        pattern[pattern_count++] = 200;  // Off
        state_count++;
    }
    pattern[pattern_count++] = 1000;  // Big off

    BLINK_SET_PATTERN(bms.led, pattern, pattern_count);
    BLINK_SET_ENABLE(bms.led, true);
    BLINK_SET_REPEAT(bms.led, false);

    blink_reset(&(bms.led));
    HAL_GPIO_WritePin(bms.led.port, bms.led.pin, GPIO_PIN_SET);
}
void bms_blink_led() {
    if (BLINK_GET_ENABLE(bms.led)) {
        blink_run(&bms.led);
    } else {
        bms_set_led_blinker();
    }
}

void _idle_entry(fsm FSM) {
    can_send(ID_TS_STATUS);
}

void _idle_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_ON:
            fsm_transition(FSM, BMS_PRECHARGE);
            break;
        case BMS_EV_HALT:
            fsm_transition(FSM, BMS_HALT);
            break;
    }
}

void _idle_exit(fsm FSM) {
}

void _precharge_entry(fsm FSM) {
    // Precharge
    pack_set_pc_start();

    uint32_t cnt = __HAL_TIM_GET_COUNTER(&htim_bms);
    __HAL_TIM_SET_COMPARE(&htim_bms, TIM_CHANNEL_1, (cnt + PRECHARGE_CHECK_INTERVAL * 10));
    __HAL_TIM_SET_COMPARE(&htim_bms, TIM_CHANNEL_2, (cnt + PRECHARGE_TIMEOUT * 10));

    HAL_TIM_OC_Start_IT(&htim_bms, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&htim_bms, TIM_CHANNEL_2);

    cli_bms_debug("Entered precharge", 18);
}

void _precharge_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_PRECHARGE_TIMEOUT:
            cli_bms_debug("Precharge timeout", 18);
            // send timeout warning
        case BMS_EV_TS_OFF:
            fsm_transition(FSM, BMS_IDLE);
            break;

        case BMS_EV_PRECHARGE_CHECK:
            char c[5] = {'\0'};
            itoa(pack_get_bus_voltage() / pack_get_int_voltage() * PRECHARGE_VOLTAGE_THRESHOLD, c, 10);
            cli_bms_debug(c, 5);

            if (pack_get_bus_voltage() >= pack_get_int_voltage() * PRECHARGE_VOLTAGE_THRESHOLD) {
                fsm_transition(bms.fsm, BMS_ON);
                cli_bms_debug("Precharge ok", 18);
            }
            break;
        case BMS_EV_HALT:
            fsm_transition(FSM, BMS_HALT);
            break;
    }
}

void _precharge_exit(fsm FSM) {
    HAL_TIM_OC_Stop_IT(&htim_bms, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&htim_bms, TIM_CHANNEL_2);
}

void _on_entry(fsm FSM) {
    pack_set_precharge_end();
}

void _on_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_OFF:
            fsm_transition(FSM, BMS_IDLE);
            break;
        case BMS_EV_HALT:
            fsm_transition(FSM, BMS_HALT);
            break;
    }
}

void _on_exit(fsm FSM) {
    pack_set_ts_off();
}

void _halt_entry(fsm FSM) {
    // bms_set_fault(&data->bms);
    HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIO4_GPIO_Port, GPIO4_Pin, GPIO_PIN_SET);
    //can_send_error(&hcan, data->error, data->error_index, &data->pack);
    //cli_bms_debug("HALT", 5);
}

void _halt_run(fsm FSM) {
    if (error_get_fatal() == 0) {
        fsm_trigger_event(FSM, BMS_EV_NO_ERRORS);
    }
}

void _halt_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_NO_ERRORS:
            fsm_transition(FSM, BMS_IDLE);
            break;
    }
}
