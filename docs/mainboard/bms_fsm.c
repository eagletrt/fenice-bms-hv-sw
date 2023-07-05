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
  8 transition functions
******************************************************************************/

#include <syslog.h>
#include "bms_fsm.h"

// SEARCH FOR Your Code Here FOR CODE INSERTION POINTS!

// GLOBALS
// State human-readable names
const char *state_names[] = {"init", "idle", "fatal_error", "wait_airn_close", "wait_ts_precharge", "wait_airp_close", "ts_on"};

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
  
  syslog(LOG_INFO, "[FSM] In state init");
  /* Your Code Here */
  
  switch (next_state) {
    case STATE_IDLE:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from init to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state idle
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_WAIT_AIRN_CLOSE
state_t do_idle(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state idle");
  /* Your Code Here */
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_WAIT_AIRN_CLOSE:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from idle to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state fatal_error
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR
state_t do_fatal_error(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state fatal_error");
  /* Your Code Here */
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from fatal_error to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state wait_airn_close
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_WAIT_AIRN_CLOSE, STATE_WAIT_TS_PRECHARGE
state_t do_wait_airn_close(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state wait_airn_close");
  /* Your Code Here */
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_WAIT_AIRN_CLOSE:
    case STATE_WAIT_TS_PRECHARGE:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from wait_airn_close to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state wait_ts_precharge
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_WAIT_TS_PRECHARGE, STATE_WAIT_AIRP_CLOSE
state_t do_wait_ts_precharge(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state wait_ts_precharge");
  /* Your Code Here */
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_WAIT_TS_PRECHARGE:
    case STATE_WAIT_AIRP_CLOSE:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from wait_ts_precharge to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state wait_airp_close
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_WAIT_AIRP_CLOSE, STATE_TS_ON
state_t do_wait_airp_close(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state wait_airp_close");
  /* Your Code Here */
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_WAIT_AIRP_CLOSE:
    case STATE_TS_ON:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from wait_airp_close to %s, remaining in this state", state_names[next_state]);
      next_state = NO_CHANGE;
  }
  
  return next_state;
}


// Function to be executed in state ts_on
// valid return states: NO_CHANGE, STATE_IDLE, STATE_FATAL_ERROR, STATE_TS_ON
state_t do_ts_on(state_data_t *data) {
  state_t next_state = NO_CHANGE;
  
  syslog(LOG_INFO, "[FSM] In state ts_on");
  /* Your Code Here */
  
  switch (next_state) {
    case NO_CHANGE:
    case STATE_IDLE:
    case STATE_FATAL_ERROR:
    case STATE_TS_ON:
      break;
    default:
      syslog(LOG_WARNING, "[FSM] Cannot pass from ts_on to %s, remaining in this state", state_names[next_state]);
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
  syslog(LOG_INFO, "[FSM] State transition init_to_idle");
  /* Your Code Here */
}

// This function is called in 5 transitions:
// 1. from idle to fatal_error
// 2. from wait_airn_close to fatal_error
// 3. from wait_ts_precharge to fatal_error
// 4. from wait_airp_close to fatal_error
// 5. from ts_on to fatal_error
void set_fatal_error(state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition set_fatal_error");
  /* Your Code Here */
}

// This function is called in 1 transition:
// 1. from idle to wait_airn_close
void close_airn(state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition close_airn");
  /* Your Code Here */
}

// This function is called in 1 transition:
// 1. from fatal_error to idle
void fatal_error_to_idle(state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition fatal_error_to_idle");
  /* Your Code Here */
}

// This function is called in 4 transitions:
// 1. from wait_airn_close to idle
// 2. from wait_ts_precharge to idle
// 3. from wait_airp_close to idle
// 4. from ts_on to idle
void set_ts_off(state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition set_ts_off");
  /* Your Code Here */
}

// This function is called in 1 transition:
// 1. from wait_airn_close to wait_ts_precharge
void start_precharge(state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition start_precharge");
  /* Your Code Here */
}

// This function is called in 1 transition:
// 1. from wait_ts_precharge to wait_airp_close
void close_airp(state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition close_airp");
  /* Your Code Here */
}

// This function is called in 1 transition:
// 1. from wait_airp_close to ts_on
void set_ts_on(state_data_t *data) {
  syslog(LOG_INFO, "[FSM] State transition set_ts_on");
  /* Your Code Here */
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
  state_t new_state = state_table[cur_state](data);
  if (new_state == NO_CHANGE) new_state = cur_state;
  transition_func_t *transition = transition_table[cur_state][new_state];
  if (transition)
    transition(data);
  return new_state;
};

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
