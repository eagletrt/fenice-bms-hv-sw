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
#include "main.h"
#include "peripherals/si8900.h"
#include "soc.h"

#include <stdbool.h>
#include <string.h>

#define CURRENT_ARRAY_LENGTH 512

#define SOC_WRITE_INTERVAL 5000
#define SOC_VERSION        0x01
#define SOC_ADDR           0x55

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

typedef struct {
    uint8_t version;

    double total_joule;
    double charge_joule;
} soc_params;
soc_params soc_params_default = {SOC_VERSION, 0, 0};

uint32_t current_50[CURRENT_ARRAY_LENGTH];   // TODO: move to pck_data and update DMA and generate getter no setter
uint32_t current_300[CURRENT_ARRAY_LENGTH];  // TODO: move to pck_data and update DMA and generate getter no setter

soc_t soc_total;
soc_t soc_last_charge;
config_t soc_config;
uint32_t soc_timer = 0;

cells_t cells;
current_t current;

/**
 * @brief	Initializes the pack
 *
 */
void pack_init() {
    cells.bus_voltage = 0;
    cells.int_voltage = 0;
    current           = 0;

    for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
        cells.voltages[i] = 0;
    }

    for (size_t i = 0; i < PACK_TEMP_COUNT; i++) {
        cells.temperatures[i] = 0;
    }

    soc_init(&soc_total);
    soc_init(&soc_last_charge);
    config_init(&soc_config, SOC_ADDR, &soc_params_default, sizeof(soc_params));

    soc_load(soc_total, ((soc_params *)config_get(soc_config))->total_joule, HAL_GetTick());
    soc_load(soc_last_charge, ((soc_params *)config_get(soc_config))->charge_joule, HAL_GetTick());

    // LTC6813 GPIO configuration
    GPIO_CONFIG = GPIO_I2C_MODE;
}

void pack_update_voltages(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart) {
    _ltc6813_adcv(hspi, 0);  // TODO: remove this?

    ltc6813_read_voltages(hspi, cells.voltages);

    voltage_t internal = 0;
    voltage_t bus      = 0;
    if (si8900_read_channel(huart, SI8900_AIN0, &internal)) {
        cells.int_voltage = internal;
    }
    HAL_Delay(0);  // TODO: this sucks
    if (si8900_read_channel(huart, SI8900_AIN1, &bus)) {
        cells.bus_voltage = bus;
    }

    voltage_t sum = 0;
    pack_update_voltage_stats(&sum, &(cells.max_voltage), &(cells.min_voltage));

    // Check if difference between readings from the ADC and LTCs is greater than 5V
    if (max(internal, sum) - min(internal, sum) > 5 * 100) {
        error_set(ERROR_INT_VOLTAGE_MISMATCH, 0, HAL_GetTick());
    } else {
        error_unset(ERROR_INT_VOLTAGE_MISMATCH, 0);
    }
    cells.int_voltage = max(internal, sum);  // TODO: is this a good thing?
}

void pack_update_temperatures(SPI_HandleTypeDef *hspi) {
    uint8_t max[LTC6813_COUNT * 2];
    uint8_t min[LTC6813_COUNT * 2];

    ltc6813_read_temperatures(hspi, max, min);

    temperature_t avg_temp = 0;
    temperature_t max_temp = 0;
    temperature_t min_temp = UINT8_MAX;

    for (uint8_t i = 0; i < LTC6813_COUNT; i++) {
        avg_temp += max[i * 2] + max[i * 2 + 1];
        avg_temp += min[i * 2] + min[i * 2 + 1];

        max_temp = max(max[i * 2], max_temp);
        min_temp = min(min[i * 2], min_temp);
    }

    cells.mean_temperature = ((float)avg_temp / (LTC6813_COUNT * 4)) * 10;
    cells.max_temperature  = max_temp;
    cells.min_temperature  = min_temp;
}

void pack_update_all_temperatures(SPI_HandleTypeDef *hspi) {
    ltc6813_read_all_temps(hspi, cells.temperatures);
}

int32_t _mean(uint32_t values[], size_t size) {
    int32_t sum = 0;
    for (uint16_t i = 0; i < size; i++) {
        sum += values[i];
    }
    return sum / size;
}

//TODO: redo DMA and then rewrite the function
void pack_update_current() {
    current_t current50  = 0;
    current_t current300 = 0;

    int32_t adc50  = _mean(current_50, CURRENT_ARRAY_LENGTH);
    int32_t adc300 = _mean(current_300, CURRENT_ARRAY_LENGTH);

    // We calculate the input voltage
    float in_volt = (((float)adc50 * 3.3) / 4096);
    current50     = ((in_volt - S160_OFFSET) / S160_50A_SENS) * 10;

    in_volt    = (((float)adc300 * 3.3) / 4096);
    current300 = ((in_volt - S160_OFFSET) / S160_300A_SENS) * 10;

    if (current300 >= 50) {
        current = current300;
    } else {
        current = current50;
    }

    soc_params params = *(soc_params *)config_get(soc_config);

    soc_sample_current(soc_total, current, pack_get_int_voltage(), HAL_GetTick());
    soc_sample_current(soc_last_charge, current, pack_get_int_voltage(), HAL_GetTick());

    params.charge_joule = soc_get_joule(soc_last_charge);

    params.total_joule = soc_get_joule(soc_total);
    config_set(soc_config, &params);

    if (HAL_GetTick() - soc_timer >= SOC_WRITE_INTERVAL) {
        soc_timer = HAL_GetTick();
        config_write(soc_config);
    }

    error_toggle_check(current > PACK_MAX_CURRENT, ERROR_OVER_CURRENT, 0);
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

void pack_reset_soc() {
    soc_reset_count(soc_last_charge, HAL_GetTick());
}

double pack_get_soc() {
    return soc_get_wh(soc_last_charge) / ((double)PACK_ENERGY_NOMINAL / 10) * 100;
}

double pack_get_energy_total() {
    return soc_get_wh(soc_total);
}

double pack_get_energy_last_charge() {
    return soc_get_wh(soc_last_charge);
}

bool pack_set_ts_off() {
    //Switch off airs
    HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_RESET);

    feedback_read(FEEDBACK_TS_OFF_MASK);
    return feedback_check(FEEDBACK_TS_OFF_MASK, FEEDBACK_TS_OFF_VAL, ERROR_FEEDBACK_HARD);
}

bool pack_set_pc_start() {
    //switch on AIR-
    HAL_GPIO_WritePin(TS_ON_GPIO_Port, TS_ON_Pin, GPIO_PIN_SET);

    // Check feedback
    feedback_read(FEEDBACK_TO_PRECHARGE_MASK);
    return feedback_check(FEEDBACK_TO_PRECHARGE_MASK, FEEDBACK_TO_PRECHARGE_VAL, ERROR_FEEDBACK_HARD);
}

bool pack_set_precharge_end() {
    //switch on AIR+
    HAL_GPIO_WritePin(PC_ENDED_GPIO_Port, PC_ENDED_Pin, GPIO_PIN_SET);
    HAL_Delay(10);  // non so quanto debba essere. In chimera con 1ms si triggerava però boh
    HAL_GPIO_WritePin(PC_ENDED_GPIO_Port, PC_ENDED_Pin, GPIO_PIN_RESET);

    // Check feedback
    feedback_read(FEEDBACK_ON_MASK);
    return feedback_check(FEEDBACK_ON_MASK, FEEDBACK_ON_VAL, ERROR_FEEDBACK_HARD);
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

current_t pack_get_current() {
    return current;
}

int16_t pack_get_power() {
    // TODO: fix units
    return current * cells.int_voltage;
}
