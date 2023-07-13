/**
 * @file fans_buzzer.h
 * @brief Functions to handle fans and buzzer
 * 
 * @date Jul 12, 2023
 * 
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef FANS_BUZZER_H
#define FANS_BUZZER_H

#include "main.h"
#include "pwm.h"

#define PWM_BUZZER_CHANNEL TIM_CHANNEL_1
#define PWM_FANS_CHANNEL   TIM_CHANNEL_3

#define PWM_FANS_STANDARD_PERIOD 0.03846153846  //26kHz
#define FANS_START_TEMP 30.f

/** @brief Initialize fans */
void fans_init();
/** @brief Toggle fans speed override */
void fans_toggle_override();
/**
 * @brief Check if fans speed is overrided
 * 
 * @return true If the fans speed is overrided
 * @return false Otherwise
 */
bool fans_is_overrided();
/**
 * @brief Set fans speed using PWM
 * @attention The percentage can be between 0 (off) and 1
 * any value out of range will be clamped to the nearest value in the range
 * 
 * @param power_percentage The fans speed (%)
 */
void fans_set_speed(float power_percentage);
/**
 * @brief A function that maps temperatures (°C) to a fans speeds (%)
 * 
 * @param temp The input temperature
 * @return float The fans speed (%)
 */
float fans_curve(float temp);

/**
 * @brief Play something with the buzzer (sborato)
 * 
 * @param htim The timer handler for the buzzer
 */
void BUZ_sborati(TIM_HandleTypeDef * htim);

#endif // FANS_BUZZER_H
