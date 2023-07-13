/******************************************************************************
Finite State Machine
Project: bms_fsm.dot
Description: bms_hv_fsm

Generated by gv_fsm ruby gem, see https://rubygems.org/gems/gv_fsm
gv_fsm version 0.3.4
Generation date: 2023-07-05 07:57:19 +0200
Generated from: bms_fsm.dot
The finite state machine has:
  7 states
  9 transition functions
******************************************************************************/

#include "bms_fsm.h"

#include <string.h>

#include "stm32f4xx_hal.h"
#include "tim.h"
#include "mainboard_config.h"
#include "current.h"
#include "config.h"
#include "pack/pack.h"
#include "feedback.h"
#include "can_comm.h"
#include "cli_bms.h"
#include "blink.h"
#include "internal_voltage.h"
#include "bal.h"

#define AIRN_TIMEOUT_CHANNEL TIM_CHANNEL_1
#define PRECHARGE_TIMEOUT_CHANNEL TIM_CHANNEL_2
#define AIRP_TIMEOUT_CHANNEL TIM_CHANNEL_3

#define AIRN_TIMEOUT_INTERRUPT TIM_IT_CC1
#define PRECHARGE_TIMEOUT_INTERRUPT TIM_IT_CC2
#define AIRP_TIMEOUT_INTERRUPT TIM_IT_CC3

// SEARCH FOR Your Code Here FOR CODE INSERTION POINTS!

// GLOBALS
// State human-readable names
const char *state_names[] = {"init", "idle", "fatal_error", "wait_airn_close", "wait_ts_precharge", "wait_airp_close", "ts_on"};
char debug_msg[100] = { 0 };

state_t fsm_state = STATE_INIT;
blink_t led;
bool airn_timeout = false;
bool precharge_timeout = false;
bool airp_timeout = false;

fsm_transition_request set_ts_request = {
    .is_new = false,
    .next_state = STATE_IDLE
};


// List of state functions
state_func_t *const state_table[NUM_STATES] = {
  do_init,              // in state init
  do_idle,              // in state idle
  do_fatal_error,       // in state fatal_error
  do_wait_airn_close,   // in state wait_airn_close
  do_wait_ts_precharge, // in state wait_ts_precharge
  do_wait_airp_close,   // in state wait_airp_close
  do_ts_on,             // in state ts_on
};

// Table of transition functions
transition_func_t *const transition_table[NUM_STATES][NUM_STATES] = {
  /* states:               init               , idle               , fatal_error        , wait_airn_close    , wait_ts_precharge  , wait_airp_close    , ts_on               */
  /* init              */ {NULL               , init_to_idle       , NULL               , NULL               , NULL               , NULL               , NULL               }, 
  /* idle              */ {NULL               , NULL               , set_fatal_error    , close_airn         , NULL               , NULL               , NULL               }, 
  /* fatal_error       */ {NULL               , fatal_error_to_idle, NULL               , NULL               , NULL               , NULL               , NULL               }, 
  /* wait_airn_close   */ {NULL               , set_ts_off         , set_fatal_error    , NULL               , start_precharge    , NULL               , NULL               }, 
  /* wait_ts_precharge */ {NULL               , set_ts_off         , set_fatal_error    , NULL               , NULL               , close_airp         , NULL               }, 
  /* wait_airp_close   */ {NULL               , set_ts_off         , set_fatal_error    , NULL               , NULL               , NULL               , set_ts_on          }, 
  /* ts_on             */ {NULL               , set_ts_off         , set_fatal_error    , NULL               , NULL               , NULL               , NULL               }, 
};

/**
 * @brief Stop the timeout timer
 * 
 * @param channel The channel of the timer to stop
 * @param interrupt The interrupt pending bit to clear
 */
void _stop_timeout(size_t channel, size_t interrupt) {
    HAL_TIM_OC_Stop_IT(&HTIM_BMS, channel);
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, interrupt);
}
/** @brief Reset all timeouts */
void _reset_timeouts() {
    _stop_timeout(AIRN_TIMEOUT_CHANNEL, AIRN_TIMEOUT_INTERRUPT);
    _stop_timeout(PRECHARGE_TIMEOUT_CHANNEL, PRECHARGE_TIMEOUT_INTERRUPT);
    _stop_timeout(AIRP_TIMEOUT_CHANNEL, AIRP_TIMEOUT_INTERRUPT);
    airn_timeout = false;
    precharge_timeout = false;
    airp_timeout = false;
}
/**
 * @brief Start the timeout timer
 * 
 * @param channel The channel of the timer to start
 * @param interrupt The interrupt pending bit to clear
 * @param timeout The timeout duration in ms
 */
void _start_timeout(size_t channel, size_t interrupt, size_t timeout) {
    // Set timer compare
    uint32_t cnt = __HAL_TIM_GET_COUNTER(&HTIM_BMS);
    __HAL_TIM_SET_COMPARE(&HTIM_BMS, channel, (cnt + TIM_MS_TO_TICKS(&HTIM_BMS, timeout)));
    // Clear existing interrupts
    __HAL_TIM_CLEAR_IT(&HTIM_BMS, interrupt);

    // Start timer
    HAL_TIM_OC_Start_IT(&HTIM_BMS, channel);
}

bool _requested_ts_on() {
    return set_ts_request.is_new && set_ts_request.next_state == STATE_WAIT_AIRN_CLOSE;
}
bool _requested_ts_off() {
    return set_ts_request.is_new && set_ts_request.next_state == STATE_IDLE;
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
// valid return states: STATE_IDLE
state_t do_init(state_data_t *data) {
  state_t next_state = STATE_IDLE;
  
  // cli_bms_debug("[FSM] In state init", 19);
  /* Your Code Here */
  
  switch (next_state) {
    case STATE_IDLE:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from init to %s, remaining in this state", state_names[next_state]);
      cli_bms_debug(debug_msg, strlen(debug_msg));
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state idle
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_WAIT_AIRN_CLOSE
state_t do_idle(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  // cli_bms_debug("[FSM] In state idle", 19);
  /* Your Code Here */

  // Check for fatal errors
  if (error_get_fatal() > 0)
    next_state = STATE_FATAL_ERROR;
  else if (_requested_ts_on() && feedback_is_ok(FEEDBACK_IDLE_MASK, FEEDBACK_IDLE_HIGH))
    next_state = STATE_WAIT_AIRN_CLOSE;

  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_WAIT_AIRN_CLOSE:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from idle to %s, remaining in this state", state_names[next_state]);
      cli_bms_debug(debug_msg, strlen(debug_msg));
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state fatal_error
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR
state_t do_fatal_error(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  // cli_bms_debug("[FSM] In state fatal_error", 26);
  /* Your Code Here */

  // Check errors and feedbacks
  if (error_get_fatal() == 0 && feedback_is_ok(FEEDBACK_FATAL_ERROR_MASK, FEEDBACK_FATAL_ERROR_HIGH))
    next_state = STATE_IDLE;
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from fatal_error to %s, remaining in this state", state_names[next_state]);
      cli_bms_debug(debug_msg, strlen(debug_msg));
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state wait_airn_close
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_WAIT_AIRN_CLOSE, STATE_WAIT_TS_PRECHARGE
state_t do_wait_airn_close(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  // cli_bms_debug("[FSM] In state wait_airn_close", 30);
  /* Your Code Here */

  // Check fatal errors
  if (error_get_fatal() > 0)
    next_state = STATE_FATAL_ERROR;
  else if (_requested_ts_off() || airn_timeout)
    next_state = STATE_IDLE;
  else if (feedback_is_ok(FEEDBACK_AIRN_CHECK_MASK, FEEDBACK_AIRN_CHECK_HIGH))
    next_state = STATE_WAIT_TS_PRECHARGE;

  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_WAIT_AIRN_CLOSE:
    case STATE_WAIT_TS_PRECHARGE:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from wait_airn_close to %s, remaining in this state", state_names[next_state]);
      cli_bms_debug(debug_msg, strlen(debug_msg));
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state wait_ts_precharge
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_WAIT_TS_PRECHARGE, STATE_WAIT_AIRP_CLOSE
state_t do_wait_ts_precharge(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  // cli_bms_debug("[FSM] In state wait_ts_precharge", 32);
  /* Your Code Here */

  // Check fatal errors
  if (error_get_fatal() > 0)
    next_state = STATE_FATAL_ERROR;
  else if (_requested_ts_off() || precharge_timeout) {
    if (precharge_timeout)
        cli_bms_debug("Precharge timeout", 17);
    if (_requested_ts_off())
        cli_bms_debug("Requested TS off", 16);
    next_state = STATE_IDLE;
  }
  else if (feedback_is_ok(FEEDBACK_PRECHARGE_CHECK_MASK, FEEDBACK_PRECHARGE_CHECK_HIGH) && internal_voltage_is_precharge_complete())
    next_state = STATE_WAIT_AIRP_CLOSE;
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_WAIT_TS_PRECHARGE:
    case STATE_WAIT_AIRP_CLOSE:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from wait_ts_precharge to %s, remaining in this state", state_names[next_state]);
      cli_bms_debug(debug_msg, strlen(debug_msg));
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state wait_airp_close
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_WAIT_AIRP_CLOSE, STATE_TS_ON
state_t do_wait_airp_close(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  // cli_bms_debug("[FSM] In state wait_airp_close", 30);
  /* Your Code Here */
  if (error_get_fatal() > 0)
    next_state = STATE_FATAL_ERROR;
  else if (_requested_ts_off() || airp_timeout) {
    if (airp_timeout)
        cli_bms_debug("AIR+ timeout", 12);
    next_state = STATE_IDLE;
  }
  else if (feedback_is_ok(FEEDBACK_AIRP_CHECK_MASK, FEEDBACK_AIRP_CHECK_HIGH))
    next_state = STATE_TS_ON;
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_WAIT_AIRP_CLOSE:
    case STATE_TS_ON:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from wait_airp_close to %s, remaining in this state", state_names[next_state]);
      cli_bms_debug(debug_msg, strlen(debug_msg));
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state ts_on
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_TS_ON
state_t do_ts_on(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  // cli_bms_debug("[FSM] In state ts_on", 20);
  /* Your Code Here */
  if (error_get_fatal() > 0) {
    error_t errors[30];
    error_dump(errors);
    cli_bms_debug("TS on errors", 12);
    next_state = STATE_FATAL_ERROR;
  }
  else if (_requested_ts_off() || !feedback_is_ok(FEEDBACK_TS_ON_CHECK_MASK, FEEDBACK_TS_ON_CHECK_HIGH))
    next_state = STATE_IDLE;

  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_TS_ON:
      break;
    default:
      sprintf(debug_msg, "[FSM] Cannot pass from ts_on to %s, remaining in this state", state_names[next_state]);
      cli_bms_debug(debug_msg, strlen(debug_msg));
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
// 1. from init to idle
void init_to_idle(state_data_t *data) {
  cli_bms_debug("[FSM] State transition init_to_idle", 35);
  /* Your Code Here */
  
  // Set blinking led pattern
  HAL_GPIO_WritePin(led.port, led.pin, GPIO_PIN_RESET);
  bms_set_led_blinker();

  // Send info via CAN
  can_car_send(PRIMARY_TS_STATUS_FRAME_ID);

  // Set default pack status
  pack_set_default_off(0);
  pack_set_fault(BMS_FAULT_OFF_VALUE);
  current_zero();
}

// This function is called in 4 transitions:
// 1. from idle to fatal_error
// 2. from wait_airn_close to fatal_error
// 3. from wait_ts_precharge to fatal_error
// 4. from wait_airp_close to fatal_error
void set_fatal_error(state_data_t *data) {
  cli_bms_debug("[FSM] State transition set_fatal_error", 38);
  /* Your Code Here */

  // Send info via CAN
  can_car_send(PRIMARY_HV_ERRORS_FRAME_ID);

  // Set fault status
  pack_set_fault(BMS_FAULT_ON_VALUE);
  pack_set_default_off(0);

  // Stop balancing if running
  if (bal_is_balancing())
    bal_stop();

  // Reset timeouts
  _reset_timeouts();

  // Set blinking led pattern
  bms_set_led_blinker();
}

// This function is called in 1 transition:
// 1. from idle to wait_airn_close
void close_airn(state_data_t *data) {
  cli_bms_debug("[FSM] State transition close_airn", 33);
  /* Your Code Here */

  // Set blinking led pattern
  bms_set_led_blinker();

  // Close AIR-
  pack_set_airn_off(AIRN_ON_VALUE);

  // Start AIR- timeout timer
  _start_timeout(AIRN_TIMEOUT_CHANNEL, AIRN_TIMEOUT_INTERRUPT, AIRN_CHECK_TIMEOUT);
}

// This function is called in 1 transition:
// 1. from fatal_error to idle
void fatal_error_to_idle(state_data_t *data) {
  cli_bms_debug("[FSM] State transition fatal_error_to_idle", 42);
  /* Your Code Here */
  
  // Set blinking led pattern
  bms_set_led_blinker();

  // Reset fault status
  pack_set_fault(BMS_FAULT_OFF_VALUE);
}

// This function is called in 3 transitions:
// 1. from wait_airn_close to idle
// 2. from wait_ts_precharge to idle
// 3. from wait_airp_close to idle
void set_ts_off(state_data_t *data) {
  cli_bms_debug("[FSM] State transition set_ts_off", 33);
  /* Your Code Here */

  // Set default pack status
  pack_set_default_off(0);

  // Reset timeouts
  _reset_timeouts();

  // Set blinking led pattern
  bms_set_led_blinker();
}

// This function is called in 1 transition:
// 1. from wait_airn_close to wait_ts_precharge
void start_precharge(state_data_t *data) {
  cli_bms_debug("[FSM] State transition start_precharge", 38);
  /* Your Code Here */
 
  // Stop AIR- timeout timer
  _stop_timeout(AIRN_TIMEOUT_CHANNEL, AIRN_TIMEOUT_INTERRUPT);

  // Set blinking led pattern
  bms_set_led_blinker();
  
  // Start precharge
  pack_set_precharge(PRECHARGE_ON_VALUE);
  
  // Start precharge timeout timer
  _start_timeout(PRECHARGE_TIMEOUT_CHANNEL, PRECHARGE_TIMEOUT_INTERRUPT, PRECHARGE_TIMEOUT);
}

// This function is called in 1 transition:
// 1. from wait_ts_precharge to wait_airp_close
void close_airp(state_data_t *data) {
  cli_bms_debug("[FSM] State transition close_airp", 33);
  /* Your Code Here */

  // Stop precharge timeout timer
  _stop_timeout(PRECHARGE_TIMEOUT_CHANNEL, PRECHARGE_TIMEOUT_INTERRUPT);

  // Set blinking led pattern
  bms_set_led_blinker();

  // Close AIR+
  pack_set_airp_off(AIRP_ON_VALUE);

  // Start AIR+ timeout timer
  _start_timeout(AIRP_TIMEOUT_CHANNEL, AIRP_TIMEOUT_INTERRUPT, AIRP_CHECK_TIMEOUT);
}

// This function is called in 1 transition:
// 1. from wait_airp_close to ts_on
void set_ts_on(state_data_t *data) {
  cli_bms_debug("[FSM] State transition set_ts_on", 32);
  /* Your Code Here */

  // Stop AIR+ timeout timer
  _stop_timeout(AIRP_TIMEOUT_CHANNEL, AIRP_TIMEOUT_INTERRUPT);

  // Set blinking led pattern
  bms_set_led_blinker();
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

state_t run_state(state_t cur_state, state_data_t *data) {
  bool received_request = set_ts_request.is_new;
  state_t new_state = state_table[cur_state](data);
  if (received_request)
    set_ts_request.is_new = false;
  if (new_state == NO_CHANGE) new_state = cur_state;
  transition_func_t *transition = transition_table[cur_state][new_state];
  
  if (transition) {
    // Send info via CAN
    can_car_send(PRIMARY_TS_STATUS_FRAME_ID);
    can_car_send(PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID);

    transition(data);
  }

  return new_state;
};

void fsm_run() {
    // Blink the led
    bms_blink_led();

    // Set or reset connection error
    error_toggle_check(HAL_GPIO_ReadPin(CONNS_DETECTION_GPIO_Port, CONNS_DETECTION_Pin) == GPIO_PIN_RESET, ERROR_CONNECTOR_DISCONNECTED, 0);

    // Run the FSM and updates
    fsm_state = run_state(fsm_state, NULL);
}
state_t fsm_get_state() {
    return fsm_state;
}
void bms_set_led_blinker() {
    uint8_t pattern_count = 0;
    uint8_t state_count = 0;
    uint32_t state = fsm_state;
    uint16_t pattern[(NUM_STATES) * 2 + 1] = { 0 };

    pattern[pattern_count++] = 200;  // Off
    while (state_count < state) {
        pattern[pattern_count++] = 200;  // On
        pattern[pattern_count++] = 200;  // Off
        state_count++;
    }
    pattern[pattern_count++] = 1000;  // Big off

    BLINK_SET_PATTERN(led, pattern, pattern_count);
    BLINK_SET_ENABLE(led, true);
    BLINK_SET_REPEAT(led, true);

    blink_reset(&led);
    HAL_GPIO_WritePin(led.port, led.pin, GPIO_PIN_SET);
}
void bms_blink_led() {
    blink_run(&led);
}


#ifdef TEST_MAIN
#include <unistd.h>
int main() {
  state_t cur_state = STATE_INIT;
  openlog("SM", LOG_PID | LOG_PERROR, LOG_USER);
  syslog(LOG_INFO, "Starting SM");
  do {
    cur_state = run_state(cur_state, NULL);
    sleep(1);
  } while (1);
  return 0;
}
#endif

void _bms_handle_tim_oc_irq(TIM_HandleTypeDef *htim) {
    switch (htim->Channel)
    {
        case HAL_TIM_ACTIVE_CHANNEL_1:  // AIR- timeout
            airn_timeout = true;
            break;
        case HAL_TIM_ACTIVE_CHANNEL_2:  // Precharge timeout 
            precharge_timeout = true;
            break;
        case HAL_TIM_ACTIVE_CHANNEL_3:  // AIR+ timeout
            airp_timeout = true;
            break;
        default:
            break;
    }
}