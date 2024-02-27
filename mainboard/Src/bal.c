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

#include "can_comm.h"
#include "cli_bms.h"
#include "fans_buzzer.h"
#include "temperature.h"
#include "measures.h"

/** @brief Current balancing status */
struct {
    uint8_t status: CELLBOARD_COUNT; // Each bit represent a celloboard status
    voltage_t threshold;
} bal_status;

BalRequest bal_request;


void bal_init(void) {
    bal_status.status = 0;
    bal_status.threshold = BAL_THRESHOLD_DEFAULT;

    bal_request.status = false;
    bal_request.threshold = BAL_THRESHOLD_DEFAULT;
    bal_request.is_new = false;
}
void bal_change_status_request(bool status, voltage_t threshold) {
    bal_request.status = status;
    bal_request.threshold = threshold <= BAL_THRESHOLD_MAX && threshold >= BAL_THRESHOLD_MIN ? threshold : BAL_THRESHOLD_DEFAULT;
    bal_request.is_new = true;
}
voltage_t bal_get_threshold(void) {
    return bal_status.threshold;
}
bool bal_is_balancing(void) {
    return (bool)bal_status.status;
}
BalRequest bal_get_request(void) {
    return bal_request;
}
void bal_update_status(uint8_t cellboard, bool status) {
    if (cellboard >= CELLBOARD_COUNT)
        return;
    
    bal_status.status &= ~(1 << cellboard);
    bal_status.status |= (1 << cellboard) & status;
}
void bal_stop(void) {
    bal_change_status_request(false, BAL_THRESHOLD_DEFAULT);
    bal_routine();
}
void bal_routine(void) {
    if (bal_request.is_new) {
        // Update the balancing status if needed
        if (bal_is_balancing() != bal_request.status) {
            bal_status.threshold = bal_request.threshold;
            can_bms_send(BMS_SET_BALANCING_STATUS_FRAME_ID);
        }
        // Reset request
        bal_request.is_new = false;
    }
}
