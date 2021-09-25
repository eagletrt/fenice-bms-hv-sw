/**
 * @file	pack.c
 * @brief	This file contains the functions to manage the battery pack
 *
 * @date	Apr 11, 2019
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "pack.h"

#include "config.h"
#include "feedback.h"
#include "peripherals/si8900.h"
#include "soc.h"

#include <stdbool.h>
#include <string.h>

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef struct {
    voltage_t bus_voltage;
    voltage_t int_voltage;

    voltage_t voltages[PACK_CELL_COUNT];
    voltage_t max_voltage;
    voltage_t min_voltage;

    temperature_t temperatures[PACK_TEMP_COUNT];
    temperature_t max_temperature;
    temperature_t min_temperature;
    temperature_t mean_temperature;

} cells_t;

cells_t cells;

/**
 * @brief	Initializes the pack
 *
 */
void pack_init() {
    cells.bus_voltage = 0;
    cells.int_voltage = 0;
    //current           = 0;

    for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
        cells.voltages[i] = 0;
    }

    for (size_t i = 0; i < PACK_TEMP_COUNT; i++) {
        cells.temperatures[i] = 0;
    }
}

void pack_update_voltages(UART_HandleTypeDef *huart) {
    voltage_t internal = 0;
    voltage_t bus      = 0;
    voltage_t sum      = 0;

    if (si8900_read_channel(huart, SI8900_AIN0, &internal)) {
        cells.int_voltage = internal;
    }
    HAL_Delay(2);  // TODO: this sucks
    if (si8900_read_channel(huart, SI8900_AIN1, &bus)) {
        cells.bus_voltage = bus;
    }

    for (uint16_t i = 0; i < PACK_CELL_COUNT; i++) {
        sum += cells.voltages[i];
    }

    // Check if difference between readings from the ADC and LTCs is greater than 5V
    if (max(internal, sum) - min(internal, sum) > 5 * 100) {
        error_set(ERROR_INT_VOLTAGE_MISMATCH, 0, HAL_GetTick());
    } else {
        error_reset(ERROR_INT_VOLTAGE_MISMATCH, 0);
    }
    cells.int_voltage = max(internal, sum);  // TODO: is this a good thing?
}

int32_t _mean(uint32_t values[], size_t size) {
    int32_t sum = 0;
    for (uint16_t i = 0; i < size; i++) {
        sum += values[i];
    }
    return sum / size;
}

void pack_update_voltage_stats(voltage_t *total, voltage_t *maxv, voltage_t *minv) {
    uint32_t tot_voltage  = 0;
    voltage_t max_voltage = cells.voltages[0];
    voltage_t min_voltage = UINT16_MAX;

    for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
        voltage_t tmp_voltage = cells.voltages[i];
        tot_voltage += (uint32_t)tmp_voltage;

        max_voltage = max(max_voltage, tmp_voltage);
        min_voltage = min(min_voltage, tmp_voltage);
    }

    *total = (voltage_t)(tot_voltage / 100);
    *maxv  = max_voltage;
    *minv  = min(min_voltage, max_voltage);
}

void pack_update_temperature_stats() {
    uint32_t avg_temperature      = 0;
    temperature_t max_temperature = 0;
    temperature_t min_temperature = UINT8_MAX;

    for (uint16_t i = 0; i < TEMP_SENSOR_COUNT; i++) {
        temperature_t tmp_temperature = cells.temperatures[i];

        avg_temperature += (uint32_t)tmp_temperature;

        max_temperature = max(max_temperature, tmp_temperature);
        min_temperature = min(min_temperature, tmp_temperature);
    }

    cells.mean_temperature = (temperature_t)(avg_temperature / TEMP_SENSOR_COUNT);
    cells.max_temperature  = max_temperature;
    cells.min_temperature  = min(min_temperature, max_temperature);
}

bool pack_set_ts_off() {
    //Switch off airs
    HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_RESET);

    feedback_read(FEEDBACK_TS_OFF_MASK);
    return feedback_check(FEEDBACK_TS_OFF_MASK, FEEDBACK_TS_OFF_VAL, ERROR_FEEDBACK);
}

bool pack_set_pc_start() {
    //switch on AIR-
    HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_SET);

    // Check feedback
    feedback_read(FEEDBACK_TO_PRECHARGE_MASK);
    return feedback_check(FEEDBACK_TO_PRECHARGE_MASK, FEEDBACK_TO_PRECHARGE_VAL, ERROR_FEEDBACK);
}

bool pack_set_precharge_end() {
    //switch on AIR+
    HAL_GPIO_WritePin(AIRP_OFF_GPIO_Port, AIRP_OFF_Pin, GPIO_PIN_RESET);

    // Check feedback
    feedback_read(FEEDBACK_ON_MASK);
    return feedback_check(FEEDBACK_ON_MASK, FEEDBACK_ON_VAL, ERROR_FEEDBACK);
}

voltage_t *pack_get_voltages() {
    return cells.voltages;
}

voltage_t pack_get_max_voltage() {
    return cells.max_voltage;
}

voltage_t pack_get_min_voltage() {
    return cells.min_voltage;
}

voltage_t pack_get_bus_voltage() {
    return cells.bus_voltage;
}

voltage_t pack_get_int_voltage() {
    return cells.int_voltage;
}

temperature_t *pack_get_temperatures() {
    return cells.temperatures;
}

temperature_t pack_get_max_temperature() {
    return cells.max_temperature;
}

temperature_t pack_get_min_temperature() {
    return cells.min_temperature;
}

temperature_t pack_get_mean_temperature() {
    return cells.mean_temperature;
}
