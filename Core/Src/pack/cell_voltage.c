/**
 * @file cell_voltage.c
 * @brief Functions to manage cells voltages in the pack
 *
 * @date Apr 11, 2019
 * 
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "pack/cell_voltage.h"
#include "bms-monitor/volt/volt.h"

static cell_voltage_t voltages[CELLBOARD_COUNT] = { 0 };


HAL_StatusTypeDef cell_voltage_measure(voltage_t volts[CELL_COUNT]) {
    volt_start_measure(&SPI_MONITOR,
        VOLT_ADC_MODE_NORMAL,
        DCP_DISABLED,
        CELL_CH_ALL,
        MONITOR_SPI_CS_GPIO_Port,
        MONITOR_SPI_CS_Pin);
    
    // TODO: Check, test and fix volt_read
    voltage_t aux[CELL_COUNT] = { 0 };
    HAL_StatusTypeDef status = volt_read(&SPI_MONITOR,
        aux,
        MONITOR_SPI_CS_GPIO_Port,
        MONITOR_SPI_CS_Pin);

    if (status == HAL_OK) {
        // Save voltages internally
        for (size_t i = 0; i < CELLBOARD_COUNT; i++) {
            voltage_t min = UINT16_MAX, max = 0;
            size_t min_i = -1, max_i = -1;
            size_t sum = 0;
            for (size_t j = 0; j < LTC_CELL_COUNT; j++) {
                size_t index = i * LTC_CELL_COUNT + j;
                if (aux[index] < min) {
                    min = aux[index];
                    min_i = index;
                }
                if (aux[index] > max) {
                    max = aux[index];
                    max_i = index;
                }
                sum += aux[index];
            }
            voltages[i].min = min;
            voltages[i].max = max;
            voltages[i].sum = sum;

            voltages[i].min_index = min_i;
            voltages[i].max_index = max_i;
        }

        // Copy voltages in the volts array if not NULL
        if (volts != NULL) {
            for (size_t i = 0; i < CELL_COUNT; i++)
                volts[i] = aux[i];
        }
    }
    return status;
}

voltage_t cell_voltage_get_min(size_t * index) {
    voltage_t min = UINT16_MAX;
    size_t min_i = -1;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++) {
        if (voltages[i].min < min) {
            min = voltages[i].min;
            min_i = voltages[i].min_index;
        }
    }
    if (index != NULL)
        *index = min_i;
    return min;
}
voltage_t cell_voltage_get_max(size_t * index) {
    voltage_t max = 0;
    size_t max_i = -1;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++) {
        if (voltages[i].max > max) {
            max = voltages[i].max;
            max_i = voltages[i].max_index;
        }
    }
    if (index != NULL)
        *index = max_i;
    return max;
}
size_t cell_voltage_get_sum() {
    size_t sum = 0;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        sum += voltages[i].sum;
    return sum;
}
float cell_voltage_get_avg() {
    float sum = 0;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        sum += voltages[i].sum / (float)LTC_CELL_COUNT;
    return sum / (float)CELLBOARD_COUNT;
}
