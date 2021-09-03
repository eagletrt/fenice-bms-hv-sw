/**
 * @file		bal_fsm.c
 * @brief		This file contains the balancing functions
 *
 * @date		Jul 07, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "bal_fsm.h"

#include "bal.h"
#include "can_comms.h"
#include "ltc6813_utils.h"
#include "main.h"
#include "spi.h"
#include "volt.h"

bal_fsm bal;

void off_entry(fsm handle);
void off_handler(fsm handle, uint8_t event);
void compute_entry(fsm handle);
void discharge_entry(fsm handle);
void discharge_handler(fsm handle, uint8_t event);
void transition_callback(fsm handle);

void bal_fsm_init() {
    bal.cycle_length = DCTO_30S;
    bal.fsm          = fsm_init(BAL_NUM_STATES, BAL_EV_NUM, transition_callback);

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
}

void transition_callback(fsm handle) {
    can_send(ID_BOARD_STATUS);
}

void off_entry(fsm handle) {
    uint16_t cells[PACK_CELL_COUNT] = {0};
    bal.cells_length                = 0;

    ltc6813_set_balancing(&LTC6813_SPI, cells, PACK_CELL_COUNT, 0);
}

void off_handler(fsm handle, uint8_t event) {
    switch (event) {
        case EV_BAL_START:
            fsm_transition(handle, BAL_COMPUTE);
            break;
    }
}

void compute_entry(fsm handle) {
    bal.cells_length = bal_get_cells_to_discharge(voltages, PACK_CELL_COUNT, BAL_MAX_VOLTAGE_THRESHOLD, bal.cells);

    if (bal.cells_length != 0) {
        fsm_transition(handle, BAL_DISCHARGE);
        return;
    }
    //cli_bms_debug("Non si può fare meglio di così.", 34);
    fsm_transition(handle, BAL_OFF);
}

void discharge_entry(fsm handle) {
    ltc6813_set_balancing(&LTC6813_SPI, bal.cells, bal.cells_length, bal.cycle_length);
    bal.discharge_time = HAL_GetTick();
    //cli_bms_debug("Discharging cells", 18);
}

void discharge_handler(fsm handle, uint8_t event) {
    switch (event) {
        case EV_BAL_STOP:
            fsm_transition(handle, BAL_OFF);

            break;
        case EV_BAL_CHECK_TIMER:
            // TODO: use timers or something
            if (HAL_GetTick() - bal.discharge_time >= DCTO_TO_MILLIS[bal.cycle_length]) {
                bal.cells_length = 0;
            }
            if (HAL_GetTick() - bal.discharge_time >= DCTO_TO_MILLIS[bal.cycle_length] + BAL_COOLDOWN_DELAY) {
                fsm_transition(handle, BAL_COMPUTE);
            }

            break;
    }
}
