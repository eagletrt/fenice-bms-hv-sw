/**
 * @file temperature.c
 * @brief Functions to manage all cell temperatures
 *
 * @date Apr 11, 2019
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Federico Carbone [federico.carbone@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "pack/temperature.h"
#include "main.h"

static cell_temperature_t temperatures[CELLBOARD_COUNT] = { 0 };


HAL_StatusTypeDef cell_temperature_measure(temperature_t temps[PACK_TEMP_COUNT]) {
    temp_start_measure(&SPI_MONITOR,
        VOLT_ADC_MODE_NORMAL,
        CHG_GPIO_ALL,
        MONITOR_SPI_CS_GPIO_Port,
        MONITOR_SPI_CS_Pin);
    
    // TODO: Check, test and fix temp_read
    temperature_t aux[PACK_TEMP_COUNT] = { 0 };
    HAL_StatusTypeDef status = temp_read(&SPI_MONITOR,
        aux,
        MONITOR_SPI_CS_GPIO_Port,
        MONITOR_SPI_CS_Pin);

    if (status == HAL_OK) {
        // Save temperatures internally
        for (size_t i = 0; i < CELLBOARD_COUNT; i++) {
            temperature_t min = UINT8_MAX, max = 0;
            size_t sum = 0;
            for (size_t j = 0; j < CELLBOARD_CELL_COUNT; j++) {
                size_t index = i * CELLBOARD_CELL_COUNT + j;
                min = MIN(min, aux[index]);
                max = MAX(max, aux[index]);
                sum += aux[index];
            }
            temperatures[i].min = min;
            temperatures[i].max = max;
            temperatures[i].sum = sum;
        }

        // Copy values in the temps array if not NULL
        if (temps != NULL) {
            for (size_t i = 0; i < PACK_TEMP_COUNT; i++)
                temps[i] = aux[i];
        }
    }
    return status;
}

temperature_t temperature_get_min() {
    temperature_t min = UINT8_MAX;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        min = MIN(min, temperatures[i].min);
    return min;
}
temperature_t temperature_get_max() {
    temperature_t max = 0;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        max = MAX(max, temperatures[i].max);
    return max;
}
size_t temperature_get_sum() {
    size_t sum = 0;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        sum += temperatures[i].sum;
    return sum;
}
float temperature_get_avg() {
    float sum = 0;
    for (size_t i = 0; i < CELLBOARD_COUNT; i++)
        sum += temperatures[i].sum / (float)CELLBOARD_CELL_COUNT;
    return sum / (float)CELLBOARD_COUNT;
}
