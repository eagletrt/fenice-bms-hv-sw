/**
 * @file    pack.c
 * @brief   This file contains the functions to manage the battery pack state
 *
 * @date    Apr 11, 2019
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author  Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#include "pack/pack.h"

#include "feedback.h"

void pack_set_airn_off(uint8_t value) {
    HAL_GPIO_WritePin(AIRN_OFF_GPIO_Port, AIRN_OFF_Pin, value);
}

void pack_set_precharge(uint8_t value) {
    HAL_GPIO_WritePin(PRECHARGE_GPIO_Port, PRECHARGE_Pin, value);
}

void pack_set_airp_off(uint8_t value) {
    HAL_GPIO_WritePin(AIRP_OFF_GPIO_Port, AIRP_OFF_Pin, value);
}

void pack_set_fault(uint8_t value) {
    HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, value);
}

void pack_set_default_off(uint16_t prech_delay) {
    pack_set_airn_off(AIRN_OFF_VALUE);
    pack_set_airp_off(AIRP_OFF_VALUE);
    if(prech_delay) HAL_Delay(prech_delay);
    pack_set_precharge(PRECHARGE_OFF_VALUE);
}
