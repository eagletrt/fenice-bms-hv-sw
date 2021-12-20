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
#include "fenice_config.h"

#define MUX_INTERVAL_MS 15
#define MUX_DELAY_MS    3

typedef uint32_t feedback_t;

extern feedback_t feedback;

#define FEEDBACK_N (FEEDBACK_MUX_N + FEEDBACK_GPIO_N)

/*
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

//#define FEEDBACK_ON_VAL  FEEDBACK_TO_PRECHARGE_VAL | FEEDBACK_TSAL_HV
*/


// Feedbacks to check after TS OFF
#define FEEDBACK_TS_OFF_VAL \
        FEEDBACK_CHECK_MUX | FEEDBACK_AIRP | FEEDBACK_AIRN | FEEDBACK_LATCH_BMS

#define FEEDBACK_TS_OFF_MASK \
        FEEDBACK_TS_OFF_VAL /*| FEEDBACK_PC_END*/ | FEEDBACK_TS_ON | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE



#define FEEDBACK_TS_ON_VAL \
        FEEDBACK_CHECK_MUX | FEEDBACK_AIRP | FEEDBACK_AIRN | FEEDBACK_FROM_SD | FEEDBACK_LATCH_IMD | FEEDBACK_LATCH_BMS | FEEDBACK_IMD_FAULT | FEEDBACK_BMS_FAULT

#define FEEDBACK_TS_ON_MASK \
        FEEDBACK_TS_ON_VAL | FEEDBACK_TS_ON | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE



#define FEEDBACK_AIRN_OFF_VAL \
        FEEDBACK_CHECK_MUX | FEEDBACK_AIRP | FEEDBACK_AIRN | FEEDBACK_FROM_SD | FEEDBACK_LATCH_IMD | FEEDBACK_LATCH_BMS | FEEDBACK_IMD_FAULT | FEEDBACK_BMS_FAULT | FEEDBACK_TO_TSMS | FEEDBACK_FROM_TSMS | FEEDBACK_TS_ON

#define FEEDBACK_AIRN_OFF_MASK \
        FEEDBACK_AIRN_OFF_VAL | FEEDBACK_AIRN_GATE | FEEDBACK_AIRP_GATE



#define FEEDBACK_PC_ON_VAL \
        FEEDBACK_CHECK_MUX | FEEDBACK_AIRP | FEEDBACK_AIRN_GATE | FEEDBACK_FROM_SD | FEEDBACK_LATCH_IMD | FEEDBACK_LATCH_BMS | FEEDBACK_IMD_FAULT | FEEDBACK_BMS_FAULT | FEEDBACK_TO_TSMS | FEEDBACK_FROM_TSMS | FEEDBACK_TS_ON | FEEDBACK_RELAY_SD | FEEDBACK_IMD_SD | FEEDBACK_BMS_SD | FEEDBACK_TS_ON

#define FEEDBACK_PC_ON_MASK \
        FEEDBACK_PC_ON_VAL | FEEDBACK_AIRN | FEEDBACK_AIRP_GATE


#define FEEDBACK_ON_VAL \
        FEEDBACK_CHECK_MUX | FEEDBACK_AIRP_GATE | FEEDBACK_AIRN_GATE | FEEDBACK_FROM_SD | FEEDBACK_LATCH_IMD | FEEDBACK_LATCH_BMS | FEEDBACK_IMD_FAULT | FEEDBACK_BMS_FAULT | FEEDBACK_TO_TSMS | FEEDBACK_FROM_TSMS | FEEDBACK_TS_ON | FEEDBACK_RELAY_SD | FEEDBACK_IMD_SD | FEEDBACK_BMS_SD | FEEDBACK_TS_ON

#define FEEDBACK_ON_MASK FEEDBACK_ALL



#define CONVERT_ADC_TO_VOLTAGE(VALUE) ((VALUE) * 3.3F / 4096)


void feedback_init();
bool feedback_check(feedback_t fb_check_mask, feedback_t fb_value, error_id error_id);
void feedback_read(feedback_t fb_mask);
bool feedback_check_charge();
bool feedback_check_precharge();
bool feedback_check_on();

uint8_t feedback_get_adc_index();
void feedback_incr_adc_index();
void feedback_set_next_mux_index();
void feedback_save_value(uint32_t adc_value, uint8_t index);