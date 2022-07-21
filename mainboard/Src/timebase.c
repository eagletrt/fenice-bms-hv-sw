#include "timebase.h"

#include "adc124s021.h"
#include "bms_fsm.h"
#include "can_comm.h"
#include "cli_bms.h"
#include "fans_buzzer.h"
#include "imd.h"
#include "pwm.h"
#include "soc.h"
#include "spi.h"
#include "temperature.h"
#include "voltage.h"

#define SEND_CAN_CAR_MSG_ATTEMPTS 2
#define SEND_CAN_CAR_MSG(id)                                                  \
    do {                                                                      \
        uint8_t i = 0;                                                        \
        while (i++ < SEND_CAN_CAR_MSG_ATTEMPTS && can_car_send(id) != HAL_OK) \
            ;                                                                 \
    } while (0)

timebase_flags_t flags;
uint32_t repetition_counter;

void timebase_init() {
    __HAL_TIM_SetAutoreload(&HTIM_TIMEBASE, TIM_MS_TO_TICKS(&HTIM_TIMEBASE, _BASE_INTERVAL));
    __HAL_TIM_CLEAR_IT(&HTIM_TIMEBASE, TIM_IT_UPDATE);
    HAL_TIM_Base_Start_IT(&HTIM_TIMEBASE);
    flags              = 0;
    repetition_counter = 0;
}

void timebase_check_flags() {
    if (flags & _10MS_INTERVAL_FLAG) {
        SEND_CAN_CAR_MSG(primary_ID_TS_STATUS);
        if (error_count() > 0) {
            SEND_CAN_CAR_MSG(primary_ID_HV_ERRORS);
        }

        if (imd_get_state() != IMD_NORMAL) {
            SEND_CAN_CAR_MSG(primary_ID_HV_IMD_STATUS);
        }
        flags &= ~_10MS_INTERVAL_FLAG;
    }
    if (flags & _50MS_INTERVAL_FLAG) {
        timebase_voltage_current_soc();
        voltage_check_errors();
        current_check_errors();
        SEND_CAN_CAR_MSG(primary_ID_HV_VOLTAGE);
        SEND_CAN_CAR_MSG(primary_ID_HV_CURRENT);
        flags &= ~_50MS_INTERVAL_FLAG;
    }
    if (flags & _100MS_INTERVAL_FLAG) {
        can_cellboards_check();
        temperature_check_errors();
        SEND_CAN_CAR_MSG(primary_ID_HV_TEMP);
        SEND_CAN_CAR_MSG(primary_ID_SHUTDOWN_STATUS);
        SEND_CAN_CAR_MSG(primary_ID_HV_FEEDBACKS_STATUS);

        if (imd_get_state() == IMD_NORMAL) {
            SEND_CAN_CAR_MSG(primary_ID_HV_IMD_STATUS);
        }
        flags &= ~_100MS_INTERVAL_FLAG;
    }
    if (flags & _500MS_INTERVAL_FLAG) {
        fans_loop(temperature_get_max() / 2.56 - 20);

        SEND_CAN_CAR_MSG(primary_ID_HV_CELLS_TEMP);
        SEND_CAN_CAR_MSG(primary_ID_HV_CELLS_VOLTAGE);
        SEND_CAN_CAR_MSG(primary_ID_HV_CELL_BALANCING_STATUS);
        SEND_CAN_CAR_MSG(primary_ID_HV_CAN_FORWARD_STATUS);
        SEND_CAN_CAR_MSG(primary_ID_HV_FANS_OVERRIDE_STATUS);
        flags &= ~_500MS_INTERVAL_FLAG;
    }
    if (flags & _1S_INTERVAL_FLAG) {
        SEND_CAN_CAR_MSG(primary_ID_HV_VERSION);
        flags &= ~_1S_INTERVAL_FLAG;
    }
    if (flags & _5S_INTERVAL_FLAG) {
        flags &= ~_5S_INTERVAL_FLAG;
    }
}

void timebase_voltage_current_soc() {
    ADC124S021_CH chs[3]     = {ADC124_BUS_CHANNEL, ADC124_INTERNAL_CHANNEL, ADC124_SHUNT_CHANNEL};
    float adc124_timebase[3] = {0};
    if (adc124s021_read_channels(&SPI_ADC124S, chs, 3, adc124_timebase)) {
        voltage_measure(adc124_timebase);
        current_read(adc124_timebase[2]);
    }
    soc_sample_energy(HAL_GetTick());
}

void _timebase_handle_tim_elapsed_handler(TIM_HandleTypeDef *htim) {
    ++repetition_counter;

    if (!(repetition_counter * _BASE_INTERVAL % _10MS_INTERVAL))
        flags |= _10MS_INTERVAL_FLAG;
    if (!(repetition_counter * _BASE_INTERVAL % _50MS_INTERVAL))
        flags |= _50MS_INTERVAL_FLAG;
    if (!(repetition_counter * _BASE_INTERVAL % _100MS_INTERVAL))
        flags |= _100MS_INTERVAL_FLAG;
    if (!(repetition_counter * _BASE_INTERVAL % _500MS_INTERVAL))
        flags |= _500MS_INTERVAL_FLAG;
    if (!(repetition_counter * _BASE_INTERVAL % _1S_INTERVAL))
        flags |= _1S_INTERVAL_FLAG;
    if (!(repetition_counter * _BASE_INTERVAL % _5S_INTERVAL))
        flags |= _5S_INTERVAL_FLAG;
}