#include "tim.h"

#define VOLTS_START_CONVERTION_CHANNEL          TIM_CHANNEL_1
#define VOLTS_READ_CHANNEL                      TIM_CHANNEL_2
#define TEMPS_READ_CHANNEL                      TIM_CHANNEL_3

#define VOLTS_START_CONVERTION_ACTIVE_CHANNEL   HAL_TIM_ACTIVE_CHANNEL_1
#define VOLTS_READ_ACTIVE_CHANNEL               HAL_TIM_ACTIVE_CHANNEL_2
#define TEMPS_READ_ACTIVE_CHANNEL               HAL_TIM_ACTIVE_CHANNEL_3

typedef uint8_t measurements_flag_t;

enum {
    MEASUREMENTS_VOLTS_START_CONVERTION_FLAG = 1,
    MEASUREMENTS_VOLTS_READ_FLAG = 2,
    MEASUREMENTS_TEMPS_READ_FLAG = 4
};

void measurements_init(TIM_HandleTypeDef *htim);

void measurements_flags_check();

void measurements_oc_handler(TIM_HandleTypeDef *htim);