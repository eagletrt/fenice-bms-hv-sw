/**
 * @file bal_fsm.c
 * @brief This file contains the balancing functions
 *
 * @date Jul 07, 2021
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "bal_fsm.h"

#include <string.h>

#include "peripherals/ltc6813.h"
#include "spi.h"
#include "bal.h"
#include "volt.h"

bal_fsm bal;

void off_entry(fsm FSM);
void off_handler(fsm FSM, uint8_t event);
void compute_entry(fsm FSM);
void discharge_entry(fsm FSM);
void discharge_handler(fsm FSM, uint8_t event);
void discharge_exit(fsm FSM);
void cooldown_entry(fsm FSM);
void cooldown_handler(fsm FSM, uint8_t event);
void cooldown_exit(fsm FSM);
void transition_callback(fsm FSM);

uint16_t bal_fsm_get_threshold() {
    return bal.threshold;
}

void bal_fsm_set_threshold(uint16_t threshold) {
    bal.threshold = MIN(threshold, BAL_MAX_VOLTAGE_THRESHOLD);
}

void bal_fsm_init() {
    bal.cells = 0;
    bal.status = bms_board_status_balancing_status_OFF;

    bal.cycle_length  = DCTO_30S;
    bal.fsm           = fsm_init(BAL_NUM_STATES, BAL_EV_NUM, NULL, transition_callback);
    bal.is_s_pin_high = 0;

    bal.discharge_time = 0;
    bal.target = 0;
    bal.threshold = BAL_MAX_VOLTAGE_THRESHOLD;

    fsm_state state;

    state.handler = off_handler;
    state.entry   = off_entry;
    state.run     = NULL;
    state.exit    = NULL;
    fsm_set_state(bal.fsm, BAL_OFF, &state);

    state.handler = NULL;
    state.entry   = compute_entry;
    state.exit    = NULL;
    fsm_set_state(bal.fsm, BAL_COMPUTE, &state);

    state.handler = discharge_handler;
    state.entry   = discharge_entry;
    state.run     = NULL;
    state.exit    = discharge_exit;
    fsm_set_state(bal.fsm, BAL_DISCHARGE, &state);

    state.handler = cooldown_handler;
    state.entry   = cooldown_entry;
    state.exit    = cooldown_exit;
    fsm_set_state(bal.fsm, BAL_COOLDOWN, &state);

    fsm_start(bal.fsm);

    // Set autoreload of balancing timer
    __HAL_TIM_SET_AUTORELOAD(&TIM_DISCHARGE, TIM_MS_TO_TICKS(&TIM_DISCHARGE, BAL_CYCLE_LENGTH));
}

void transition_callback(fsm FSM) {
    can_send(BMS_BOARD_STATUS_FRAME_ID);
}

void off_entry(fsm FSM) {
    bal.target = 0;

    // Stop cooldown timers
    HAL_TIM_OC_Stop_IT(&TIM_COOLDOWN, TIM_COOLDOWN_START_CHANNEL);
    HAL_TIM_OC_Stop_IT(&TIM_COOLDOWN, TIM_COOLDOWN_STOP_CHANNEL);
    
    bal.cells = 0;

    // Reset balancing
    uint32_t cells = 0;
    ltc6813_set_balancing(&LTC6813_SPI, cells, 0);
}

void off_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_START:
            fsm_transition(FSM, BAL_COMPUTE);
            break;
        case EV_BAL_STOP:
            bal.cells = 0;
    }
}

void compute_entry(fsm FSM) {
    // Get cells to discharge
    uint16_t discharge_count = bal_get_cells_to_discharge(
        volt_get_volts(),
        &bal.cells,
        bal.target,
        bal.threshold
    );

    if (discharge_count > 0) {
        fsm_transition(FSM, BAL_DISCHARGE);
        return;
    }
    fsm_transition(FSM, BAL_OFF);
}

void discharge_entry(fsm FSM) {
    // Start balancing
    bal.is_s_pin_high = 1;
    ltc6813_set_balancing(&LTC6813_SPI, bal.cells, bal.cycle_length);

    // Reset and start the discharge timer
    __HAL_TIM_SetCounter(&TIM_DISCHARGE, 0U);
    __HAL_TIM_SetCompare(&TIM_DISCHARGE, TIM_CHANNEL_1, TIM_MS_TO_TICKS(&TIM_DISCHARGE, BAL_TIME_ON));
    __HAL_TIM_CLEAR_IT(&TIM_DISCHARGE, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&TIM_DISCHARGE);
    HAL_TIM_OC_Start_IT(&TIM_DISCHARGE, TIM_CHANNEL_1);

    // Reset and start cooldown start timer channel
    HAL_TIM_OC_Stop_IT(&TIM_COOLDOWN, TIM_COOLDOWN_START_CHANNEL);
    __HAL_TIM_SET_COMPARE(&TIM_COOLDOWN, TIM_COOLDOWN_START_CHANNEL, __HAL_TIM_GET_COUNTER(&TIM_COOLDOWN) + bal.cycle_length);
    HAL_TIM_OC_Start_IT(&TIM_COOLDOWN, TIM_COOLDOWN_START_CHANNEL);
}

void discharge_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_STOP:
            fsm_transition(FSM, BAL_OFF);
            break;
        case EV_BAL_COOLDOWN_START:
            if (bal.status == BMS_BOARD_STATUS_BALANCING_STATUS_OFF_CHOICE) {
                // Get cells to discharge
                uint16_t discharge_count = bal_get_cells_to_discharge(
                    volt_get_volts(),
                    &bal.cells,
                    bal.target,
                    bal.threshold
                );
                if (discharge_count > 0)
                    fsm_transition(FSM, BAL_COOLDOWN);
                else
                    fsm_transition(FSM, BAL_OFF);
            }
            else
                fsm_trigger_event(FSM, EV_BAL_COOLDOWN_START);
            break;
    }
}

void discharge_exit(fsm FSM) {
    bal.is_s_pin_high = 0;
    bal.cells = 0;

    // Stop discharge timer
    HAL_TIM_Base_Stop_IT(&TIM_DISCHARGE);
    HAL_TIM_OC_Stop_IT(&TIM_DISCHARGE, TIM_CHANNEL_1);

    // Stop cooldown start timer channel
    HAL_TIM_OC_Stop_IT(&TIM_COOLDOWN, TIM_COOLDOWN_START_CHANNEL);
}

void cooldown_entry(fsm FSM) {
    bal.cells = 0;

    // Stop cooldown start timer channel
    HAL_TIM_OC_Stop_IT(&TIM_COOLDOWN, TIM_COOLDOWN_START_CHANNEL);
    __HAL_TIM_SET_COMPARE(&TIM_COOLDOWN,
        TIM_COOLDOWN_STOP_CHANNEL,
        __HAL_TIM_GET_COUNTER(&TIM_COOLDOWN) + TIM_MS_TO_TICKS(&TIM_COOLDOWN, BAL_COOLDOWN_DELAY));
    __HAL_TIM_CLEAR_FLAG(&TIM_COOLDOWN, TIM_IT_CC2); // clears existing interrupts on channel 1
    
    // Start cooldown stop timer channel
    HAL_TIM_OC_Start_IT(&TIM_COOLDOWN, TIM_COOLDOWN_STOP_CHANNEL);
}

void cooldown_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_STOP:
            fsm_transition(FSM, BAL_OFF);
            break;
        case EV_BAL_COOLDOWN_END:
            fsm_transition(FSM, BAL_COMPUTE);
            break;
    }
}

void cooldown_exit(fsm FSM) {
    // Stop cooldown stop timer channel
    HAL_TIM_OC_Stop_IT(&TIM_COOLDOWN, TIM_COOLDOWN_STOP_CHANNEL);
}

void bal_timers_handler(TIM_HandleTypeDef *htim, fsm handle) {
    // if (htim->Instance == TIM_DISCHARGE.Instance) {
    //     bal.cells = 0;
    //     fsm_trigger_event(bal.fsm, EV_BAL_STOP);
    // }
    // else
    if (htim->Instance == TIM_COOLDOWN.Instance) {
        switch (htim->Channel) {
            case HAL_TIM_ACTIVE_CHANNEL_1:
                // Start cooldown
                fsm_trigger_event(bal.fsm, EV_BAL_COOLDOWN_START);
                break;
            case HAL_TIM_ACTIVE_CHANNEL_2:
                // Stop cooldown
                fsm_trigger_event(bal.fsm, EV_BAL_COOLDOWN_END);
                break;
            default:
                break;
        }
    }
}

void bal_oc_timer_handler(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t cmp = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1);

        if (bal.is_s_pin_high) {
            uint32_t cells = 0;
            ltc6813_set_balancing(&LTC6813_SPI, cells, 0);
            __HAL_TIM_SetCompare(htim, TIM_CHANNEL_1, cmp + TIM_MS_TO_TICKS(htim, BAL_TIME_OFF));
            bal.is_s_pin_high = 0;
        } else {
            ltc6813_set_balancing(&LTC6813_SPI, bal.cells, bal.cycle_length);
            __HAL_TIM_SetCompare(htim, TIM_CHANNEL_1, cmp + TIM_MS_TO_TICKS(htim, BAL_TIME_ON));
            bal.is_s_pin_high = 1;
        }
    }
}

bool bal_is_cells_empty() {
    return bal.cells == 0;
}
