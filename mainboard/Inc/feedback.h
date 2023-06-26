/**
 * @file	feedback.h
 * @brief	Feedback parsing utilities
 *
 * @date	Mar 16, 2021
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author  	Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <inttypes.h>

#include <../../fenice_config.h>


#define MUX_INTERVAL_MS 0.1

#define FB_CHECK_INTERVAL_MS 2
#define FB_TIMEOUT_MS        100

typedef uint32_t feedback_t;

extern uint64_t feedback;

// TODO: Change feedback macros

#define FEEDBACK_TS_OFF_HIGH                                                                                     \
    (FEEDBACK_CHECK_MUX /* | FEEDBACK_TSAL_GREEN_FAULT_LATCHED*/ | FEEDBACK_BMS_COCKPIT | FEEDBACK_IMD_COCKPIT | \
     FEEDBACK_IMD_FAULT /* | FEEDBACK_TSAL_GREEN*/ | FEEDBACK_IMPLAUSIBILITY_DETECTED | FEEDBACK_AIRN_STATUS |          \
     FEEDBACK_AIRP_STATUS | FEEDBACK_SD_IN | FEEDBACK_SD_OUT)
#define FEEDBACK_TS_OFF_LOW (FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE | FEEDBACK_RELAY_SD)

#define FEEDBACK_TS_OFF_VAL  (FEEDBACK_TS_OFF_HIGH)
#define FEEDBACK_TS_OFF_MASK ((FEEDBACK_TS_OFF_HIGH) | (FEEDBACK_TS_OFF_LOW))

#define FEEDBACK_AIRN_CLOSE_HIGH                                                                                 \
    (FEEDBACK_CHECK_MUX /* | FEEDBACK_TSAL_GREEN_FAULT_LATCHED*/ | FEEDBACK_BMS_COCKPIT | FEEDBACK_IMD_COCKPIT | \
     FEEDBACK_IMD_FAULT /* | FEEDBACK_TSAL_GREEN*/ | FEEDBACK_IMPLAUSIBILITY_DETECTED | FEEDBACK_AIRN_STATUS |          \
     FEEDBACK_AIRP_STATUS | FEEDBACK_SD_IN | FEEDBACK_SD_OUT | FEEDBACK_SD_END)
#define FEEDBACK_AIRN_CLOSE_LOW (FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE | FEEDBACK_RELAY_SD)

#define FEEDBACK_AIRN_CLOSE_VAL  (FEEDBACK_AIRN_CLOSE_HIGH)
#define FEEDBACK_AIRN_CLOSE_MASK ((FEEDBACK_AIRN_CLOSE_HIGH) | (FEEDBACK_AIRN_CLOSE_LOW))

#define FEEDBACK_AIRN_STATUS_HIGH                                                                                  \
    (FEEDBACK_CHECK_MUX /* | FEEDBACK_TSAL_GREEN_FAULT_LATCHED*/ | FEEDBACK_BMS_COCKPIT | FEEDBACK_IMD_COCKPIT |   \
     FEEDBACK_IMD_FAULT | FEEDBACK_IMPLAUSIBILITY_DETECTED | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_STATUS | FEEDBACK_SD_IN | \
     FEEDBACK_SD_OUT | FEEDBACK_SD_END)
#define FEEDBACK_AIRN_STATUS_LOW \
    (FEEDBACK_AIRN_STATUS | FEEDBACK_AIRP_GATE | FEEDBACK_RELAY_SD /* | FEEDBACK_TSAL_GREEN*/)

#define FEEDBACK_AIRN_STATUS_VAL  (FEEDBACK_AIRN_STATUS_HIGH)
#define FEEDBACK_AIRN_STATUS_MASK ((FEEDBACK_AIRN_STATUS_HIGH) | (FEEDBACK_AIRN_STATUS_LOW))

#define FEEDBACK_PC_ON_HIGH                                                                                        \
    (FEEDBACK_CHECK_MUX /* | FEEDBACK_TSAL_GREEN_FAULT_LATCHED*/ | FEEDBACK_BMS_COCKPIT | FEEDBACK_IMD_COCKPIT |   \
     FEEDBACK_IMD_FAULT | FEEDBACK_IMPLAUSIBILITY_DETECTED | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_STATUS | FEEDBACK_SD_IN | \
     FEEDBACK_SD_OUT | FEEDBACK_SD_END)
#define FEEDBACK_PC_ON_LOW                                                   \
    (FEEDBACK_AIRN_STATUS | FEEDBACK_AIRP_GATE | FEEDBACK_PRECHARGE_STATUS | \
     FEEDBACK_RELAY_SD /* | FEEDBACK_TSAL_GREEN*/)

#define FEEDBACK_PC_ON_VAL  (FEEDBACK_PC_ON_HIGH)
#define FEEDBACK_PC_ON_MASK ((FEEDBACK_PC_ON_HIGH) | (FEEDBACK_PC_ON_LOW))

#define FEEDBACK_ON_HIGH (FEEDBACK_CHECK_MUX | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE)
#define FEEDBACK_ON_LOW                                                          \
    (FEEDBACK_AIRN_STATUS | FEEDBACK_AIRP_STATUS | FEEDBACK_TS_OVER_60V_STATUS | \
     FEEDBACK_TSP_OVER_60V_STATUS /* | FEEDBACK_TSAL_GREEN*/)

#define FEEDBACK_ON_VAL  (FEEDBACK_ON_HIGH)
#define FEEDBACK_ON_MASK ((FEEDBACK_ON_HIGH) | (FEEDBACK_ON_LOW))

#define FEEDBACK_FAULT_EXIT_HIGH (FEEDBACK_CHECK_MUX | FEEDBACK_AIRN_STATUS | FEEDBACK_AIRP_STATUS)
#define FEEDBACK_FAULT_EXIT_LOW  (FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE)

#define FEEDBACK_FAULT_EXIT_VAL  (FEEDBACK_FAULT_EXIT_HIGH)
#define FEEDBACK_FAULT_EXIT_MASK ((FEEDBACK_FAULT_EXIT_HIGH) | (FEEDBACK_FAULT_EXIT_LOW))

/*
// Feedbacks to check after TS OFF
#define FEEDBACK_TS_OFF_VAL \
        FEEDBACK_CHECK_MUX| FEEDBACK_TSAL_GREEN | FEEDBACK_AIRN_STATUS

#define FEEDBACK_TS_OFF_MASK 
        FEEDBACK_TS_OFF_VAL | FEEDBACK_PC_END | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE | FEEDBACK_AIRN_STATUS



#define FEEDBACK_TS_ON_VAL \
        FEEDBACK_CHECK_MUX | FEEDBACK_IMD_FAULT

#define FEEDBACK_TS_ON_MASK \
        FEEDBACK_TS_ON_VAL | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE



#define FEEDBACK_AIRN_CLOSE_VAL \
        FEEDBACK_CHECK_MUX

#define FEEDBACK_AIRN_CLOSE_MASK \
        FEEDBACK_AIRN_CLOSE_VAL | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE



#define FEEDBACK_PC_ON_VAL \
        FEEDBACK_CHECK_MUX | FEEDBACK_AIRN_GATE \
        | FEEDBACK_IMD_FAULT

#define FEEDBACK_PC_ON_MASK \
        FEEDBACK_PC_ON_VAL | FEEDBACK_AIRP_GATE


#define FEEDBACK_ON_VAL \
        FEEDBACK_CHECK_MUX | FEEDBACK_AIRP_GATE | FEEDBACK_AIRN_GATE
        
#define FEEDBACK_ON_MASK FEEDBACK_ALL
*/

#define FEEDBACK_CONVERT_ADC_TO_VOLTAGE(VALUE) ((VALUE) * (3.3F / 4095))

/** @brief State of the feedbacks */
typedef enum {
    FEEDBACK_STATE_H,
    FEEDBACK_STATE_L,
    FEEDBACK_STATE_ERROR
} FEEDBACK_STATE;

/** @brief Information about feedbacks */
typedef struct {
    FEEDBACK_STATE state;
    float voltage;
} feedback_feed_t;


/** @brief Initialize the feedbacks */
void feedback_init();
/**
 * @brief Get the state of the feedbacks
 * 
 * @param out_value A pointer to a structure where the result is stored
 */
void feedback_get_feedback_states(feedback_feed_t out_value[FEEDBACK_N]);
/**
 * @brief Check if selected feedbacks values are valid
 * 
 * @param fb_check_mask The bit mask used to select the feedbacks
 * @param fb_value The feedback 
 * @return feedback_t An integer where each bit set to 1 represent a feedback which voltage is invalid
 */
feedback_t feedback_check(feedback_t fb_check_mask, feedback_t fb_value);

/** @brief Set next multiplexer index */
void feedback_set_next_mux_index();

/** @brief Feedback timer callback handler */
void _feedback_handle_tim_elapsed_irq();
/** @brief Feedback ADC callback handler */
void _feedback_handle_adc_cnv_cmpl_irq();

#endif // FEEDBACK_H