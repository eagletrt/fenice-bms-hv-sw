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

typedef uint32_t feedback_t;

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



/** @brief Feedback timer callback handler */
void _feedback_handle_tim_elapsed_irq();
/** @brief Feedback ADC callback handler */
void _feedback_handle_adc_cnv_cmpl_irq();


/** @brief Initialize the feedbacks */
void feedback_init();
/**
 * @brief Check if a feedbacks needs to be updated
 * 
 * @return true If the feedbacks needs to be updated
 * @return false Otherwise
 */
bool feedback_need_update();
/**
 * @brief Check the multiplexer VDC
 * 
 * @param handcart_connected Handcart connection status
 * @return true If the VDC is in the correct voltage range
 * @return false Otherwise
 */
bool feedback_check_mux_vdc(bool handcart_connected);
/**
 * @brief Check all the feedbacks from the multiplexer except the VDC
 * 
 * @param value The expected values of the feedbacks
 * @param handcart_connected Handcart connection status
 * @return feedback_t A bitset of feedbacks that set an error
 */
feedback_t feedback_check_mux(feedback_t value, bool handcart_connected);
/**
 * @brief Check the shutdown feedbacks
 * @details In the shutdown feedbacks are included:
 *  - SD_IMD
 *  - SD_BMS
 *  - SD_IN
 *  - SD_OUT
 * 
 * @param value The excpected values of the feedbacks
 * @return feedback_t A bitset of feedbacks that set an error
 */
feedback_t feedback_check_sd(feedback_t value);
/**
 * @brief Get the status of a single feedback
 * 
 * @param index The index of the feedback
 * @param handcart_connected Handcart connection status
 * @return feedback_feed_t The information about the status of the feedback
 */
feedback_feed_t feedback_get_state(size_t index, bool handcart_connected);
/**
 * @brief Get the status of all the feedbacks
 * 
 * @param out_value An array where the resulting status are stored
 * @param handcart_connected Handcart connection status
 */
void feedback_get_all_states(feedback_feed_t out_value[FEEDBACK_N], bool handcart_connected);


#endif // FEEDBACK_H