#include "error_simple.h"
// #include "/home/gmazzucchi/ssd/eagle/old-hv/fenice-bms-hv-sw/mainboard/Inc/error/error_simple.h"

const static size_t error_instances[N_ERROR_GROUPS] = {
    [ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE]     = ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE_N_INSTANCES,
    [ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE]      = ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE_N_INSTANCES,
    [ERROR_GROUP_ERROR_CELL_UNDER_TEMPERATURE] = ERROR_GROUP_ERROR_CELL_UNDER_TEMPERATURE_N_INSTANCES,
    [ERROR_GROUP_ERROR_CELL_OVER_TEMPERATURE]  = ERROR_GROUP_ERROR_CELL_OVER_TEMPERATURE_N_INSTANCES,
    [ERROR_GROUP_ERROR_OVER_CURRENT]           = ERROR_GROUP_ERROR_OVER_CURRENT_N_INSTANCES,
    [ERROR_GROUP_ERROR_CAN]                    = ERROR_GROUP_ERROR_CAN_N_INSTANCES,
    [ERROR_GROUP_ERROR_INT_VOLTAGE_MISMATCH]   = ERROR_GROUP_ERROR_INT_VOLTAGE_MISMATCH_N_INSTANCES,
    [ERROR_GROUP_ERROR_CELLBOARD_COMM]         = ERROR_GROUP_ERROR_CELLBOARD_COMM_N_INSTANCES,
    [ERROR_GROUP_ERROR_CELLBOARD_INTERNAL]     = ERROR_GROUP_ERROR_CELLBOARD_INTERNAL_N_INSTANCES,
    [ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED] = ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED_N_INSTANCES,
    [ERROR_GROUP_ERROR_FANS_DISCONNECTED]      = ERROR_GROUP_ERROR_FANS_DISCONNECTED_N_INSTANCES,
    [ERROR_GROUP_ERROR_FEEDBACK]               = ERROR_GROUP_ERROR_FEEDBACK_N_INSTANCES,
    [ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY]     = ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY_N_INSTANCES,
    [ERROR_GROUP_ERROR_EEPROM_COMM]            = ERROR_GROUP_ERROR_EEPROM_COMM_N_INSTANCES,
    [ERROR_GROUP_ERROR_EEPROM_WRITE]           = ERROR_GROUP_ERROR_EEPROM_WRITE_N_INSTANCES,
};

#define ERROR_SIMPLE_STATE_SIZE                                                                                   \
    (ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE_N_INSTANCES + ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE_N_INSTANCES +         \
     ERROR_GROUP_ERROR_CELL_UNDER_TEMPERATURE_N_INSTANCES + ERROR_GROUP_ERROR_CELL_OVER_TEMPERATURE_N_INSTANCES + \
     ERROR_GROUP_ERROR_OVER_CURRENT_N_INSTANCES + ERROR_GROUP_ERROR_CAN_N_INSTANCES +                             \
     ERROR_GROUP_ERROR_INT_VOLTAGE_MISMATCH_N_INSTANCES + ERROR_GROUP_ERROR_CELLBOARD_COMM_N_INSTANCES +          \
     ERROR_GROUP_ERROR_CELLBOARD_INTERNAL_N_INSTANCES + ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED_N_INSTANCES +    \
     ERROR_GROUP_ERROR_FANS_DISCONNECTED_N_INSTANCES + ERROR_GROUP_ERROR_FEEDBACK_N_INSTANCES +                   \
     ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY_N_INSTANCES + ERROR_GROUP_ERROR_EEPROM_COMM_N_INSTANCES +               \
     ERROR_GROUP_ERROR_EEPROM_WRITE_N_INSTANCES)

static uint8_t error_simple_state[ERROR_SIMPLE_STATE_SIZE];
static size_t error_expired = 0;
error_simple_dump_element_t error_simple_dump[ERROR_SIMPLE_DUMP_SIZE] = {0};

size_t _error_simple_from_group_and_instance_to_index(error_simple_groups_t group, size_t instance) {
    uint32_t retidx = 0;
    for (size_t i = 0; i < group && i < N_ERROR_GROUPS; i++) {
        retidx += error_instances[i];
    }
    return retidx + instance;
}

/**
 * @brief From the index in the array error_simple_state to the error group and instance number
 * 
 * @param index the index in the array error_simple_state
 * @param group the error group
 * @return instance number
 */
size_t _error_simple_from_index_to_group_and_instance(size_t index, error_simple_groups_t *group) {
    int sindex = index;
    for (size_t i = 0; sindex >= error_instances[i] && i < N_ERROR_GROUPS; i++) {
        *group = i;
        sindex -= error_instances[i];
    }
    if (*group != 0) (*group)++;
    return sindex;
}

void _add_error_to_dump(size_t index) {
    error_simple_groups_t group;
    size_t instance                           = _error_simple_from_index_to_group_and_instance(index, &group);
    error_simple_dump[error_expired].group    = group;
    error_simple_dump[error_expired].instance = instance;
}

int error_simple_set(error_simple_groups_t group, size_t instance) {
    if (group >= N_ERROR_GROUPS || instance >= error_instances[group]) {
        return -1;
    }
    (error_simple_state[_error_simple_from_group_and_instance_to_index(group, instance)])++;
    return 0;
}

int error_simple_reset(error_simple_groups_t group, size_t instance) {
    if (group >= N_ERROR_GROUPS || instance >= error_instances[group]) {
        return -1;
    }
    error_simple_state[_error_simple_from_group_and_instance_to_index(group, instance)] = 0;
    return 0;
}

int error_simple_routine(void) {
    if (error_expired > 0) {
        return error_expired;
    }
    for (size_t i = 0; i < ERROR_SIMPLE_STATE_SIZE; i++) {
        if (error_simple_state[i] >= ERROR_SIMPLE_COUNTER_THRESHOLD) {
            _add_error_to_dump(i);
            error_expired++;
        }
    }
    return error_expired;
}

size_t get_expired_errors(void) {
    return error_expired;
}

