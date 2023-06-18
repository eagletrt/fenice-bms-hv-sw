/**
 * @file		bal_fsm.c
 * @brief		This file contains the balancing functions
 *
 * @date		May 09, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "bal.h"

// TODO: Start and stop balancing per cellboard

#define CONF_VER  0x01
#define CONF_ADDR 0x01

typedef struct {
    voltage_t threshold;
} bal_params;
bal_params bal_params_default = {BAL_MAX_VOLTAGE_THRESHOLD};

config_t config;

voltage_t bal_get_threshold() {
    return ((bal_params *)config_get(&config))->threshold;
}

void bal_set_threshold(uint16_t thresh) {
    bal_params params = *(bal_params *)config_get(&config);
    params.threshold  = thresh;

    config_set(&config, &params);
    config_write(&config);
}

void bal_fsm_init() {
    config_init(&config, CONF_ADDR, CONF_VER, &bal_params_default, sizeof(bal_params));
}

void off_entry(fsm FSM) {
    bal.target = 0;
    HAL_TIM_OC_Stop_IT(&HTIM_BAL, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&HTIM_BAL, TIM_CHANNEL_2);
    cli_bms_debug("disabling balancing", 20);
    memset(bal.cells, 0, sizeof(bms_balancing_converted_t) * LTC6813_COUNT);
    can_bms_send(BMS_BALANCING_FRAME_ID);
}

void off_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_START:
            fsm_transition(FSM, BAL_COMPUTE);
            break;
        case EV_BAL_STOP:
            memset(bal.cells, 0, sizeof(bms_balancing_converted_t) * LTC6813_COUNT);
            can_bms_send(BMS_BALANCING_FRAME_ID);
            break;
    }
}

void compute_entry(fsm FSM) {
    if (bal_get_cells_to_discharge(
            cell_voltage_get_cells(),
            PACK_CELL_COUNT,
            bal_get_threshold(),
            bal.cells,
            LTC6813_COUNT,
            bal.target) != 0) {
        fsm_transition(FSM, BAL_DISCHARGE);
        return;
    }
    cli_bms_debug("Non si può fare meglio di così.", 34);
    can_bms_send(BMS_BALANCING_FRAME_ID);
    fsm_transition(FSM, BAL_OFF);
}

void discharge_entry(fsm FSM) {
    /*
    fsm_trigger_event(FSM, EV_BAL_CHECK_TIMER);
    bal.discharge_time = HAL_GetTick();
    */

    HAL_TIM_OC_Stop_IT(&HTIM_BAL, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&HTIM_BAL, TIM_CHANNEL_1, __HAL_TIM_GET_COUNTER(&HTIM_BAL) + bal.cycle_length);
    HAL_TIM_OC_Start_IT(&HTIM_BAL, TIM_CHANNEL_1);

    can_bms_send(BMS_BALANCING_FRAME_ID);
    cli_bms_debug("Discharging cells", 18);
}

void discharge_handler(fsm FSM, uint8_t event) {
    switch (event) {
        case EV_BAL_STOP:
            fsm_transition(FSM, BAL_OFF);
            break;
        case EV_BAL_COOLDOWN_START:
            if (bal_are_cells_off_status()) {
                if (bal_get_cells_to_discharge(
                        cell_voltage_get_cells(),
                        PACK_CELL_COUNT,
                        bal_get_threshold() + 5,
                        bal.cells,
                        LTC6813_COUNT,
                        bal.target) == 0) {
                    cli_bms_debug("Non si può fare meglio di così.", 34);
                    can_bms_send(BMS_BALANCING_FRAME_ID);
                    fsm_transition(FSM, BAL_OFF);
                } else {
                    fsm_transition(FSM, BAL_COOLDOWN);
                }
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
    memset(bal.cells, 0, sizeof(bms_balancing_converted_t) * LTC6813_COUNT);
    HAL_TIM_OC_Stop_IT(&HTIM_BAL, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(
        &HTIM_BAL, TIM_CHANNEL_2, __HAL_TIM_GET_COUNTER(&HTIM_BAL) + TIM_MS_TO_TICKS(&HTIM_BAL, BAL_COOLDOWN_DELAY));
    __HAL_TIM_CLEAR_FLAG(&HTIM_BAL, TIM_IT_CC2);  //clears existing interrupts on channel 1
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


void _bal_handle_tim_oc_irq(TIM_HandleTypeDef *htim) {
    switch (htim->Channel) {
        case HAL_TIM_ACTIVE_CHANNEL_1:
            fsm_trigger_event(bal.fsm, EV_BAL_COOLDOWN_START);
            break;
        case HAL_TIM_ACTIVE_CHANNEL_2:
            fsm_trigger_event(bal.fsm, EV_BAL_COOLDOWN_END);
            break;
        default:
            break;
    }
}