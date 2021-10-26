/**
 * @file    pack.c
 * @brief   This file contains the functions to manage the battery pack state
 *
 * @date    Apr 11, 2019
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "pack/pack.h"

#include "feedback.h"

bool pack_set_ts_off() {
    //Switch off airs
    HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_RESET);

    feedback_read(FEEDBACK_TS_OFF_MASK);
    return feedback_check(FEEDBACK_TS_OFF_MASK, FEEDBACK_TS_OFF_VAL, ERROR_FEEDBACK);
}

bool pack_set_pc_start() {
    //switch on AIR-
    HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_SET);

    // Check feedback
    feedback_read(FEEDBACK_TO_PRECHARGE_MASK);
    return feedback_check(FEEDBACK_TO_PRECHARGE_MASK, FEEDBACK_TO_PRECHARGE_VAL, ERROR_FEEDBACK);
}

bool pack_set_precharge_end() {
    //switch on AIR+
    HAL_GPIO_WritePin(AIRP_OFF_GPIO_Port, AIRP_OFF_Pin, GPIO_PIN_RESET);

    // Check feedback
    feedback_read(FEEDBACK_ON_MASK);
    return feedback_check(FEEDBACK_ON_MASK, FEEDBACK_ON_VAL, ERROR_FEEDBACK);
}
