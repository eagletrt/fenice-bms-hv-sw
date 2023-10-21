/**
 * @file current.c
 * @brief Functions that handle current measurement
 *
 * @date Sep 24, 2021
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */
#include "pack/current.h"

#include <math.h>
#include <stdbool.h>

#include "stm32f4xx_hal.h"
#include "mainboard_config.h"

#define MEASURE_SAMPLE_SIZE 128

#define CURRENT_SHUNT_THRESHOLD 4.f // V
#define CURRENT_HALL_THRESHOLD 34.f // V


typedef struct {
    uint16_t data[MEASURE_SAMPLE_SIZE];
    size_t index;
    voltage_t zero;
    voltage_t value;
    voltage_t weight;
} CurrentHandler;

CurrentHandler shunt;
CurrentHandler hall_50;
CurrentHandler hall_300;

// Total number of readings from the start (until it reaches MEASURE_SAMPLE_SIZE)
uint8_t readings = 0;


void current_start_measure() {
    HAL_ADC_Start_DMA(&ADC_HALL50, (uint32_t *)(&hall_50.value), MEASURE_SAMPLE_SIZE);
    HAL_ADC_Start_DMA(&ADC_HALL300, (uint32_t *)(&hall_300.value), MEASURE_SAMPLE_SIZE);
}

void current_read(voltage_t shunt_voltage) {
    if (readings < MEASURE_SAMPLE_SIZE)
        ++readings;
    else {
        shunt.data[shunt.index] = shunt_voltage;
        shunt.index = (shunt.index + 1) % MEASURE_SAMPLE_SIZE;
    }

    voltage_t shunt_volt = (readings < MEASURE_SAMPLE_SIZE) ? shunt_voltage : 0;
    voltage_t hall_50_volt = 0;
    voltage_t hall_300_volt = 0;

    // Average sample values
    for (size_t i = 0; i < MEASURE_SAMPLE_SIZE; i++) {
        if (readings >= MEASURE_SAMPLE_SIZE)
            shunt_volt += shunt.data[i];
        hall_50_volt += hall_50.data[i];
        hall_300_volt += hall_300.data[i];
    }
    // Update values
    shunt.value = (readings < MEASURE_SAMPLE_SIZE ? shunt_volt : shunt_volt / MEASURE_SAMPLE_SIZE) * (3.3f / 4095.f);
    hall_50.value = (hall_50_volt / MEASURE_SAMPLE_SIZE) * (3.3f / 4095.f);
    hall_300.value = (hall_300_volt / MEASURE_SAMPLE_SIZE) * (3.3f / 4095.f);

    // Check for over-current
    ERROR_TOGGLE_CHECK_STR(fabsf(current_get_current()) > PACK_MAX_CURRENT, ERROR_OVER_CURRENT, error_mainboard_instance);
}

void current_zero() {
    voltage_t shunt_volt = (readings < MEASURE_SAMPLE_SIZE) ? shunt.data[shunt.index] : 0;
    voltage_t hall_50_volt = 0;
    voltage_t hall_300_volt = 0;

    // Average sample values
    for (size_t i = 0; i < MEASURE_SAMPLE_SIZE; ++i) {
        if (readings >= MEASURE_SAMPLE_SIZE)
            shunt_volt += shunt.data[i];
        hall_50_volt += hall_50.data[i];
        hall_300_volt += hall_300.data[i];
    }
    // Update zeros
    shunt.zero = (readings < MEASURE_SAMPLE_SIZE ? shunt_volt : shunt_volt / MEASURE_SAMPLE_SIZE) * (3.3f / 4095.f);
    hall_50.zero = (hall_50_volt / MEASURE_SAMPLE_SIZE) * (3.3f / 4095.f);
    hall_300.zero = (hall_300_volt / MEASURE_SAMPLE_SIZE) * (3.3f / 4095.f);
}

current_t current_get_current() {
    current_t shunt_cur = CURRENT_SHUNT_TO_CURRENT(shunt.value - shunt.zero);
    current_t hall_50_cur = CURRENT_HALL_50_TO_CURRENT(hall_50.value - hall_50.zero);
    current_t hall_300_cur = CURRENT_HALL_300_TO_CURRENT(hall_300.value - hall_300.zero);

    current_t hall_abs = fabs(hall_50_cur);

    // TODO: Weighted average
    // Return current read from the correct sensor
    if (fabsf(shunt_cur) < CURRENT_SHUNT_THRESHOLD && hall_abs < CURRENT_SHUNT_THRESHOLD)
        return shunt_cur;
    if (hall_abs < CURRENT_HALL_THRESHOLD && fabsf(hall_300_cur) < CURRENT_HALL_THRESHOLD)
        return hall_50_cur;
    return hall_300_cur;
}

current_t current_get_current_from_sensor(uint8_t sensor) {
    switch (sensor)
    {
        case CURRENT_SENSOR_SHUNT:
            return CURRENT_SHUNT_TO_CURRENT(shunt.value - shunt.zero);
        case CURRENT_SENSOR_50:
            return CURRENT_HALL_50_TO_CURRENT(hall_50.value - hall_50.zero);
        case CURRENT_SENSOR_300:
            return CURRENT_HALL_300_TO_CURRENT(hall_300.value - hall_300.zero);

        default:
            return 0;
    }
}

void current_check_errors() {
    ERROR_TOGGLE_CHECK_STR(fabsf(CURRENT_HALL_300_TO_CURRENT(hall_300.value - hall_300.zero)) > PACK_MAX_CURRENT, ERROR_OVER_CURRENT, error_mainboard_instance);
    
    // Hall effect sensor disconnected
    ERROR_TOGGLE_CHECK_STR(hall_300.value < CURRENT_SENSOR_DISCONNECTED_THRESHOLD, ERROR_CONNECTOR_DISCONNECTED, error_current_instance);
}
