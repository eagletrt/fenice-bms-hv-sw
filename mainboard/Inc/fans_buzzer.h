#include "main.h"
#include "pwm.h"

#define PWM_BUZZER_CHANNEL TIM_CHANNEL_3
#define PWM_FANS_CHANNEL   TIM_CHANNEL_4

#define PWM_FANS_STANDARD_PERIOD 0.04  //25kHz

extern uint8_t fans_override;
extern float fans_override_value;

void fans_init();
void fans_set_speed(float power_percentage);
void fans_set_speed_from_temp(float temp);

void BUZ_sborati(TIM_HandleTypeDef *htim);