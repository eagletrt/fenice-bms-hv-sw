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
#include "pack/pack.h"
#include "pack/voltage.h"
#include "peripherals/can_comm.h"
#include "tim.h"
#include "feedback.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

//------------------------------Declarations------------------------------------------
void bms_set_led_blinker();
void bms_blink_led();

void _idle_entry(fsm FSM);
void _idle_handler(fsm FSM, uint8_t event);
void _ts_on_entry(fsm FSM);
void _ts_on_handler(fsm FSM, uint8_t event);
void _ts_on_exit(fsm FSM);
void _airn_off_entry(fsm FSM);
void _airn_off_handler(fsm FSM, uint8_t event);
void _airn_off_exit(fsm FSM);
void _precharge_entry(fsm FSM);
void _precharge_handler(fsm FSM, uint8_t event);
void _precharge_exit(fsm FSM);
void _on_entry(fsm FSM);
void _on_handler(fsm FSM, uint8_t event);
void _on_exit(fsm FSM);
void _fault_entry(fsm FSM);
void _fault_run(fsm FSM);
void _fault_handler(fsm FSM, uint8_t event);

bms_fsm bms;

void bms_fsm_init() {
    bms.fsm = fsm_init(BMS_NUM_STATES, BMS_EV_NUM, &bms_blink_led, &bms_set_led_blinker);

    fsm_state state;
    state.run     = NULL;
    state.handler = _idle_handler;
    state.entry   = _idle_entry;
    state.exit    = NULL;
    fsm_set_state(bms.fsm, BMS_IDLE, &state);

    state.handler = _ts_on_handler;
    state.entry   = _ts_on_entry;
    state.exit    = NULL;
    fsm_set_state(bms.fsm, BMS_TS_ON, &state);

    state.handler = _airn_off_handler;
    state.entry   = _airn_off_entry;
    state.exit    = NULL;
    fsm_set_state(bms.fsm, BMS_AIRN_OFF, &state);

    state.handler = _precharge_handler;
    state.entry   = _precharge_entry;
    state.exit    = _precharge_exit;
    fsm_set_state(bms.fsm, BMS_PRECHARGE, &state);

    state.handler = _on_handler;
    state.entry   = _on_entry;
    state.exit    = _on_exit;
    fsm_set_state(bms.fsm, BMS_ON, &state);

    state.handler = _fault_handler;
    state.entry   = _fault_entry;
    state.run     = _fault_run;
    state.exit    = NULL;
    fsm_set_state(bms.fsm, BMS_FAULT, &state);

    //HAL_TIM_Base_Start_IT(&HTIM_BMS);
    fsm_start(bms.fsm);

    bms.led.port = STATE_LED_GPIO;
    bms.led.pin  = STATE_LED_PIN;
    bms.led.time = HAL_GetTick();

    bms_set_led_blinker();

    HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, GPIO_PIN_RESET);
}

void bms_set_led_blinker() {
    uint8_t pattern_count                    = 0;
    uint8_t state_count                      = 0;
    uint32_t state                           = fsm_get_state(bms.fsm);
    uint16_t pattern[(BMS_NUM_STATES)*2 + 1] = {0};

    pattern[pattern_count++] = 200;  // Off
    while (state_count < state) {
        pattern[pattern_count++] = 200;  // On
        pattern[pattern_count++] = 200;  // Off
        state_count++;
    }
    pattern[pattern_count++] = 1000;  // Big off

    BLINK_SET_PATTERN(bms.led, pattern, pattern_count);
    BLINK_SET_ENABLE(bms.led, true);
    BLINK_SET_REPEAT(bms.led, true);

    blink_reset(&(bms.led));
    HAL_GPIO_WritePin(bms.led.port, bms.led.pin, GPIO_PIN_SET);

    can_car_send(ID_TS_STATUS);    
}
void bms_blink_led() {
    blink_run(&bms.led);
}

void _idle_entry(fsm FSM) {
    can_car_send(ID_TS_STATUS);
    pack_set_airn_off(GPIO_PIN_SET);
    fsm_trigger_event(FSM, BMS_EV_FB_CHECK);
    cli_bms_debug("idle state", 10);
}

void _idle_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_ON:
            fsm_transition(FSM, BMS_TS_ON);
            break;
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
        case BMS_EV_FB_CHECK:
            feedback_check(FEEDBACK_TS_OFF_MASK, FEEDBACK_TS_OFF_VAL, ERROR_FEEDBACK);
            fsm_trigger_event(FSM, BMS_EV_FB_CHECK);
    }
}

void _ts_on_entry(fsm FSM) {
    fsm_trigger_event(FSM, BMS_EV_FB_CHECK);
    cli_bms_debug("ts on state", 11);
}

void _ts_on_handler(fsm FSM, uint8_t event) {
    switch(event) {
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
        case BMS_EV_FB_CHECK:
            //feedback_read(FEEDBACK_TS_ON_VAL);
            if(feedback_check(FEEDBACK_TS_ON_MASK, FEEDBACK_TS_ON_VAL, ERROR_FEEDBACK)) {
                pack_set_ts_on();
                fsm_transition(FSM, BMS_AIRN_OFF);
            } else {
                fsm_trigger_event(FSM, BMS_EV_FB_CHECK);
            }
    }
}

void _airn_off_entry(fsm FSM) {
    fsm_trigger_event(FSM, BMS_EV_FB_CHECK);
    cli_bms_debug("airn close state", 16);
}

void _airn_off_handler(fsm FSM, uint8_t event) {
    switch(event) {
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
        case BMS_EV_FB_CHECK:
            //feedback_read(FEEDBACK_AIRN_OFF_VAL);
            if(feedback_check(FEEDBACK_AIRN_OFF_MASK, FEEDBACK_AIRN_OFF_VAL, ERROR_FEEDBACK)) {
                pack_set_airn_off(GPIO_PIN_RESET);
                fsm_transition(FSM, BMS_PRECHARGE);
            } else {
                fsm_trigger_event(FSM, BMS_EV_FB_CHECK);
            }
    }
}

void _precharge_entry(fsm FSM) {
    // Precharge
    pack_set_pc_start();

    HAL_TIM_OC_Stop_IT(&HTIM_BMS, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&HTIM_BMS, TIM_CHANNEL_2);

    uint32_t cnt = __HAL_TIM_GET_COUNTER(&HTIM_BMS);
    __HAL_TIM_SET_COMPARE(&HTIM_BMS, TIM_CHANNEL_1, (cnt + TIM_MS_TO_TICKS(&HTIM_BMS, PRECHARGE_CHECK_INTERVAL)));
    __HAL_TIM_SET_COMPARE(&HTIM_BMS, TIM_CHANNEL_2, (cnt + TIM_MS_TO_TICKS(&HTIM_BMS, PRECHARGE_TIMEOUT)));

    __HAL_TIM_CLEAR_FLAG(&HTIM_BMS, TIM_IT_CC1); //clears existing interrupts on channel 1
    __HAL_TIM_CLEAR_FLAG(&HTIM_BMS, TIM_IT_CC2); //clears existing interrupts on channel 2

    HAL_TIM_OC_Start_IT(&HTIM_BMS, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&HTIM_BMS, TIM_CHANNEL_2);

    cli_bms_debug("Entered precharge", 18);
}

void _precharge_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_PRECHARGE_TIMEOUT:
            cli_bms_debug("Precharge timeout", 18);
            // send timeout warning
            //NB there's no break; !!!!
        case BMS_EV_TS_OFF:
            fsm_transition(FSM, BMS_IDLE);
            break;

        case BMS_EV_PRECHARGE_CHECK:
            char c[5] = {'\0'};
            itoa(voltage_get_bus() / voltage_get_internal() * PRECHARGE_VOLTAGE_THRESHOLD, c, 10);
            cli_bms_debug(c, 5);

            if (voltage_get_bus() >= voltage_get_internal() * PRECHARGE_VOLTAGE_THRESHOLD && 
                    feedback_check(FEEDBACK_PC_ON_MASK, FEEDBACK_PC_ON_VAL, ERROR_FEEDBACK)) {
                pack_set_precharge_end();
                fsm_transition(bms.fsm, BMS_ON);
                cli_bms_debug("Precharge ok", 18);
            }
            break;
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
    }
}

void _precharge_exit(fsm FSM) {
    HAL_TIM_OC_Stop_IT(&HTIM_BMS, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&HTIM_BMS, TIM_CHANNEL_2);
}

void _on_entry(fsm FSM) {
    fsm_trigger_event(FSM, BMS_EV_FB_CHECK);
    cli_bms_debug("on state", 8);
}

void _on_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_OFF:
            fsm_transition(FSM, BMS_IDLE);
            break;
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
        case BMS_EV_FB_CHECK:
            //feedback_read(FEEDBACK_ON_MASK);
            feedback_check(FEEDBACK_ON_MASK, FEEDBACK_ON_VAL, ERROR_FEEDBACK);
            fsm_trigger_event(FSM, BMS_EV_FB_CHECK);
    }
}

void _on_exit(fsm FSM) {
    pack_set_ts_off();
}

void _fault_entry(fsm FSM) {
    // bms_set_fault(&data->bms);
    HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIO4_GPIO_Port, GPIO4_Pin, GPIO_PIN_SET);
    //can_send_error(&hcan, data->error, data->error_index, &data->pack);
    //cli_bms_debug("HALT", 5);
}

void _fault_run(fsm FSM) {
    feedback_check(FEEDBACK_TS_OFF_MASK, FEEDBACK_TS_OFF_VAL, ERROR_FEEDBACK);
    if (error_get_fatal() == 0) {
        fsm_trigger_event(FSM, BMS_EV_NO_ERRORS);
    }
}

void _fault_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_NO_ERRORS:
            fsm_transition(FSM, BMS_IDLE);
            break;
    }
}
