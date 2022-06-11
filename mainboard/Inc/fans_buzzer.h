#include "main.h"
#include "pwm.h"

#define PWM_BUZZER_CHANNEL TIM_CHANNEL_3
#define PWM_FANS_CHANNEL   TIM_CHANNEL_4

#define PWM_FANS_STANDARD_PERIOD 0.03846153846  //26kHz

void fans_init();
void fans_set_speed(float power_percentage);

void BUZ_sborati(TIM_HandleTypeDef *htim);