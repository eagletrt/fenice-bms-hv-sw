#include "main.h"
#include "pwm.h"

#define PWM_BUZZER_CHANNEL TIM_CHANNEL_1
#define PWM_FANS_CHANNEL   TIM_CHANNEL_3

#define PWM_FANS_STANDARD_PERIOD 0.03846153846  //26kHz
#define FANS_START_TEMP 30.f

void fans_init();
void fans_set_speed(float power_percentage);
float fans_curve(float temp);

void BUZ_sborati(TIM_HandleTypeDef *htim);