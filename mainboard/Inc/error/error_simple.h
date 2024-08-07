#ifndef ERROR_SIMPLE_H
#define ERROR_SIMPLE_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define ERROR_SIMPLE_COUNTER_THRESHOLD (10U)
#define ERROR_SIMPLE_DUMP_SIZE         (50U)

typedef enum {
    ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE,
    ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE,
    ERROR_GROUP_ERROR_CELL_UNDER_TEMPERATURE,
    ERROR_GROUP_ERROR_CELL_OVER_TEMPERATURE,
    ERROR_GROUP_ERROR_OVER_CURRENT,
    ERROR_GROUP_ERROR_CAN,
    ERROR_GROUP_ERROR_INT_VOLTAGE_MISMATCH,
    ERROR_GROUP_ERROR_CELLBOARD_COMM,
    ERROR_GROUP_ERROR_CELLBOARD_INTERNAL,
    ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED,
    ERROR_GROUP_ERROR_FANS_DISCONNECTED,
    ERROR_GROUP_ERROR_FEEDBACK,
    ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY,
    ERROR_GROUP_ERROR_EEPROM_COMM,
    ERROR_GROUP_ERROR_EEPROM_WRITE,
    N_ERROR_GROUPS
} error_simple_groups_t;

typedef struct {
    error_simple_groups_t group;
    size_t instance;
} error_simple_dump_element_t;

#define ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE_N_INSTANCES     (1U)
#define ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE_N_INSTANCES      (1U)
#define ERROR_GROUP_ERROR_CELL_UNDER_TEMPERATURE_N_INSTANCES (1U)
#define ERROR_GROUP_ERROR_CELL_OVER_TEMPERATURE_N_INSTANCES  (1U)
#define ERROR_GROUP_ERROR_OVER_CURRENT_N_INSTANCES           (1U)
#define ERROR_GROUP_ERROR_CAN_N_INSTANCES                    (2U)
#define ERROR_GROUP_ERROR_INT_VOLTAGE_MISMATCH_N_INSTANCES   (1U)
#define ERROR_GROUP_ERROR_CELLBOARD_COMM_N_INSTANCES         (6U)
#define ERROR_GROUP_ERROR_CELLBOARD_INTERNAL_N_INSTANCES     (6U)
#define ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED_N_INSTANCES (3U)
#define ERROR_GROUP_ERROR_FANS_DISCONNECTED_N_INSTANCES      (1U)
#define ERROR_GROUP_ERROR_FEEDBACK_N_INSTANCES               (20U)
#define ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY_N_INSTANCES     (20U)
#define ERROR_GROUP_ERROR_EEPROM_COMM_N_INSTANCES            (1U)
#define ERROR_GROUP_ERROR_EEPROM_WRITE_N_INSTANCES           (1U)

int error_simple_set(error_simple_groups_t group, size_t instance);
int error_simple_reset(error_simple_groups_t group, size_t instance);
int error_simple_routine(void);
size_t get_expired_errors(void);

extern error_simple_dump_element_t error_simple_dump[ERROR_SIMPLE_DUMP_SIZE];

size_t _error_simple_from_group_and_instance_to_index(error_simple_groups_t group, size_t instance);

#endif  // ERROR_SIMPLE_H
