/**
 * @file feedback.h
 * @brief Feedback parsing utilities
 *
 * @date Mar 16, 2021
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#include <../../fenice_config.h>


#define MUX_INTERVAL_MS 0.1

#define FB_CHECK_INTERVAL_MS 2
#define FB_TIMEOUT_MS        100

typedef uint32_t feedback_t;

extern uint64_t feedback;

// TODO: Check carefully feeback masks

#define FEEDBACK_TS_OFF_HIGH \
    (FEEDBACK_CHECK_MUX | FEEDBACK_BMS_COCKPIT | FEEDBACK_IMD_COCKPIT | \
     FEEDBACK_IMD_FAULT | FEEDBACK_IMPLAUSIBILITY_DETECTED | FEEDBACK_AIRN_STATUS | \
     FEEDBACK_AIRP_STATUS | FEEDBACK_SD_IN | FEEDBACK_SD_OUT)
#define FEEDBACK_TS_OFF_LOW (FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE | FEEDBACK_SD_BMS | FEEDBACK_SD_IMD | FEEDBACK_SD_END)

#define FEEDBACK_TS_OFF_VAL  (FEEDBACK_TS_OFF_HIGH)
#define FEEDBACK_TS_OFF_MASK ((FEEDBACK_TS_OFF_HIGH) | (FEEDBACK_TS_OFF_LOW))

#define FEEDBACK_AIRN_CLOSE_HIGH \
    (FEEDBACK_CHECK_MUX | FEEDBACK_BMS_COCKPIT | FEEDBACK_IMD_COCKPIT | \
     FEEDBACK_IMD_FAULT | FEEDBACK_IMPLAUSIBILITY_DETECTED | FEEDBACK_AIRN_STATUS | \
     FEEDBACK_AIRP_STATUS | FEEDBACK_SD_IN | FEEDBACK_SD_OUT | FEEDBACK_SD_END)
#define FEEDBACK_AIRN_CLOSE_LOW (FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE | FEEDBACK_SD_BMS | FEEDBACK_SD_IMD)

#define FEEDBACK_AIRN_CLOSE_VAL  (FEEDBACK_AIRN_CLOSE_HIGH)
#define FEEDBACK_AIRN_CLOSE_MASK ((FEEDBACK_AIRN_CLOSE_HIGH) | (FEEDBACK_AIRN_CLOSE_LOW))

#define FEEDBACK_AIRN_STATUS_HIGH \
    (FEEDBACK_CHECK_MUX | FEEDBACK_BMS_COCKPIT | FEEDBACK_IMD_COCKPIT | \
     FEEDBACK_IMD_FAULT | FEEDBACK_IMPLAUSIBILITY_DETECTED | FEEDBACK_AIRN_GATE | \
     FEEDBACK_AIRP_STATUS | FEEDBACK_SD_IN | FEEDBACK_SD_OUT | FEEDBACK_SD_END)
#define FEEDBACK_AIRN_STATUS_LOW (FEEDBACK_AIRN_STATUS | FEEDBACK_AIRP_GATE | FEEDBACK_SD_BMS | FEEDBACK_SD_IMD)

#define FEEDBACK_AIRN_STATUS_VAL  (FEEDBACK_AIRN_STATUS_HIGH)
#define FEEDBACK_AIRN_STATUS_MASK ((FEEDBACK_AIRN_STATUS_HIGH) | (FEEDBACK_AIRN_STATUS_LOW))

#define FEEDBACK_PC_ON_HIGH \
    (FEEDBACK_CHECK_MUX | FEEDBACK_BMS_COCKPIT | FEEDBACK_IMD_COCKPIT | \
     FEEDBACK_IMD_FAULT | FEEDBACK_IMPLAUSIBILITY_DETECTED | FEEDBACK_AIRN_GATE | \
     FEEDBACK_AIRP_STATUS | FEEDBACK_SD_IN | FEEDBACK_SD_OUT | FEEDBACK_SD_END)
#define FEEDBACK_PC_ON_LOW \
    (FEEDBACK_AIRN_STATUS | FEEDBACK_AIRP_GATE | FEEDBACK_PRECHARGE_STATUS | \
     FEEDBACK_SD_BMS | FEEDBACK_SD_IMD)

#define FEEDBACK_PC_ON_VAL  (FEEDBACK_PC_ON_HIGH)
#define FEEDBACK_PC_ON_MASK ((FEEDBACK_PC_ON_HIGH) | (FEEDBACK_PC_ON_LOW))

#define FEEDBACK_ON_HIGH (FEEDBACK_CHECK_MUX | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE)
#define FEEDBACK_ON_LOW \
    (FEEDBACK_AIRN_STATUS | FEEDBACK_AIRP_STATUS | FEEDBACK_TS_OVER_60V_STATUS | \
     FEEDBACK_TSP_OVER_60V_STATUS)

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

#define FEEDBACK_MUX_VREF 3.3f
#define FEEDBACK_SD_VREF 13.f

#define FEEDBACK_CONVERT_ADC_MUX_TO_VOLTAGE(VALUE) ((VALUE) * (FEEDBACK_MUX_VREF / 4095))
#define FEEDBACK_CONVERT_ADC_SD_TO_VOLTAGE(VALUE) ((VALUE) * (FEEDBACK_SD_VREF / 4095))

/** @brief State of the feedbacks */
typedef enum {
    FEEDBACK_STATE_L,
    FEEDBACK_STATE_H,
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
 * @brief Check if the multiplexer feedback voltage is in the correct voltage range
 * 
 * @return true The MUX voltage is in the correct range
 * @return false Otherwise
 */
bool is_check_mux_ok();
/**
 * @brief Get the state of a single feedback
 * 
 * @param index The index of the feedback
 * @return feedback_feed_t The feedback state
 */
feedback_feed_t feedback_get_feedback_state(size_t index);
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