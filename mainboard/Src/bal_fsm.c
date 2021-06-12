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

fsm bal_fsm;
config_t config;

uint32_t cycle_length = BAL_CYCLE_LENGTH;

uint32_t discharge_start = 0;

uint16_t trans_stop(fsm *FSM);
uint16_t do_off(fsm *FSM);
uint16_t do_compute(fsm *FSM);
uint16_t do_discharge(fsm *FSM);
uint16_t do_cooldown(fsm *FSM);

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
    fsm_init(&bal_fsm, BAL_NUM_STATES);

    bal_fsm.state_table[BAL_OFF][BAL_OFF]     = &do_off;
    bal_fsm.state_table[BAL_OFF][BAL_COMPUTE] = &do_compute;

    bal_fsm.state_table[BAL_COMPUTE][BAL_DISCHARGE] = &do_discharge;
    bal_fsm.state_table[BAL_COMPUTE][BAL_OFF]       = &trans_stop;

    bal_fsm.state_table[BAL_DISCHARGE][BAL_DISCHARGE] = &do_discharge;
    bal_fsm.state_table[BAL_DISCHARGE][BAL_COMPUTE]   = &do_compute;
    bal_fsm.state_table[BAL_DISCHARGE][BAL_OFF]       = &trans_stop;

    bal_fsm.state_table[BAL_COOLDOWN][BAL_COOLDOWN] = &do_cooldown;
    bal_fsm.state_table[BAL_COOLDOWN][BAL_COMPUTE]  = &do_compute;
    bal_fsm.state_table[BAL_COOLDOWN][BAL_OFF]      = &trans_stop;

    config_init(&config, CONF_ADDR, &bal_params_default, sizeof(bal_params));
}

uint16_t trans_stop(fsm *FSM) {
    uint16_t cells[PACK_CELL_COUNT] = {0};
    ltc6813_set_balancing(&LTC6813_PERIPHERAL, cells, 0);
    return BAL_OFF;
}
uint16_t do_off(fsm *FSM) {
    return BAL_OFF;
}

uint16_t do_compute(fsm *FSM) {
    uint16_t cells[PACK_CELL_COUNT];

    if (bal_get_cells_to_discharge(pack_get_voltages(), PACK_CELL_COUNT, bal_get_threshold(), cells) != 0) {
        ltc6813_set_balancing(&LTC6813_PERIPHERAL, cells, cycle_length);
        discharge_start = HAL_GetTick();

        cli_bms_debug("Discharging cells", 18);
        return BAL_DISCHARGE;
    }
    cli_bms_debug("Non si può fare meglio di così.", 34);
    return BAL_OFF;
}

uint16_t do_discharge(fsm *FSM) {
    if (discharge_start - HAL_GetTick() >= cycle_length) {
        return BAL_COOLDOWN;
    }
    return BAL_DISCHARGE;
}

uint16_t do_cooldown(fsm *FSM) {
    if (discharge_start - HAL_GetTick() >= cycle_length + BAL_COOLDOWN_DELAY) {
        return BAL_COMPUTE;
    }
    return BAL_COOLDOWN;
}