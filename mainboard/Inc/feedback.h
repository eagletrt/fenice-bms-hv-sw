/**
 * @file		feedback.h
 * @brief		Feedback parsing utilities
 *
 * @date		Mar 16, 2021
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include "error.h"

#include <inttypes.h>

typedef uint16_t feedback_t;

extern feedback_t feedback;

/**
 * Feedback bit set bit position 
 */
enum {
    FEEDBACK_VREF_POS,
    FEEDBACK_FROM_TSMS_POS,
    FEEDBACK_TO_TSMS_POS,
    FEEDBACK_FROM_SHUTDOWN_POS,
    FEEDBACK_LATCH_IMD_POS,
    FEEDBACK_LATCH_BMS_POS,
    FEEDBACK_IMD_FAULT_POS,
    FEEDBACK_BMS_FAULT_POS,
    FEEDBACK_TSAL_HV_POS,
    FEEDBACK_AIR_POSITIVE_POS,
    FEEDBACK_AIR_NEGATIVE_POS,
    FEEDBACK_PC_END_POS,
    FEEDBACK_RELAY_LV_POS,
    FEEDBACK_IMD_SHUTDOWN_POS,
    FEEDBACK_BMS_SHUTDOWN_POS,
    FEEDBACK_TS_ON_POS,

    //do not move FEEDBACK_N
    FEEDBACK_N,
};

/**
 * Feedback bit sets 
 */
#define FEEDBACK_NULL          0
#define FEEDBACK_VREF          ((feedback_t)1 << FEEDBACK_VREF_POS)
#define FEEDBACK_FROM_TSMS     ((feedback_t)1 << FEEDBACK_FROM_TSMS_POS)
#define FEEDBACK_TO_TSMS       ((feedback_t)1 << FEEDBACK_TO_TSMS_POS)
#define FEEDBACK_FROM_SHUTDOWN ((feedback_t)1 << FEEDBACK_FROM_SHUTDOWN_POS)
#define FEEDBACK_LATCH_IMD     ((feedback_t)1 << FEEDBACK_LATCH_IMD_POS)
#define FEEDBACK_LATCH_BMS     ((feedback_t)1 << FEEDBACK_LATCH_BMS_POS)
#define FEEDBACK_IMD_FAULT     ((feedback_t)1 << FEEDBACK_IMD_FAULT_POS)
#define FEEDBACK_BMS_FAULT     ((feedback_t)1 << FEEDBACK_BMS_FAULT_POS)
#define FEEDBACK_TSAL_HV       ((feedback_t)1 << FEEDBACK_TSAL_HV_POS)
#define FEEDBACK_AIR_POSITIVE  ((feedback_t)1 << FEEDBACK_AIR_POSITIVE_POS)
#define FEEDBACK_AIR_NEGATIVE  ((feedback_t)1 << FEEDBACK_AIR_NEGATIVE_POS)
#define FEEDBACK_PC_END        ((feedback_t)1 << FEEDBACK_PC_END_POS)
#define FEEDBACK_RELAY_LV      ((feedback_t)1 << FEEDBACK_RELAY_LV_POS)
#define FEEDBACK_IMD_SHUTDOWN  ((feedback_t)1 << FEEDBACK_IMD_SHUTDOWN_POS)
#define FEEDBACK_BMS_SHUTDOWN  ((feedback_t)1 << FEEDBACK_BMS_SHUTDOWN_POS)
#define FEEDBACK_TS_ON         ((feedback_t)1 << FEEDBACK_TS_ON_POS)
#define FEEDBACK_ALL           (feedback_t)(((feedback_t)1 << FEEDBACK_N) - 1)

// Feedbacks to check after TS OFF
#define FEEDBACK_TS_OFF_VAL FEEDBACK_VREF
#define FEEDBACK_TS_OFF_MASK \
    FEEDBACK_VREF | FEEDBACK_AIR_POSITIVE | FEEDBACK_AIR_NEGATIVE | FEEDBACK_PC_END | FEEDBACK_TS_ON

// Feedback states before closing AIR- and trigger PC
#define FEEDBACK_IDLE_TS_ON_TRIGGER_VAL                                                                    \
    (FEEDBACK_VREF | FEEDBACK_FROM_TSMS | FEEDBACK_TO_TSMS | FEEDBACK_FROM_SHUTDOWN | FEEDBACK_LATCH_IMD | \
     FEEDBACK_LATCH_BMS | FEEDBACK_IMD_FAULT | FEEDBACK_BMS_FAULT)
#define FEEDBACK_IDLE_TS_ON_TRIGGER_MASK FEEDBACK_ALL

// Feedback states after closing AIR-
#define FEEDBACK_TO_PRECHARGE_VAL                                                                          \
    (FEEDBACK_IDLE_TS_ON_TRIGGER_VAL | FEEDBACK_AIR_NEGATIVE | FEEDBACK_RELAY_LV | FEEDBACK_IMD_SHUTDOWN | \
     FEEDBACK_BMS_FAULT | FEEDBACK_BMS_SHUTDOWN | FEEDBACK_TS_ON)
#define FEEDBACK_TO_PRECHARGE_MASK FEEDBACK_ALL &(~FEEDBACK_TSAL_HV)

#define FEEDBACK_ON_VAL  FEEDBACK_TO_PRECHARGE_VAL | FEEDBACK_TSAL_HV
#define FEEDBACK_ON_MASK FEEDBACK_ALL

bool feedback_check(feedback_t fb_check_mask, feedback_t fb_value, error_id error_id);
void feedback_read(feedback_t fb_mask);
bool feedback_check_charge();
bool feedback_check_precharge();
bool feedback_check_on();
