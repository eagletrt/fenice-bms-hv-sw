/**
 * @file bal.h
 * @brief This file contains the balancing functions
 *
 * @date May 09, 2021
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef BAL_H
#define BAL_H

#include <inttypes.h>
#include <stdbool.h>

#include "../../fenice_config.h"

#define BAL_THRESHOLD_DEFAULT 300 // mV * 10
#define BAL_THRESHOLD_MIN 50 // mV * 10
#define BAL_THRESHOLD_MAX 2000 // mV * 10

/** @brief Request a change of state in the balancing status */
typedef struct {
    bool status;
    voltage_t threshold;
    bool is_new;
} BalRequest;


/** @brief Initialize balancing */
void bal_init(void);

/**
 * @brief Request a change to the current balancing status
 * 
 * @param status The new status to change into
 * @param threshold The minimum allowed voltage difference between all cells (in mV * 10)
 */
void bal_change_status_request(bool status, voltage_t threshold);

/**
 * @brief Get balancing threshold voltage value
 * 
 * @return voltage_t The threshold voltage
 */
voltage_t bal_get_threshold(void);

/**
 * @brief Check if the cellboards are balancing or not
 * 
 * @return true If any cellboards is balancing
 * @return false If no cellboards are balancing
 */
bool bal_is_balancing(void);

/**
 * @brief Workaround for 'can_bms_send'
 * 
 * @return BalRequest The request data
 */
BalRequest bal_get_request(void);

/**
 * @brief Update the balancing status of a cellboard
 * 
 * @param cellboard The cellboard ID 
 * @param status The balancing status of the cellboard
 */
void bal_update_status(uint8_t cellboard, bool status);

/** @brief Stop balancing */
void bal_stop(void);

/**
 * @brief Routine that keep the current balancing status updated
 * @details This function handles the start and stop mechanism of the balancing
 */
void bal_routine(void);

#endif  // BAL_H