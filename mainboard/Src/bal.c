/**
 * @file		bal_fsm.c
 * @brief		This file contains the balancing functions
 *
 * @date		May 09, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "bal.h"

#include <string.h>

#include "config.h"
#include "can_comm.h"
#include "cli_bms.h"

// TODO: Start and stop balancing per cellboard

#define CONF_VER  0x01
#define CONF_ADDR 0x01

typedef struct {
    voltage_t threshold;
} bal_params;
bal_params bal_params_default = { BAL_MAX_VOLTAGE_THRESHOLD };

config_t config;
uint8_t is_balancing;
bool set_balancing;

voltage_t bal_get_threshold() {
    return ((bal_params *)config_get(&config))->threshold;
}
void bal_set_threshold(uint16_t thresh) {
    thresh = MAX(thresh, BAL_MAX_VOLTAGE_THRESHOLD);

    bal_params params = *(bal_params *)config_get(&config);
    params.threshold  = thresh;

    config_set(&config, &params);
    config_write(&config);
}
bool bal_is_balancing() {
    return is_balancing != 0;
}
void bal_set_is_balancing(uint8_t cellboard_id, bool is_bal) {
    // Set the bit of the cellboard who's balancing
    is_balancing &= ~(1 << cellboard_id);
    is_balancing |= (1 << cellboard_id) & is_bal;
}
bool bal_need_balancing() {
    return set_balancing;
}

void bal_init() {
    is_balancing = 0;
    set_balancing = false;
    config_init(&config, CONF_ADDR, CONF_VER, &bal_params_default, sizeof(bal_params));
}

void bal_start() {
    cli_bms_debug("Starting balancing...\r\n", strlen("Starting balancing...\r\n"));
    set_balancing = true;
    can_bms_send(BMS_SET_BALANCING_STATUS_FRAME_ID);
    set_balancing = false;
}
void bal_stop() {
    set_balancing = false;
    can_bms_send(BMS_SET_BALANCING_STATUS_FRAME_ID);
}