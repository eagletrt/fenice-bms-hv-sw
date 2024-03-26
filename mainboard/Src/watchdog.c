/**
 * @file watchdog.c
 * @brief Functions and structure to handle watchdog timeouts
 * 
 * @date Jul 06, 2023
 * 
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */
#include "watchdog.h"

#include <stddef.h>
#include <string.h> // Used for memset in primary and bms watchdogs

#include "primary_network.h"
#include "primary_watchdog.h"
#include "bms_network.h"
#include "bms_watchdog.h"
#include "bms_fsm.h"
#include "cli_bms.h"

#define PRIMARY_WATCHDOG_IDS_SIZE 1
#define BMS_WATCHDOG_IDS_SIZE 1

primary_watchdog car_watchdog = { 0 };
bms_watchdog cell_watchdog = { 0 };

bool car_watchdog_timed_out = false;
bool cell_watchdog_timed_out = false;

/** @brief Primary IDs monitored by the watchdog */
const uint16_t watchdog_primary_ids[PRIMARY_WATCHDOG_IDS_SIZE] = {
    PRIMARY_CAR_STATUS_FRAME_ID,
    // TODO: Add handcart watchdog only when using handcart
    // PRIMARY_HANDCART_STATUS_FRAME_ID
};
/** @brief Bms IDs monitored by the watchdog */
const uint16_t watchdog_bms_ids[BMS_WATCHDOG_IDS_SIZE] = {
    BMS_BOARD_STATUS_FRAME_ID
};

void watchdog_init() {
    for (size_t i = 0; i < PRIMARY_WATCHDOG_IDS_SIZE; i++) {
        uint16_t id = watchdog_primary_ids[i];
        CANLIB_BITSET_ARRAY(car_watchdog.activated, primary_watchdog_index_from_id(id));
    }
    for (size_t i = 0; i < BMS_WATCHDOG_IDS_SIZE; i++) {
        uint16_t id = watchdog_bms_ids[i];
        CANLIB_BITSET_ARRAY(cell_watchdog.activated, bms_watchdog_index_from_id(id));
    }
}

bool is_watchdog_timed_out() {
    return car_watchdog_timed_out || cell_watchdog_timed_out;
}

void watchdog_reset(uint16_t id) {
    if (primary_id_is_message(id)) {
        // Reset errors
        // error_reset(ERROR_CAN, 0);

        // Reset primary watchdog
        primary_watchdog_reset(&car_watchdog, id, HAL_GetTick());
        car_watchdog_timed_out = false;
    }
    else if (bms_id_is_message(id)) {
        // Reset errors
        // error_reset(ERROR_CAN, 1);

        // Reset bms watchdog
        bms_watchdog_reset(&cell_watchdog, id, HAL_GetTick());
        cell_watchdog_timed_out = false;
    }
}

void watchdog_routine() {
    primary_watchdog_timeout(&car_watchdog, HAL_GetTick());
    bms_watchdog_timeout(&cell_watchdog, HAL_GetTick());

#if !defined(WATCHDOG_IGNORE_PRIMARY) && !defined(WATCHDOG_IGNORE)
    for (size_t i = 0; i < PRIMARY_WATCHDOG_IDS_SIZE; i++) {
        // Check if the primary watchdog has timed out
        uint16_t id = watchdog_primary_ids[i];
        bool timed_out = CANLIB_BITTEST_ARRAY(car_watchdog.timeout, primary_watchdog_index_from_id(id));
        if (timed_out) {
            char msg[50] = { 0 };
            sprintf(msg, "Car watchdog id: %d\n", id);
            cli_bms_debug(msg, strlen(msg));

            car_watchdog_timed_out = true;

            // Set error
            // error_set(ERROR_CAN, 0, HAL_GetTick());

            // Send TS off request
            set_ts_request.is_new = true;
            set_ts_request.next_state = STATE_IDLE;
        }
    }
#endif // WATCHDOG_IGNORE_PRIMARY && WATCHDOG_IGNORE

#if !defined(WATCHDOG_IGNORE_BMS) && !defined(WATCHDOG_IGNORE)
    for (size_t i = 0; i < BMS_WATCHDOG_IDS_SIZE; i++) {
        // Check if the bms watchdog has timed out
        uint16_t id = watchdog_bms_ids[i];
        bool timed_out = CANLIB_BITTEST_ARRAY(cell_watchdog.timeout, bms_watchdog_index_from_id(id));
        if (timed_out) {
            char msg[50] = { 0 };
            sprintf(msg, "Cell watchdog id: %d\n", id);
            cli_bms_debug(msg, strlen(msg));

            cell_watchdog_timed_out = true;
            // Set error
            // error_set(ERROR_CAN, 1, HAL_GetTick());

            // Send TS off request
            set_ts_request.is_new = true;
            set_ts_request.next_state = STATE_IDLE;
        }
    }
#endif // WATCHDOG_IGNORE_BMS && WATCHDOG_IGNORE
}
