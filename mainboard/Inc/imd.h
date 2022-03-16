/**
 * @file    imd.h
 * @brief   functions that checks imd status
 *
 * @date    Dec 01, 2021
 *
 * @author  Federico Carbone [federico.carbone@studenti.unitn.it]
 */

#pragma once

#include "main.h"
#include "mainboard_config.h"
#include "tim.h"

typedef enum { IMD_SC, IMD_NORMAL, IMD_UNDER_VOLTAGE, IMD_START_MEASURE, IMD_DEVICE_ERROR, IMD_EARTH_FAULT } IMD_STATE;

void imd_init();
/**
 * @brief	gets the pwm frequency
 */
uint8_t imd_get_freq();
/**
 * @brief	gets the pwm period
 */
uint8_t imd_get_period();
/**
 * @brief	gets pwm duty cycle
 * @returns a value between 0 and 1
 */
float imd_get_duty_cycle();
/**
 * @brief	gets the duty cycle in percentage
 */
float imd_get_duty_cycle_percentage();
/**
 * @brief	reads the fb_imd_fault
 * @details reads the fb_imd_fault pin and 
 * @returns its negate value (fault pin is active low)
 */
uint8_t imd_is_fault();
/**
 * @brief	gets the imd state
 */
IMD_STATE imd_get_state();
/**
 * @brief	gets the imd details
 * @details based on the imd state this function 
 * @returns IMD_SC              ==> 0 or 1
 *          IMD_NORMAL          ==> resistance value calculated
 *          IMD_START_MEASURE   ==> good (1) or bad (0)
 *          IMD_DEVICE_ERROR    ==> 1 when valid, -1 when not
 *          IMD_EARTH_FAULT     ==> 1 when valid, -1 when not
 */
int16_t imd_get_details();