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
#include "config.h"
#include "current.h"
#include "error.h"
#include "feedback.h"
#include "main.h"
#include "mainboard_config.h"
#include "pack/pack.h"
#include "peripherals/can_comm.h"
#include "tim.h"
#include "internal_voltage.h"
#include "watchdog.h"


#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define CELLBOARD_DISTR_ADDR 0x50
#define CELLBOARD_DISTR_VER  0x01

bms_fsm bms;
config_t cellboard_distribution;

//------------------------------Declarations------------------------------------------
void bms_set_led_blinker();
void bms_blink_led();

void _idle_entry(fsm FSM);
void _idle_handler(fsm FSM, uint8_t event);

void _airn_close_entry(fsm FSM);
void _airn_close_handler(fsm FSM, uint8_t event);
void _airn_close_exit(fsm FSM);

void _airn_status_entry(fsm FSM);
void _airn_status_handler(fsm FSM, uint8_t event);
void _airn_status_exit(fsm FSM);

void _precharge_entry(fsm FSM);
void _precharge_handler(fsm FSM, uint8_t event);
void _precharge_exit(fsm FSM);

void _on_entry(fsm FSM);
void _on_handler(fsm FSM, uint8_t event);
void _on_exit(fsm FSM);

void _fault_entry(fsm FSM);
void _fault_handler(fsm FSM, uint8_t event);
void _fault_exit(fsm FSM);

void _start_pc_check_timer() {
    uint32_t cnt = __HAL_TIM_GET_COUNTER(&HTIM_BMS);
    __HAL_TIM_SET_COMPARE(&HTIM_BMS, TIM_CHANNEL_1, (cnt + TIM_MS_TO_TICKS(&HTIM_BMS, PRECHARGE_CHECK_INTERVAL)));
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, TIM_IT_CC1);  //clears existing interrupts on channel 1

    HAL_TIM_OC_Start_IT(&HTIM_BMS, TIM_CHANNEL_1);
}

void _start_pc_timeout_timer() {
    uint32_t cnt = __HAL_TIM_GET_COUNTER(&HTIM_BMS);
    __HAL_TIM_SET_COMPARE(&HTIM_BMS, TIM_CHANNEL_2, (cnt + TIM_MS_TO_TICKS(&HTIM_BMS, PRECHARGE_TIMEOUT)));
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, TIM_IT_CC2);  //clears existing interrupts on channel 1

    HAL_TIM_OC_Start_IT(&HTIM_BMS, TIM_CHANNEL_2);
}

void _start_fb_check_timer() {
    uint32_t cnt = __HAL_TIM_GET_COUNTER(&HTIM_BMS);
    __HAL_TIM_SET_COMPARE(&HTIM_BMS, TIM_CHANNEL_3, (cnt + TIM_MS_TO_TICKS(&HTIM_BMS, FB_CHECK_INTERVAL_MS)));
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, TIM_IT_CC3);  //clears existing interrupts on channel 1

    HAL_TIM_OC_Start_IT(&HTIM_BMS, TIM_CHANNEL_3);
}

void _start_fb_timeout_timer() {
    uint32_t cnt = __HAL_TIM_GET_COUNTER(&HTIM_BMS);
    __HAL_TIM_SET_COMPARE(&HTIM_BMS, TIM_CHANNEL_4, (cnt + TIM_MS_TO_TICKS(&HTIM_BMS, FB_TIMEOUT_MS)));
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, TIM_IT_CC4);  //clears existing interrupts on channel 1

    HAL_TIM_OC_Start_IT(&HTIM_BMS, TIM_CHANNEL_4);
}

void _stop_pc_check_timer() {
    HAL_TIM_OC_Stop_IT(&HTIM_BMS, TIM_CHANNEL_1);
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, TIM_IT_CC1);
}

void _stop_pc_timeout_timer() {
    HAL_TIM_OC_Stop_IT(&HTIM_BMS, TIM_CHANNEL_2);
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, TIM_IT_CC2);
}

void _stop_fb_check_timer() {
    HAL_TIM_OC_Stop_IT(&HTIM_BMS, TIM_CHANNEL_3);
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, TIM_IT_CC3);
}

void _stop_fb_timeout_timer() {
    HAL_TIM_OC_Stop_IT(&HTIM_BMS, TIM_CHANNEL_4);
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, TIM_IT_CC4);
}

uint8_t *bms_get_cellboard_distribution() {
    return (uint8_t *)config_get(&cellboard_distribution);
}

void bms_set_cellboard_distribution(uint8_t distribution[static 6]) {
    config_set(&cellboard_distribution, (void *)distribution);
    config_write(&cellboard_distribution);
}

void bms_fsm_init() {
    bms.fsm                = fsm_init(BMS_NUM_STATES, BMS_EV_NUM, &bms_blink_led, &bms_set_led_blinker);
    bms.handcart_connected = false;

    fsm_state state;
    state.run     = NULL;
    state.handler = _idle_handler;
    state.entry   = _idle_entry;
    state.exit    = NULL;
    fsm_set_state(bms.fsm, BMS_IDLE, &state);

    state.handler = _airn_close_handler;
    state.entry   = _airn_close_entry;
    state.exit    = _airn_close_exit;
    fsm_set_state(bms.fsm, BMS_AIRN_CLOSE, &state);

    state.handler = _airn_status_handler;
    state.entry   = _airn_status_entry;
    state.exit    = _airn_status_exit;
    fsm_set_state(bms.fsm, BMS_AIRN_STATUS, &state);

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
    state.exit    = _fault_exit;
    fsm_set_state(bms.fsm, BMS_FAULT, &state);

    //HAL_TIM_Base_Start_IT(&HTIM_BMS);
    fsm_start(bms.fsm);

    bms.led.port = STATE_LED_GPIO;
    bms.led.pin  = STATE_LED_PIN;
    bms.led.time = HAL_GetTick();
    HAL_GPIO_WritePin(bms.led.port, bms.led.pin, GPIO_PIN_RESET);
    bms_set_led_blinker();

    uint8_t cell_distr_default[CELLBOARD_COUNT] = {0, 1, 2, 3, 4, 5};
    config_init(&cellboard_distribution, CELLBOARD_DISTR_ADDR, CELLBOARD_DISTR_VER, cell_distr_default, 6);
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
}
void bms_blink_led() {
    blink_run(&bms.led);
}

void _idle_entry(fsm FSM) {
    can_car_send(PRIMARY_TS_STATUS_FRAME_ID);

    pack_set_default_off(0);
    pack_set_fault(BMS_FAULT_OFF_VALUE);

    _start_fb_check_timer();
    cli_bms_debug("idle state", 10);
}

void _idle_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_ON:
            if (/* !is_watchdog_timed_out() && */ feedback_check(FEEDBACK_TS_OFF_MASK, FEEDBACK_TS_OFF_VAL) == 0)
                fsm_transition(FSM, BMS_AIRN_CLOSE);
            break;
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
        case BMS_EV_FB_CHECK:
            feedback_check(FEEDBACK_TS_OFF_MASK, FEEDBACK_TS_OFF_VAL);
            break;
    }
}

void _idle_exit(fsm FSM) {
    _stop_fb_check_timer();
}

void _airn_close_entry(fsm FSM) {
    _start_fb_check_timer();
    _start_fb_timeout_timer();
    fsm_trigger_event(FSM, BMS_EV_FB_CHECK);

    current_zero();

    cli_bms_debug("airn close state", 16);
}

void _airn_close_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_OFF:
            pack_set_default_off(0);
            fsm_transition(FSM, BMS_IDLE);
            break;
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
        case BMS_EV_FB_CHECK:
            if (/* !is_watchdog_timed_out() && */ feedback_check(FEEDBACK_AIRN_CLOSE_MASK, FEEDBACK_AIRN_CLOSE_VAL) == 0) {
                pack_set_airn_off(AIRN_ON_VALUE);
                fsm_transition(FSM, BMS_AIRN_STATUS);
            }
            break;
        case BMS_EV_FB_TIMEOUT:
            pack_set_default_off(0);
            fsm_transition(FSM, BMS_IDLE);
            break;
    }
}

void _airn_close_exit(fsm FSM) {
    _stop_fb_timeout_timer();
    _stop_fb_check_timer();
}

void _airn_status_entry(fsm FSM) {
    _start_fb_check_timer();
    _start_fb_timeout_timer();
    fsm_trigger_event(FSM, BMS_EV_FB_CHECK);

    cli_bms_debug("airn status state", 17);

}

void _airn_status_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_OFF:
            pack_set_default_off(0);
            fsm_transition(FSM, BMS_IDLE);
            break;
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
        case BMS_EV_FB_CHECK:
            if (/* !is_watchdog_timed_out() && */ feedback_check(FEEDBACK_AIRN_STATUS_MASK, FEEDBACK_AIRN_STATUS_VAL) == 0) {
                pack_set_precharge(PRECHARGE_ON_VALUE);
                fsm_transition(FSM, BMS_PRECHARGE);
            }
            break;
        case BMS_EV_FB_TIMEOUT:
            pack_set_default_off(0);
            fsm_transition(FSM, BMS_IDLE);
            break;
    }
}

void _airn_status_exit(fsm FSM) {
    _stop_fb_check_timer();
    _stop_fb_timeout_timer();
}

uint32_t tick;

void _precharge_entry(fsm FSM) {
    _start_pc_check_timer();
    _start_pc_timeout_timer();
    _start_fb_check_timer();
    _start_fb_timeout_timer();
    can_car_send(PRIMARY_TS_STATUS_FRAME_ID);
    fsm_trigger_event(FSM, BMS_EV_PRECHARGE_CHECK);

    tick = HAL_GetTick();

    cli_bms_debug("Entered precharge", 18);
}

bool is_fb_timeout_triggered_old = false;
void _precharge_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_PRECHARGE_TIMEOUT:
            cli_bms_debug("Precharge timeout", 18);
            // send timeout warning
            //NB there's no break; !!!!
        case BMS_EV_TS_OFF:
            pack_set_default_off(0);
            fsm_transition(FSM, BMS_IDLE);
            break;

        case BMS_EV_FB_CHECK:
            if (/* !is_watchdog_timed_out() && */ feedback_check(FEEDBACK_PC_ON_MASK, FEEDBACK_PC_ON_VAL) == 0) {
                _stop_fb_timeout_timer();
            }
            break;

        case BMS_EV_FB_TIMEOUT:
            if (is_fb_timeout_triggered_old)
                is_fb_timeout_triggered_old = false;
            else {
                cli_bms_debug("Precharge FB timeout", 20);
                pack_set_default_off(0);
                fsm_transition(FSM, BMS_IDLE);
            }
            break;

        case BMS_EV_PRECHARGE_CHECK:
            char c[5] = {'\0'};
            snprintf(c, 5, "%4.2f", CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp()) / (CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_bat()) * PRECHARGE_VOLTAGE_THRESHOLD));
            cli_bms_debug(c, 5);

            // Wait until
            if (HAL_GetTick() - tick > 5000 && (
                (!bms.handcart_connected && internal_voltage_get_tsp() > 0 &&
                 CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp()) >= CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_bat()) * PRECHARGE_VOLTAGE_THRESHOLD) ||
                (bms.handcart_connected && CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp()) > 0 &&
                 CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp()) >= CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_bat()) * PRECHARGE_VOLTAGE_THRESHOLD_CARELINO))) {

                if (/* !is_watchdog_timed_out() && */ feedback_check(FEEDBACK_PC_ON_MASK, FEEDBACK_PC_ON_VAL) == 0) {
                    pack_set_airp_off(AIRP_ON_VALUE);
                    // pack_set_precharge(PRECHARGE_OFF_VALUE);
                    // _stop_fb_check_timer();
                    cli_bms_debug("Precharge ok", 18);
                    fsm_transition(bms.fsm, BMS_ON);
                }
            }
            break;
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
    }
}

void _precharge_exit(fsm FSM) {
    _stop_pc_check_timer();
    _stop_pc_timeout_timer();
    _stop_fb_check_timer();
}

void _on_entry(fsm FSM) {
    _start_fb_timeout_timer();
    can_car_send(PRIMARY_TS_STATUS_FRAME_ID);

    cli_bms_debug("on state", 8);
}

size_t feed = 0;
void _on_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_TS_OFF:
            pack_set_default_off(0);
            fsm_transition(FSM, BMS_IDLE);
            break;
        case BMS_EV_FAULT:
            fsm_transition(FSM, BMS_FAULT);
            break;
        case BMS_EV_FB_CHECK:
            if (/* is_watchdog_timed_out() || */ (feed = feedback_check(FEEDBACK_ON_MASK, FEEDBACK_ON_VAL)) != 0) {
                char msg[50] = { 0 };
                for (size_t i = 1; i < FEEDBACK_N; i++) {
                    if (feed & (1 << i))
                        sprintf(msg + strlen(msg), "%d %d\r\n", i, feedbacks[i]);
                }
                if (strlen(msg) > 0)
                    cli_bms_debug(msg, strlen(msg));

                can_car_send(PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID);
                can_car_send(PRIMARY_TS_STATUS_FRAME_ID);
                cli_bms_debug("FB check sborato...!\r\n", strlen("FB check sborato...!\r\n"));
                pack_set_default_off(0);
                fsm_transition(FSM, BMS_IDLE);
            }
            break;
        case BMS_EV_FB_TIMEOUT:
            if (/* is_watchdog_timed_out() || */ (feed = feedback_check(FEEDBACK_ON_MASK, FEEDBACK_ON_VAL)) != 0) {
                char msg[50] = { 0 };
                for (size_t i = 0; i < FEEDBACK_N; i++) {
                    if (feed & (1 << i))
                        sprintf(msg + strlen(msg), "%d %d\r\n", i, feedbacks[i]);
                }
                if (strlen(msg) > 0)
                    cli_bms_debug(msg, strlen(msg));

                can_car_send(PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID);
                can_car_send(PRIMARY_TS_STATUS_FRAME_ID);
                cli_bms_debug("FB timeout sborato...!\r\n", strlen("FB timeout sborato...!\r\n"));
                pack_set_default_off(0);
                fsm_transition(FSM, BMS_IDLE);
                return;
            }
            _start_fb_check_timer();
            break;
    }
}

void _on_exit(fsm FSM) {
    can_car_send(PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID);
    _stop_fb_timeout_timer();
    _stop_fb_check_timer();
}

void _fault_entry(fsm FSM) {
    pack_set_fault(BMS_FAULT_ON_VALUE);

    pack_set_default_off(0);
    _start_fb_check_timer();

    cli_bms_debug("fault state", 11);
}

void _fault_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case BMS_EV_FB_CHECK:
            if (error_get_fatal() == 0 && /* !is_watchdog_timed_out() && */ feedback_check(FEEDBACK_FAULT_EXIT_MASK, FEEDBACK_FAULT_EXIT_VAL) == 0)
                fsm_transition(FSM, BMS_IDLE);
            break;
    }
}

void _fault_exit(fsm FSM) {
    _stop_fb_check_timer();
    pack_set_fault(BMS_FAULT_OFF_VALUE);
}

void _bms_handle_tim_oc_irq(TIM_HandleTypeDef *htim) {
    uint32_t cnt = __HAL_TIM_GetCounter(htim);
    switch (htim->Channel) {
        case HAL_TIM_ACTIVE_CHANNEL_1:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (cnt + TIM_MS_TO_TICKS(htim, PRECHARGE_CHECK_INTERVAL)));
            fsm_trigger_event(bms.fsm, BMS_EV_PRECHARGE_CHECK);
            break;
        case HAL_TIM_ACTIVE_CHANNEL_2:
            fsm_trigger_event(bms.fsm, BMS_EV_PRECHARGE_TIMEOUT);
            break;
        case HAL_TIM_ACTIVE_CHANNEL_3:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_3, (cnt + TIM_MS_TO_TICKS(htim, FB_CHECK_INTERVAL_MS)));
            fsm_trigger_event(bms.fsm, BMS_EV_FB_CHECK);
            break;
        case HAL_TIM_ACTIVE_CHANNEL_4:
            if (fsm_get_state(bms.fsm) == BMS_AIRN_STATUS)
                is_fb_timeout_triggered_old = true;
            fsm_trigger_event(bms.fsm, BMS_EV_FB_TIMEOUT);
            break;
        default:
            break;
    }
}