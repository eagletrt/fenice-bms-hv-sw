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
#define FEEDBACK_IDLE_LOW \
    ( \
        FEEDBACK_AIRN_GATE | \
        FEEDBACK_AIRP_GATE | \
        FEEDBACK_SD_BMS | \
        FEEDBACK_SD_IMD \
    )
#define FEEDBACK_IDLE_MASK ((FEEDBACK_IDLE_HIGH) | (FEEDBACK_IDLE_LOW))


#define FEEDBACK_FATAL_ERROR_HIGH \
    ( \
        FEEDBACK_IMPLAUSIBILITY_DETECTED | \
        FEEDBACK_IMD_COCKPIT | \
        FEEDBACK_TSAL_GREEN_FAULT_LATCHED | \
        FEEDBACK_BMS_COCKPIT | \
        FEEDBACK_EXT_LATCHED | \
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
#define FEEDBACK_FATAL_ERROR_LOW \
    ( \
        FEEDBACK_AIRN_GATE | \
        FEEDBACK_AIRP_GATE | \
        FEEDBACK_SD_BMS | \
        FEEDBACK_SD_IMD \
    )
#define FEEDBACK_FATAL_ERROR_MASK ((FEEDBACK_FATAL_ERROR_HIGH) | (FEEDBACK_FATAL_ERROR_LOW))

#define FEEDBACK_AIRN_CHECK_HIGH \
    ( \
        FEEDBACK_IMPLAUSIBILITY_DETECTED | \
        FEEDBACK_IMD_COCKPIT | \
        FEEDBACK_TSAL_GREEN_FAULT_LATCHED | \
        FEEDBACK_BMS_COCKPIT | \
        FEEDBACK_EXT_LATCHED | \
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
#define FEEDBACK_AIRN_CHECK_LOW \
    ( \
        FEEDBACK_AIRN_STATUS | \
        FEEDBACK_AIRP_GATE | \
        FEEDBACK_SD_BMS | \
        FEEDBACK_SD_IMD \
    )
#define FEEDBACK_AIRN_CHECK_MASK ((FEEDBACK_AIRN_CHECK_HIGH) | (FEEDBACK_AIRN_CHECK_LOW))

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
#define FEEDBACK_PRECHARGE_CHECK_LOW \
    ( \
        FEEDBACK_TS_OVER_60V_STATUS | \
        FEEDBACK_AIRN_STATUS | \
        FEEDBACK_AIRP_GATE | \
        FEEDBACK_PRECHARGE_STATUS | \
        FEEDBACK_TSP_OVER_60V_STATUS | \
        FEEDBACK_SD_BMS | \
        FEEDBACK_SD_IMD \
    )
#define FEEDBACK_PRECHARGE_CHECK_MASK ((FEEDBACK_PRECHARGE_CHECK_HIGH) | (FEEDBACK_PRECHARGE_CHECK_LOW))

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
#define FEEDBACK_AIRP_CHECK_LOW \
    ( \
        FEEDBACK_TS_OVER_60V_STATUS | \
        FEEDBACK_AIRN_STATUS | \
        FEEDBACK_AIRP_STATUS | \
        FEEDBACK_PRECHARGE_STATUS | \
        FEEDBACK_TSP_OVER_60V_STATUS | \
        FEEDBACK_SD_BMS | \
        FEEDBACK_SD_IMD \
    )
#define FEEDBACK_AIRP_CHECK_MASK ((FEEDBACK_AIRP_CHECK_HIGH) | (FEEDBACK_AIRP_CHECK_LOW))

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
#define FEEDBACK_TS_ON_CHECK_LOW \
    ( \
        FEEDBACK_TS_OVER_60V_STATUS | \
        FEEDBACK_TSP_OVER_60V_STATUS | \
        FEEDBACK_SD_BMS | \
        FEEDBACK_SD_IMD \
    )
#define FEEDBACK_TS_ON_CHECK_MASK ((FEEDBACK_TS_ON_CHECK_HIGH) | (FEEDBACK_TS_ON_CHECK_LOW))


typedef uint32_t feedback_t;

/** @brief State of the feedbacks */
typedef enum {
    FEEDBACK_STATE_L,
    FEEDBACK_STATE_H,
    FEEDBACK_STATE_ERROR
} FEEDBACK_STATE;

/** @brief Information about feedbacks */
typedef struct {
    FEEDBACK_STATE real_state;
    FEEDBACK_STATE cur_state;
    float voltage;
} feedback_feed_t;



/** @brief Feedback timer callback handler */
void _feedback_handle_tim_elapsed_irq();
/** @brief Feedback ADC callback handler */
void _feedback_handle_adc_cnv_cmpl_irq();

uint16_t feedback_get_volt(size_t index);

/** @brief Initialize the feedbacks */
void feedback_init();
/**
 * @brief Check if the feedbacks specified in the mask are in the right range
 * 
 * @param mask A bitmask to select the feedbacks
 * @param value A bitset used to specify the feedbacks that should have a logic value of 1
 * @return true If all the feedbacks are ok
 * @return false Otherwise
 */
bool feedback_is_ok(feedback_t mask, feedback_t value);
/**
 * @brief Get the status of a single feedback
 * 
 * @param index The index of the feedback
 * @return feedback_feed_t The information about the status of the feedback
 */
feedback_feed_t feedback_get_state(size_t index);
/**
 * @brief Get the status of all the feedbacks
 * 
 * @param out_value An array where the resulting status are stored
 */
void feedback_get_all_states(feedback_feed_t out_value[FEEDBACK_N]);

#endif // FEEDBACK_H