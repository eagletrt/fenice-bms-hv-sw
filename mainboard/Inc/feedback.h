/**
 * @file		feedback.h
 * @brief		Feedback parsing utilities
 *
 * @date		Mar 16, 2021
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#pragma once

#include <inttypes.h>

#include "error.h"
#include "fenice_config.h"

typedef uint16_t feedback_t;

extern feedback_t feedback;

// Feedbacks to check after TS OFF
#define FEEDBACK_TS_OFF_VAL FEEDBACK_VREF
#define FEEDBACK_TS_OFF_MASK FEEDBACK_VREF | FEEDBACK_AIR_POSITIVE | FEEDBACK_AIR_NEGATIVE | FEEDBACK_PC_END | FEEDBACK_TS_ON

// Feedback states before closing AIR- and trigger PC
#define FEEDBACK_IDLE_TS_ON_TRIGGER_VAL \
	(FEEDBACK_VREF |                    \
	 FEEDBACK_FROM_TSMS |               \
	 FEEDBACK_TO_TSMS |                 \
	 FEEDBACK_FROM_SHUTDOWN |           \
	 FEEDBACK_LATCH_IMD |               \
	 FEEDBACK_LATCH_BMS |               \
	 FEEDBACK_IMD_FAULT |               \
	 FEEDBACK_BMS_FAULT)
#define FEEDBACK_IDLE_TS_ON_TRIGGER_MASK FEEDBACK_ALL

// Feedback states after closing AIR-
#define FEEDBACK_TO_PRECHARGE_VAL      \
	(FEEDBACK_IDLE_TS_ON_TRIGGER_VAL | \
	 FEEDBACK_AIR_NEGATIVE |           \
	 FEEDBACK_RELAY_LV |               \
	 FEEDBACK_IMD_SHUTDOWN |           \
	 FEEDBACK_BMS_FAULT |              \
	 FEEDBACK_BMS_SHUTDOWN |           \
	 FEEDBACK_TS_ON)
#define FEEDBACK_TO_PRECHARGE_MASK FEEDBACK_ALL &(~FEEDBACK_TSAL_HV)

#define FEEDBACK_ON_VAL FEEDBACK_TO_PRECHARGE_VAL | FEEDBACK_TSAL_HV
#define FEEDBACK_ON_MASK FEEDBACK_ALL

bool feedback_check(feedback_t fb_check_mask, feedback_t fb_value, error_id error_id);
void feedback_read(feedback_t fb_mask);
bool feedback_check_charge();
bool feedback_check_precharge();
bool feedback_check_on();
