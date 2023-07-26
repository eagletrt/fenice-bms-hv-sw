/******************************************************************************
Finite State Machine
Project: bal_fsm.dot
Description: bal_fsm

Generated by gv_fsm ruby gem, see https://rubygems.org/gems/gv_fsm
gv_fsm version 0.3.4
Generation date: 2023-07-19 21:28:20 +0200
Generated from: bal_fsm.dot
The finite state machine has:
  4 states
  5 transition functions
******************************************************************************/

#include "bal_fsm.h"

#include <string.h>

#include "usart.h"
#include "spi.h"
#include "ltc6813.h"
#include "bal.h"
#include "cellboard_config.h"
#include "volt.h"

// SEARCH FOR Your Code Here FOR CODE INSERTION POINTS!

// GLOBALS
// State human-readable names
const char * state_names[] = {"init", "off", "discharge", "cooldown"};
char debug_msg[100] = { 0 };

bal_state_t fsm_state = STATE_INIT;
// Set balancing parameters to default values
bal_fsm_params bal_params = {
    .is_s_pin_high = false,
    .discharge_cells = 0,
    .cycle_length = DCTO_30S,
    .target = CELL_MAX_VOLTAGE,
    .threshold = BAL_MAX_VOLTAGE_THRESHOLD
};
bal_fsm_transition_request set_bal_request = {
    .is_new = false,
    .next_state = STATE_OFF
};

bool discharge_timeout = false;
bool cooldown_timeout = false;

// List of state functions
state_func_t * const state_table[NUM_STATES] = {
  do_init,      // in state init
  do_off,       // in state off
  do_discharge, // in state discharge
  do_cooldown,  // in state cooldown
};

// Table of transition functions
transition_func_t * const transition_table[NUM_STATES][NUM_STATES] = {
  /* states:       init                 , off                  , discharge            , cooldown              */
  /* init      */ {NULL                 , init_to_off          , NULL                 , NULL                 }, 
  /* off       */ {NULL                 , NULL                 , start_discharge      , NULL                 }, 
  /* discharge */ {NULL                 , stop_balancing       , NULL                 , start_cooldown       }, 
  /* cooldown  */ {NULL                 , stop_balancing       , cooldown_to_discharge, NULL                 }, 
};

/**
 * @brief Check if a request to start balancing was sent
 * 
 * @return true If a balancing request was sent
 * @return false Otherwise
 */
bool _requested_bal_on() {
    return set_bal_request.is_new && set_bal_request.next_state == STATE_DISCHARGE;
}
/**
 * @brief Check if a request to stop balancing was sent
 * 
 * @return true If a balancing request was sent
 * @return false Otherwise
 */
bool _requested_bal_off() {
    return set_bal_request.is_new && set_bal_request.next_state == STATE_OFF;
}

bool bal_is_cells_empty() {
    return bal_params.discharge_cells == 0;
}

/*  ____  _        _       
 * / ___|| |_ __ _| |_ ___ 
 * \___ \| __/ _` | __/ _ \
 *  ___) | || (_| | ||  __/
 * |____/ \__\__,_|\__\___|
 *                         
 *   __                  _   _                 
 *  / _|_   _ _ __   ___| |_(_) ___  _ __  ___ 
 * | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 * |  _| |_| | | | | (__| |_| | (_) | | | \__ \
 * |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 */                                             

// Function to be executed in state init
// valid return states: STATE_OFF
bal_state_t do_init(state_data_t *data) {
  bal_state_t next_state = STATE_OFF;
  
  /* Your Code Here */
  
  switch (next_state) {
    case STATE_OFF:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from init to %s, remaining in this state\r\n", state_names[next_state]);
      HAL_UART_Transmit(&CLI_UART, (uint8_t *)debug_msg, strlen(debug_msg), 100);
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state off
// valid return states: NO_CHANGE, STATE_OFF, STATE_DISCHARGE
bal_state_t do_off(state_data_t *data) {
  bal_state_t next_state = NO_CHANGE;
  
  /* Your Code Here */

  if (_requested_bal_on())
    next_state = STATE_DISCHARGE;
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_OFF:
    case STATE_DISCHARGE:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from init to %s, remaining in this state\r\n", state_names[next_state]);
      HAL_UART_Transmit(&CLI_UART, (uint8_t *)debug_msg, strlen(debug_msg), 100);
      next_state = NO_CHANGE;
  }
  
  return next_state;    
}


// Function to be executed in state discharge
// valid return states: NO_CHANGE, STATE_OFF, STATE_DISCHARGE, STATE_COOLDOWN
bal_state_t do_discharge(state_data_t *data) {
  bal_state_t next_state = NO_CHANGE;
  
  /* Your Code Here */
  // Get cells to discharge
  bal_get_cells_to_discharge(
    volt_get_volts(),
    &bal_params.discharge_cells,
    bal_params.target,
    bal_params.threshold
  );

  if (_requested_bal_off() || bal_is_cells_empty())
    next_state = STATE_OFF;
  else if (discharge_timeout)
    next_state = STATE_COOLDOWN;
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_OFF:
    case STATE_DISCHARGE:
    case STATE_COOLDOWN:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from init to %s, remaining in this state\r\n", state_names[next_state]);
      HAL_UART_Transmit(&CLI_UART, (uint8_t *)debug_msg, strlen(debug_msg), 100);
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state cooldown
// valid return states: NO_CHANGE, STATE_OFF, STATE_DISCHARGE, STATE_COOLDOWN
bal_state_t do_cooldown(state_data_t *data) {
  bal_state_t next_state = NO_CHANGE;
  
  /* Your Code Here */
  // Get cells to discharge
  bal_get_cells_to_discharge(
    volt_get_volts(),
    &bal_params.discharge_cells,
    bal_params.target,
    bal_params.threshold
  );

  if (_requested_bal_off() || bal_is_cells_empty())
    next_state = STATE_OFF;
  else if (cooldown_timeout)
    next_state = STATE_DISCHARGE;
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_OFF:
    case STATE_DISCHARGE:
    case STATE_COOLDOWN:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from init to %s, remaining in this state\r\n", state_names[next_state]);
      HAL_UART_Transmit(&CLI_UART, (uint8_t *)debug_msg, strlen(debug_msg), 100);
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


/*  _____                    _ _   _              
 * |_   _| __ __ _ _ __  ___(_) |_(_) ___  _ __   
 *   | || '__/ _` | '_ \/ __| | __| |/ _ \| '_ \
 *   | || | | (_| | | | \__ \ | |_| | (_) | | | | 
 *   |_||_|  \__,_|_| |_|___/_|\__|_|\___/|_| |_| 
 *                                                
 *   __                  _   _                 
 *  / _|_   _ _ __   ___| |_(_) ___  _ __  ___ 
 * | |_| | | | '_ \ / __| __| |/ _ \| '_ \/ __|
 * |  _| |_| | | | | (__| |_| | (_) | | | \__ \
 * |_|  \__,_|_| |_|\___|\__|_|\___/|_| |_|___/
 */    
                                         
// This function is called in 1 transition:
// 1. from init to off
void init_to_off(state_data_t * data) {
  HAL_UART_Transmit(&CLI_UART, (uint8_t *)"[FSM] State transition init_to_off\r\n", 36, 100);
  /* Your Code Here */

  // Set timers autoreload
  __HAL_TIM_SET_AUTORELOAD(&HTIM_DISCHARGE, TIM_MS_TO_TICKS(&HTIM_COOLDOWN, BAL_CYCLE_LENGTH));
  __HAL_TIM_SET_AUTORELOAD(&HTIM_COOLDOWN, TIM_MS_TO_TICKS(&HTIM_COOLDOWN, BAL_COOLDOWN_DELAY));

  // Reset balancing (just in case)
  bal_params.discharge_cells = 0;
  ltc6813_set_balancing(&LTC6813_SPI, bal_params.discharge_cells, DCTO_DISABLED);
}

// This function is called in 1 transition:
// 1. from off to discharge
void start_discharge(state_data_t * data) {
  HAL_UART_Transmit(&CLI_UART, (uint8_t *)"[FSM] State transition start_discharge\r\n", 40, 100);
  /* Your Code Here */

  // Reset the discharge timer
  __HAL_TIM_SetCounter(&HTIM_DISCHARGE, 0U);
  __HAL_TIM_SetCompare(&HTIM_DISCHARGE, TIM_CHANNEL_1, TIM_MS_TO_TICKS(&HTIM_DISCHARGE, BAL_TIME_ON));
  __HAL_TIM_CLEAR_IT(&HTIM_DISCHARGE, TIM_IT_UPDATE);

  // Start discharge timer
  HAL_TIM_Base_Start_IT(&HTIM_DISCHARGE);
  HAL_TIM_OC_Start_IT(&HTIM_DISCHARGE, TIM_CHANNEL_1);
  
  // Start balancing
  bal_params.is_s_pin_high = true;
  ltc6813_set_balancing(&LTC6813_SPI, bal_params.discharge_cells, bal_params.cycle_length);

  discharge_timeout = false;
}

// This function is called in 2 transitions:
// 1. from discharge to off
// 2. from cooldown to off
void stop_balancing(state_data_t * data) {
  HAL_UART_Transmit(&CLI_UART, (uint8_t *)"[FSM] State transition stop_balancing\r\n", 39, 100);
  /* Your Code Here */

  // Stop discharge timer
  HAL_TIM_Base_Stop_IT(&HTIM_DISCHARGE);
  HAL_TIM_OC_Stop_IT(&HTIM_DISCHARGE, TIM_CHANNEL_1);
  __HAL_TIM_CLEAR_IT(&HTIM_DISCHARGE, TIM_IT_UPDATE);
  __HAL_TIM_CLEAR_FLAG(&HTIM_DISCHARGE, TIM_IT_CC1);  

  // Stop cooldown timer
  HAL_TIM_Base_Stop_IT(&HTIM_COOLDOWN);
  __HAL_TIM_CLEAR_IT(&HTIM_COOLDOWN, TIM_IT_UPDATE);

  // Reset balancing
  bal_params.is_s_pin_high = false;
  bal_params.discharge_cells = 0;
  ltc6813_set_balancing(&LTC6813_SPI, bal_params.discharge_cells, DCTO_DISABLED);

  // Reset timeouts
  discharge_timeout = false;
  cooldown_timeout = false;
}

// This function is called in 1 transition:
// 1. from discharge to cooldown
void start_cooldown(state_data_t *data) {
  HAL_UART_Transmit(&CLI_UART, (uint8_t *)"[FSM] State transition start_cooldown\r\n", 39, 100);
  /* Your Code Here */

  // Stop discharge timer
  HAL_TIM_Base_Stop_IT(&HTIM_DISCHARGE);
  HAL_TIM_OC_Stop_IT(&HTIM_DISCHARGE, TIM_CHANNEL_1);
  __HAL_TIM_CLEAR_IT(&HTIM_DISCHARGE, TIM_IT_UPDATE);
  __HAL_TIM_CLEAR_FLAG(&HTIM_DISCHARGE, TIM_IT_CC1);

  // Start cooldown timer
  HAL_TIM_Base_Start_IT(&HTIM_COOLDOWN);

  // Reset balancing
  bal_params.is_s_pin_high = false;
  ltc6813_set_balancing(&LTC6813_SPI, 0, DCTO_DISABLED);

  // Reset cooldown timeout
  cooldown_timeout = false;
}

// This function is called in 1 transition:
// 1. from cooldown to discharge
void cooldown_to_discharge(state_data_t *data) {
  HAL_UART_Transmit(&CLI_UART, (uint8_t *)"[FSM] State transition cooldown_to_discharge\r\n", 46, 100);
  /* Your Code Here */

  // Stop cooldown timer
  HAL_TIM_Base_Stop_IT(&HTIM_COOLDOWN);
  __HAL_TIM_CLEAR_IT(&HTIM_COOLDOWN, TIM_IT_UPDATE);

  // Reset the discharge timer
  __HAL_TIM_SetCounter(&HTIM_DISCHARGE, 0U);
  __HAL_TIM_SetCompare(&HTIM_DISCHARGE, TIM_CHANNEL_1, TIM_MS_TO_TICKS(&HTIM_DISCHARGE, BAL_TIME_ON));
  __HAL_TIM_CLEAR_IT(&HTIM_DISCHARGE, TIM_IT_UPDATE);

  // Start discharge timer
  HAL_TIM_Base_Start_IT(&HTIM_DISCHARGE);
  HAL_TIM_OC_Start_IT(&HTIM_DISCHARGE, TIM_CHANNEL_1);

  // Restart balancing
  bal_params.is_s_pin_high = true;
  ltc6813_set_balancing(&LTC6813_SPI, bal_params.discharge_cells, bal_params.cycle_length);

  // Reset discharge time
  discharge_timeout = false;
}


/*  ____  _        _        
 * / ___|| |_ __ _| |_ ___  
 * \___ \| __/ _` | __/ _ \
 *  ___) | || (_| | ||  __/ 
 * |____/ \__\__,_|\__\___| 
 *                          
 *                                              
 *  _ __ ___   __ _ _ __   __ _  __ _  ___ _ __ 
 * | '_ ` _ \ / _` | '_ \ / _` |/ _` |/ _ \ '__|
 * | | | | | | (_| | | | | (_| | (_| |  __/ |   
 * |_| |_| |_|\__,_|_| |_|\__,_|\__, |\___|_|   
 *                              |___/           
 */

bal_state_t run_state(bal_state_t cur_state, state_data_t * data) {
  bool received_request = set_bal_request.is_new;
  bal_state_t new_state = state_table[cur_state](data);
  if (received_request)
    set_bal_request.is_new = false;
  if (new_state == NO_CHANGE) new_state = cur_state;
  transition_func_t * transition = transition_table[cur_state][new_state];

  if (transition) {
    // Send info via CAN
    can_send(BMS_BOARD_STATUS_FRAME_ID);

    transition(data);
  }
  return new_state;
};

void fsm_run() {
    // Run the FSM and updates
    fsm_state = run_state(fsm_state, NULL);
}
bal_state_t fsm_get_state() {
    return fsm_state;
}

void bal_oc_timer_handler(TIM_HandleTypeDef * htim) {
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
        uint32_t cmp = __HAL_TIM_GetCompare(htim, TIM_CHANNEL_1);
        
        if (bal_params.is_s_pin_high) {
            ltc6813_set_balancing(&LTC6813_SPI, 0, DCTO_DISABLED);
            __HAL_TIM_SET_COMPARE(&HTIM_DISCHARGE, TIM_CHANNEL_1, cmp + TIM_MS_TO_TICKS(htim, BAL_TIME_OFF));
        }
        else {
            ltc6813_set_balancing(&LTC6813_SPI, bal_params.discharge_cells, bal_params.cycle_length);
            __HAL_TIM_SET_COMPARE(&HTIM_DISCHARGE, TIM_CHANNEL_1, cmp + TIM_MS_TO_TICKS(htim, BAL_TIME_ON));
        }
        bal_params.is_s_pin_high = !bal_params.is_s_pin_high;
    }
}
// TODO: Handle discharge timeouts greater than 30s
void bal_timers_handler(TIM_HandleTypeDef * htim) {
    if (htim->Instance == HTIM_DISCHARGE.Instance)
        discharge_timeout = true;
    else if (htim->Instance == HTIM_COOLDOWN.Instance)
        cooldown_timeout = true;
}