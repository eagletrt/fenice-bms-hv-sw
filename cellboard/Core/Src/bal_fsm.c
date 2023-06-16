/**
 * @file		bal_fsm.c
 * @brief		This file contains the balancing functions
 *
 * @date		Jul 07, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "bal_fsm.h"

#include "can_comms.h"
#include "ltc6813_utils.h"
#include "main.h"
#include "spi.h"
#include "volt.h"

#include <string.h>

bal_fsm bal;

void off_entry(fsm handle);
void off_handler(fsm handle, uint8_t event);
void discharge_entry(fsm handle);
void discharge_handler(fsm handle, uint8_t event);
void discharge_exit(fsm handle);
void transition_callback(fsm handle);

void bal_fsm_init() {
    bal.cycle_length  = DCTO_30S;
    bal.fsm           = fsm_init(BAL_NUM_STATES, BAL_EV_NUM, NULL, NULL);
    bal.is_s_pin_high = 0;

    fsm_state state;

    state.handler = off_handler;
    state.entry   = off_entry;
    state.run     = NULL;
    state.exit    = NULL;
    fsm_set_state(bal.fsm, BAL_OFF, &state);

    state.handler = discharge_handler;
    state.entry   = discharge_entry;
    state.run     = NULL;
    state.exit    = discharge_exit;
    fsm_set_state(bal.fsm, BAL_DISCHARGE, &state);

    __HAL_TIM_SET_AUTORELOAD(&TIM_DISCHARGE, TIM_MS_TO_TICKS(&TIM_DISCHARGE, BAL_CYCLE_LENGTH));
}

void transition_callback(fsm handle) {
    can_send(BMS_BOARD_STATUS_FRAME_ID); // 0); //TODO: sborato
}

void off_entry(fsm handle) {
    bms_balancing_converted_t cells = { 0 };
    ltc6813_set_balancing(&LTC6813_SPI, cells, 0);
}

void off_handler(fsm handle, uint8_t event) {
    switch (event) {
        case EV_BAL_START:
            fsm_transition(handle, BAL_DISCHARGE);
            break;
    }
}

void discharge_entry(fsm handle) {
    bal.is_s_pin_high = 1;
    ltc6813_set_balancing(&LTC6813_SPI, bal.cells, bal.cycle_length);
    //  Resetting and starting the DISCHARGE_TIMER
    __HAL_TIM_SetCounter(&TIM_DISCHARGE, 0U);
    __HAL_TIM_SetCompare(&TIM_DISCHARGE, TIM_CHANNEL_1, TIM_MS_TO_TICKS(&TIM_DISCHARGE, BAL_TIME_ON));
    __HAL_TIM_CLEAR_IT(&TIM_DISCHARGE, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&TIM_DISCHARGE);
    HAL_TIM_OC_Start_IT(&TIM_DISCHARGE, TIM_CHANNEL_1);
    //cli_bms_debug("Discharging cells", 18);
}

void discharge_handler(fsm handle, uint8_t event) {
    switch (event) {
        case EV_BAL_STOP:
            fsm_transition(handle, BAL_OFF);
            break;
    }
}

void discharge_exit(fsm handle) {
    bal.is_s_pin_high = 0;
    HAL_TIM_Base_Stop_IT(&TIM_DISCHARGE);
    HAL_TIM_OC_Stop_IT(&TIM_DISCHARGE, TIM_CHANNEL_1);
}
/**
  * @brief  When DISCHARGE_TIMER elapses the discharge is complete 
  *         so the cells_length will be set to zero
  * @retval None
  */
void bal_timers_handler(TIM_HandleTypeDef *htim, fsm handle) {
    memset(&bal.cells, 0, sizeof(bms_balancing_converted_t));
    fsm_trigger_event(bal.fsm, EV_BAL_STOP);
}

void bal_oc_timer_handler(TIM_HandleTypeDef *htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t cmp = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1);
        if (bal.is_s_pin_high) {
            bms_balancing_converted_t cells = { 0 };
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

uint8_t bal_is_cells_empty() {
    uint32_t empty = bal.cells.cells_cell0 |
        bal.cells.cells_cell1 |
        bal.cells.cells_cell2 |
        bal.cells.cells_cell3 |
        bal.cells.cells_cell4 |
        bal.cells.cells_cell5 |
        bal.cells.cells_cell6 |
        bal.cells.cells_cell7 |
        bal.cells.cells_cell8 |
        bal.cells.cells_cell9 |
        bal.cells.cells_cell10 |
        bal.cells.cells_cell11 |
        bal.cells.cells_cell12 |
        bal.cells.cells_cell13 |
        bal.cells.cells_cell14 |
        bal.cells.cells_cell15 |
        bal.cells.cells_cell16 |
        bal.cells.cells_cell17;
    return empty == 0;
}
