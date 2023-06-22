/**
 * @file bal_fsm.h
 * @brief This file contains the balancing functions
 *
 * @date Jul 07, 2021
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef BAL_FSM_H
#define BAL_FSM_H

#include <stdbool.h>
#include <inttypes.h>

#include "fsm.h"
#include "bms/bms_network.h"
#include "cellboard_config.h"
#include "stm32l4xx_hal.h"

enum { BAL_OFF = 0, BAL_COMPUTE, BAL_DISCHARGE, BAL_COOLDOWN, BAL_NUM_STATES };
enum { EV_BAL_STOP = 0, EV_BAL_START, EV_BAL_COOLDOWN_START, EV_BAL_COOLDOWN_END, BAL_EV_NUM };

// TODO: Set threshold (get from mainboard via CAN)
typedef struct {
    fsm fsm;
    uint8_t is_s_pin_high;
    uint32_t cells;
    bms_board_status_balancing_status status;
    uint32_t discharge_time;
    uint32_t cycle_length;
    voltage_t target;
    uint16_t threshold;
} bal_fsm;

extern bal_fsm bal;

/**
 * @brief Get the voltage threshold value for balancing
 * 
 * @return uint16_t The voltage threshold
 */
uint16_t bal_fsm_get_threshold();
/**
 * @brief Set the voltage threshold value for balancing
 * 
 * @param threshold The new threshold voltage to set
 */
void bal_fsm_set_threshold(uint16_t threshold);

/** @brief Initialize the fsm */
void bal_fsm_init();

/**
 * @brief Reset balancing fsm status
 * @details This function should get called when the discharge timer elapses
 * 
 * @param htim The timer instance
 * @param FSM The balancing fsm
 */
void bal_timers_handler(TIM_HandleTypeDef * htim, fsm FSM);
/**
 * @brief Set balancing configuration
 * 
 * @param htim The timer instance
 */
void bal_oc_timer_handler(TIM_HandleTypeDef *htim);

/**
 * @brief Check if there are no cells to balance
 * 
 * @return bool False if there are cells to balance, true otherwise
 */
bool bal_is_cells_empty();

#endif