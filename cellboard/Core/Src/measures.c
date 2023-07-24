/**
 * @file measurements.h
 * @brief Functions to ensure that all the measurements and error checks
 * happens periodically for a given interval
 * 
 * @date Jul 24, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "measures.h"

#include <stdint.h>
#include <stdbool.h>

#include "tim.h"
#include "measures.h"
#include "cellboard_config.h"
#include "volt.h"
#include "temp.h"
#include "bms/bms_network.h"

#define _MEASURE_CHECK_INTERVAL(interval) (((counter) % (interval)) == 0) // Check if a given interval is passed

uint32_t counter = 0; // Each timer interrupt it increments by 1
bool flags_checked = false;
bool is_volt_measure_new = false; 
bool is_open_wire_measure_new = false;
uint8_t open_wire_check_status = 0;

void measures_init() {
    counter = 0;
    flags_checked = false;
    open_wire_check_status = 0;

    // Set timer
    __HAL_TIM_SET_COMPARE(&HTIM_MEASURES, TIM_CHANNEL_1, TIM_MS_TO_TICKS(&HTIM_MEASURES, MEASURE_BASE_INTERVAL_MS));
    __HAL_TIM_CLEAR_IT(&HTIM_MEASURES, TIM_CHANNEL_1);
    HAL_TIM_OC_Start_IT(&HTIM_MEASURES, TIM_CHANNEL_1);
}

void measures_check_flags() {
    if (flags_checked)
        return;
    flags_checked = false;

    // 10 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_10MS)) {
        // Read voltages
        if (is_volt_measure_new) {
            volt_read();
            can_send(BMS_VOLTAGES_FRAME_ID);
            can_send(BMS_VOLTAGES_INFO_FRAME_ID);
        }
        else
            volt_start_measure();

        // Check for open wire
        if (is_open_wire_measure_new) {
            volt_read_open_wire(open_wire_check_status);
            if (open_wire_check_status == 4)
                volt_open_wire_check();
        }
        else
            volt_start_open_wire_check(open_wire_check_status);

        open_wire_check_status = (open_wire_check_status + 1) % 5;
    }
    // 200 ms interval
    if (_MEASURE_CHECK_INTERVAL(MEASURE_INTERVAL_200MS)) {
        // Measure temperatures
        temp_measure_all();

        // Send info via CAN
        can_send(BMS_TEMPERATURES_FRAME_ID);
        can_send(BMS_TEMPERATURES_INFO_FRAME_ID);
        can_send(BMS_BOARD_STATUS_FRAME_ID);
    }
}

void _measures_handle_tim_oc_irq(TIM_HandleTypeDef * htim) {
    uint32_t cnt = __HAL_TIM_GetCounter(htim);

    switch(htim->Channel) {
        case HAL_TIM_ACTIVE_CHANNEL_1:
            __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (cnt + TIM_MS_TO_TICKS(htim, MEASURE_BASE_INTERVAL_MS)));
            if (counter == UINT32_MAX)
                counter = 0;
            ++counter;
            flags_checked = false;
            break;
        default:
            break;
    }
}