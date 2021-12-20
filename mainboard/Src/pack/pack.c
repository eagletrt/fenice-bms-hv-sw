/**
 * @file    pack.c
 * @brief   This file contains the functions to manage the battery pack state
 *
 * @date    Apr 11, 2019
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "pack/pack.h"

#include "feedback.h"

void pack_set_ts_on() {
    HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_SET);
}

void pack_set_airn_off(uint8_t value) {
    HAL_GPIO_WritePin(AIRN_OFF_GPIO_Port, AIRN_OFF_Pin, value);
}

void pack_set_pc_on() {
    //HAL_GPIO_WritePin(PC_ON)
}

bool pack_set_ts_off() {
    //Switch off airs
    HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_RESET);

    //feedback_read(FEEDBACK_TS_OFF_MASK);
    return 0;//feedback_check(FEEDBACK_TS_OFF_MASK, FEEDBACK_TS_OFF_VAL, ERROR_FEEDBACK);
}

void pack_set_pc_start() {
    HAL_GPIO_WritePin(PC_ON_GPIO_Port, PC_ON_Pin, GPIO_PIN_SET);

    /*
    // Check feedback
    feedback_read(FEEDBACK_TO_PRECHARGE_MASK);
    return feedback_check(FEEDBACK_TO_PRECHARGE_MASK, FEEDBACK_TO_PRECHARGE_VAL, ERROR_FEEDBACK);
    */
}

void pack_set_precharge_end() {
    //switch on AIR+
    HAL_GPIO_WritePin(AIRP_OFF_GPIO_Port, AIRP_OFF_Pin, GPIO_PIN_RESET);
}

void pack_set_fault(uint8_t value) {
    HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, value);
}
