/**
 * @file		bal_fsm.h
 * @brief		This file contains the balancing functions
 *
 * @date		May 09, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef BAL_FSM_H
#define BAL_FSM_H

#include <inttypes.h>
#include <stdbool.h>

#include "../../fenice_config.h"

/**
 * @brief Get balancing threshold voltage value
 * 
 * @return voltage_t The threshold voltage
 */
voltage_t bal_get_threshold();
/**
 * @brief Set balancing threshold voltage value
 * 
 * @param thresh The new threshold voltage to set
 */
void bal_set_threshold(uint16_t thresh);
/**
 * @brief Check if the cellboards are balancing or not
 * 
 * @return true If any cellboards is balancing
 * @return false If no cellboards are balancing
 */
bool bal_is_balancing();
/**
 * @brief Set if the cellboards are balancing or not
 * 
 * @param is_bal True if any cellboard is balancing false
 */
void bal_set_is_balancing(uint8_t cellboard_id, bool is_bal);
/**
 * @brief Workaround for can_bms_send
 * 
 * @return true during bal_start
 * @return false otherwise
 */
bool bal_need_balancing();

/** @brief Initialize balancing */
void bal_init();
/** @brief Start balancing */
void bal_start();
/** @brief Stop balancing */
void bal_stop();

#endif