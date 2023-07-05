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
#include "bms_fsm.h"


typedef uint32_t feedback_t;

extern uint16_t feedbacks[FEEDBACK_N];

/**
 * @brief Feedbacks that should be logical high for each state of the FSM
*/
#define FEEDBACK_IDLE_HIGH \
    ( \
        FEEDBACK_IMPLAUSIBILITY_DETECTED | \
        FEEDBACK_IMD_COCKPIT | \
        FEEDBACK_TSAL_GREEN_FAULT_LATCHED | \
        FEEDBACK_BMS_COCKPIT | \
        FEEDBACK_EXT_LATCHED | \
        FEEDBACK_TSAL_GREEN | \
        FEEDBACK_TS_OVER_60V_STATUS | \
        FEEDBACK_AIRN_STATUS | \
        FEEDBACK_AIRP_STATUS | \
        FEEDBACK_PRECHARGE_STATUS | \
        FEEDBACK_TSP_OVER_60V_STATUS | \
        FEEDBACK_IMD_FAULT | \
        FEEDBACK_CHECK_MUX | \
        FEEDBACK_SD_END | \
        FEEDBACK_SD_OUT | \
        FEEDBACK_SD_IN \
    )
#define FEEDBACK_AIRN_CHECK_HIGH \
    ( \
        FEEDBACK_IMPLAUSIBILITY_DETECTED | \
        FEEDBACK_IMD_COCKPIT | \
        FEEDBACK_TSAL_GREEN_FAULT_LATCHED | \
        FEEDBACK_BMS_COCKPIT | \
        FEEDBACK_EXT_LATCHED | \
        FEEDBACK_TSAL_GREEN | \
        FEEDBACK_TS_OVER_60V_STATUS | \
        FEEDBACK_AIRP_STATUS | \
        FEEDBACK_AIRN_GATE | \
        FEEDBACK_PRECHARGE_STATUS | \
        FEEDBACK_TSP_OVER_60V_STATUS | \
        FEEDBACK_IMD_FAULT | \
        FEEDBACK_CHECK_MUX | \
        FEEDBACK_SD_END | \
        FEEDBACK_SD_OUT | \
        FEEDBACK_SD_IN \
    )
#define FEEDBACK_PRECHARGE_CHECK_HIGH \
    ( \
        FEEDBACK_IMPLAUSIBILITY_DETECTED | \
        FEEDBACK_IMD_COCKPIT | \
        FEEDBACK_TSAL_GREEN_FAULT_LATCHED | \
        FEEDBACK_BMS_COCKPIT | \
        FEEDBACK_EXT_LATCHED | \
        FEEDBACK_AIRP_STATUS | \
        FEEDBACK_AIRN_GATE | \
        FEEDBACK_IMD_FAULT | \
        FEEDBACK_CHECK_MUX | \
        FEEDBACK_SD_END | \
        FEEDBACK_SD_OUT | \
        FEEDBACK_SD_IN \
    )
#define FEEDBACK_AIRP_CHECK_HIGH \
    ( \
        FEEDBACK_IMPLAUSIBILITY_DETECTED | \
        FEEDBACK_IMD_COCKPIT | \
        FEEDBACK_TSAL_GREEN_FAULT_LATCHED | \
        FEEDBACK_BMS_COCKPIT | \
        FEEDBACK_EXT_LATCHED | \
        FEEDBACK_AIRN_GATE | \
        FEEDBACK_AIRP_GATE | \
        FEEDBACK_IMD_FAULT | \
        FEEDBACK_CHECK_MUX | \
        FEEDBACK_SD_END | \
        FEEDBACK_SD_OUT | \
        FEEDBACK_SD_IN \
    )
#define FEEDBACK_TS_ON_CHECK_HIGH \
    ( \
        FEEDBACK_IMPLAUSIBILITY_DETECTED | \
        FEEDBACK_IMD_COCKPIT | \
        FEEDBACK_TSAL_GREEN_FAULT_LATCHED | \
        FEEDBACK_BMS_COCKPIT | \
        FEEDBACK_EXT_LATCHED | \
        FEEDBACK_AIRN_GATE | \
        FEEDBACK_AIRP_GATE | \
        FEEDBACK_IMD_FAULT | \
        FEEDBACK_CHECK_MUX | \
        FEEDBACK_SD_END | \
        FEEDBACK_SD_OUT | \
        FEEDBACK_SD_IN \
    )

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
bool feedback_is_mux_ok();
/**
 * @brief Check if the feedbacks are updated
 * 
 * @return true The feedbacks are updated
 * @return false 
 */
bool feedback_is_updated();
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
feedback_t feedback_check(feedback_t value);
/** @brief Request a feedback update */
void feedback_request_update();
/**
 * @brief Set the errors of the feedbacks
 * 
 * @param state The current state of the FSM
 */
void feedback_set_errors(state_t state);


/** @brief Feedback timer callback handler */
void _feedback_handle_tim_elapsed_irq();
/** @brief Feedback ADC callback handler */
void _feedback_handle_adc_cnv_cmpl_irq();

#endif // FEEDBACK_H