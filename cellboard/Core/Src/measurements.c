#include "measurements.h"
#include "volt.h"
#include "temp.h"
#include "tim.h"

bool volts_start_conv_flag=false, volts_read_flag=false, temps_read_flag=false;

void measurements_init(TIM_HandleTypeDef *htim) {
    __HAL_TIM_SetCompare(htim, VOLTS_START_CONVERTION_CHANNEL, TIM_MS_TO_TICKS(htim, VOLT_MEASURE_INTERVAL - VOLT_MEASURE_TIME));
    __HAL_TIM_SetCompare(htim, VOLTS_READ_CHANNEL, TIM_MS_TO_TICKS(htim, VOLT_MEASURE_INTERVAL));
    __HAL_TIM_SetCompare(htim, TEMPS_READ_CHANNEL, TIM_MS_TO_TICKS(htim, TEMP_MEASURE_INTERVAL));
    HAL_TIM_OC_Start_IT(htim, VOLTS_START_CONVERTION_CHANNEL);
    HAL_TIM_OC_Start_IT(htim, VOLTS_READ_CHANNEL);
    HAL_TIM_OC_Start_IT(htim, TEMPS_READ_CHANNEL);
}

void measurements_flags_check() {
    if(volts_start_conv_flag) {
        volt_start_measure();
        volts_start_conv_flag = false;
    }
    if(volts_read_flag) {
        volt_read();
        can_send(TOPIC_VOLTAGE_INFO_FILTER);
        volts_read_flag = false;
    }
    if(temps_read_flag) {
        temp_measure_all();
        can_send(TOPIC_TEMPERATURE_INFO_FILTER);
        temps_read_flag = false;
    }
}

void measurements_oc_handler(TIM_HandleTypeDef *htim) {
    uint32_t compare_value = __HAL_TIM_GetCounter(htim);
    switch (htim->Channel)
    {
        case VOLTS_START_CONVERTION_ACTIVE_CHANNEL:
            volts_start_conv_flag = true;
            break;

        case VOLTS_READ_ACTIVE_CHANNEL:
            volts_read_flag = true;
            __HAL_TIM_SetCompare(htim, VOLTS_START_CONVERTION_CHANNEL, compare_value + TIM_MS_TO_TICKS(htim, VOLT_MEASURE_INTERVAL - VOLT_MEASURE_TIME));
            __HAL_TIM_SetCompare(htim, VOLTS_READ_CHANNEL, compare_value + TIM_MS_TO_TICKS(htim, VOLT_MEASURE_INTERVAL));
            break;
        
        case TEMPS_READ_ACTIVE_CHANNEL:
            __HAL_TIM_SetCompare(htim, TEMPS_READ_CHANNEL, compare_value + TIM_MS_TO_TICKS(htim, TEMP_MEASURE_INTERVAL));
            temps_read_flag = true;
            break;

        default:
            break;
    }
}