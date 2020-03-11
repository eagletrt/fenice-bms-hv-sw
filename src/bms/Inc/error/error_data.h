
#ifndef ERROR_DATA_H
#define ERROR_DATA_H

#include <stdio.h>
#include "error/error_def.h"
#include "error/list.h"
#include "../../../fenice_config.h"

er_node_t *error_voltages[PACK_CELL_COUNT];

er_node_t *error_temperatures[PACK_TEMP_COUNT];

er_node_t *error_total_voltage;
er_node_t *error_max_voltage;
er_node_t *error_min_voltage;

er_node_t *error_avg_temperature;
er_node_t *error_max_temperature;
er_node_t *error_min_temperature;

er_node_t *error_current;

er_node_t *error_ltc[LTC6813_COUNT];

er_node_t *error_can;

er_node_t **error_reference[ERROR_NUM_ERRORS] = {NULL, error_ltc, error_voltages, error_voltages, error_temperatures, &error_current, &error_can};
#endif