#include "measurements.h"

#include "temp.h"
#include "tim.h"
#include "volt.h"

measurements_flag_t flags;

void measurements_init(TIM_HandleTypeDef *htim) {
    __HAL_TIM_SetCompare(
        htim, VOLTS_START_CONVERTION_CHANNEL, TIM_MS_TO_TICKS(htim, VOLT_MEASURE_INTERVAL - VOLT_MEASURE_TIME));
    __HAL_TIM_SetCompare(htim, VOLTS_READ_CHANNEL, TIM_MS_TO_TICKS(htim, VOLT_MEASURE_INTERVAL));
    __HAL_TIM_SetCompare(htim, TEMPS_READ_CHANNEL, TIM_MS_TO_TICKS(htim, TEMP_MEASURE_INTERVAL));
    HAL_TIM_OC_Start_IT(htim, VOLTS_START_CONVERTION_CHANNEL);
    HAL_TIM_OC_Start_IT(htim, VOLTS_READ_CHANNEL);
    HAL_TIM_OC_Start_IT(htim, TEMPS_READ_CHANNEL);
    flags = 0;
}

void measurements_flags_check() {
    if (flags & MEASUREMENTS_VOLTS_START_CONVERTION_FLAG) {
        volt_start_measure();
        flags &= ~MEASUREMENTS_VOLTS_START_CONVERTION_FLAG;
    }
    if (flags & MEASUREMENTS_VOLTS_READ_FLAG) {
        volt_read();
        can_send(TOPIC_VOLTAGE_INFO_FILTER);
        flags &= ~MEASUREMENTS_VOLTS_READ_FLAG;
    }
    if (flags & MEASUREMENTS_TEMPS_READ_FLAG) {
        temp_measure_all();
        can_send(TOPIC_TEMPERATURE_INFO_FILTER);
        can_send(TOPIC_STATUS_FILTER);
        flags &= ~MEASUREMENTS_TEMPS_READ_FLAG;
    }
}

void measurements_oc_handler(TIM_HandleTypeDef *htim) {
    uint32_t cnt = __HAL_TIM_GetCounter(htim);
    switch (htim->Channel) {
        case VOLTS_START_CONVERTION_ACTIVE_CHANNEL:
            flags |= MEASUREMENTS_VOLTS_START_CONVERTION_FLAG;
            break;

        case VOLTS_READ_ACTIVE_CHANNEL:
            __HAL_TIM_SetCompare(
                htim,
                VOLTS_START_CONVERTION_CHANNEL,
                cnt + TIM_MS_TO_TICKS(htim, VOLT_MEASURE_INTERVAL - VOLT_MEASURE_TIME));
            __HAL_TIM_SetCompare(htim, VOLTS_READ_CHANNEL, cnt + TIM_MS_TO_TICKS(htim, VOLT_MEASURE_INTERVAL));
            flags |= MEASUREMENTS_VOLTS_READ_FLAG;
            break;

        case TEMPS_READ_ACTIVE_CHANNEL:
            __HAL_TIM_SetCompare(htim, TEMPS_READ_CHANNEL, cnt + TIM_MS_TO_TICKS(htim, TEMP_MEASURE_INTERVAL));
            flags |= MEASUREMENTS_TEMPS_READ_FLAG;
            break;

        default:
            break;
    }
}