/*******************************************************************************
 * Critical error handler library generator
 * Generated by error_gen ruby gem, for more information see:
 * https://github.com/eagletrt/micro-utils/tree/master/error-handler-generator
 *
 * Error_gen version 1.3.0
 * Generation date: 2024-03-24 12:45:37 +0100
 * Generation from: 
 * The error handler contains:
 *     - 15 error groups
 *     - 66 total error instances
 ******************************************************************************/

#include "error-handler.h"

#include <string.h>

#include "ring-buffer.h"
#include "min-heap.h"

// Ring buffer maximum number of elements
#define ERROR_BUFFER_SIZE 16

/**
 * @brief Error data type definition needed to manage the errors
 *
 * @param group The group to which the error belongs
 * @param instance The instance of the error
 * @param timestamp The current time (in ms)
 * @param op A pointer to the operation that needs to be executed
 */
typedef struct _ErrorData {
    ErrorGroup group;
    ErrorInstance instance;
    uint32_t timestamp;
    void (* op)(struct _ErrorData);
} ErrorData;

// Total number of instances for each group
static const uint16_t instances[] = {
    [ERROR_CELL_UNDER_VOLTAGE] = 1,
    [ERROR_CELL_OVER_VOLTAGE] = 1,
    [ERROR_CELL_UNDER_TEMPERATURE] = 1,
    [ERROR_CELL_OVER_TEMPERATURE] = 1,
    [ERROR_OVER_CURRENT] = 1,
    [ERROR_CAN] = 2,
    [ERROR_INT_VOLTAGE_MISMATCH] = 1,
    [ERROR_CELLBOARD_COMM] = 6,
    [ERROR_CELLBOARD_INTERNAL] = 6,
    [ERROR_CONNECTOR_DISCONNECTED] = 3,
    [ERROR_FANS_DISCONNECTED] = 1,
    [ERROR_FEEDBACK] = 20,
    [ERROR_FEEDBACK_CIRCUITRY] = 20,
    [ERROR_EEPROM_COMM] = 1,
    [ERROR_EEPROM_WRITE] = 1
};
// Error timeout for each group
static const uint16_t timeouts[] = {
    [ERROR_CELL_UNDER_VOLTAGE] = 50,
    [ERROR_CELL_OVER_VOLTAGE] = 50,
    [ERROR_CELL_UNDER_TEMPERATURE] = 100,
    [ERROR_CELL_OVER_TEMPERATURE] = 100,
    [ERROR_OVER_CURRENT] = 50,
    [ERROR_CAN] = 250,
    [ERROR_INT_VOLTAGE_MISMATCH] = 50,
    [ERROR_CELLBOARD_COMM] = 100,
    [ERROR_CELLBOARD_INTERNAL] = 100,
    [ERROR_CONNECTOR_DISCONNECTED] = 50,
    [ERROR_FANS_DISCONNECTED] = 100,
    [ERROR_FEEDBACK] = 100,
    [ERROR_FEEDBACK_CIRCUITRY] = 100,
    [ERROR_EEPROM_COMM] = 250,
    [ERROR_EEPROM_WRITE] = 250
};

// Errors information
static Error error_cell_under_voltage_instances[1];
static Error error_cell_over_voltage_instances[1];
static Error error_cell_under_temperature_instances[1];
static Error error_cell_over_temperature_instances[1];
static Error error_over_current_instances[1];
static Error error_can_instances[2];
static Error error_int_voltage_mismatch_instances[1];
static Error error_cellboard_comm_instances[6];
static Error error_cellboard_internal_instances[6];
static Error error_connector_disconnected_instances[3];
static Error error_fans_disconnected_instances[1];
static Error error_feedback_instances[20];
static Error error_feedback_circuitry_instances[20];
static Error error_eeprom_comm_instances[1];
static Error error_eeprom_write_instances[1];
static Error * errors[] = {
    [ERROR_CELL_UNDER_VOLTAGE] = error_cell_under_voltage_instances,
    [ERROR_CELL_OVER_VOLTAGE] = error_cell_over_voltage_instances,
    [ERROR_CELL_UNDER_TEMPERATURE] = error_cell_under_temperature_instances,
    [ERROR_CELL_OVER_TEMPERATURE] = error_cell_over_temperature_instances,
    [ERROR_OVER_CURRENT] = error_over_current_instances,
    [ERROR_CAN] = error_can_instances,
    [ERROR_INT_VOLTAGE_MISMATCH] = error_int_voltage_mismatch_instances,
    [ERROR_CELLBOARD_COMM] = error_cellboard_comm_instances,
    [ERROR_CELLBOARD_INTERNAL] = error_cellboard_internal_instances,
    [ERROR_CONNECTOR_DISCONNECTED] = error_connector_disconnected_instances,
    [ERROR_FANS_DISCONNECTED] = error_fans_disconnected_instances,
    [ERROR_FEEDBACK] = error_feedback_instances,
    [ERROR_FEEDBACK_CIRCUITRY] = error_feedback_circuitry_instances,
    [ERROR_EEPROM_COMM] = error_eeprom_comm_instances,
    [ERROR_EEPROM_WRITE] = error_eeprom_write_instances
};

// Function declaration needed for the min heap
int8_t _error_compare(void * a, void * b);

bool routine_lock = false;
RingBuffer(ErrorData, ERROR_BUFFER_SIZE) err_buf;
RingBuffer(Error *, ERROR_INSTANCE_COUNT) expired_errors = ring_buffer_new(Error *, ERROR_INSTANCE_COUNT, NULL, NULL);
MinHeap(Error *, ERROR_INSTANCE_COUNT) running_errors = min_heap_new(Error *, ERROR_INSTANCE_COUNT, _error_compare);

// Fast lookup for groups that are running or expired
uint16_t running_groups[ERROR_COUNT];
uint16_t expired_groups[ERROR_COUNT];

/**
 * @brief Compare two errors based on the time when they were set
 * and their timeouts
 * 
 * @param t1 The timestamp of the first error
 * @param dt1 The timeout of the first error
 * @param t2 The timestamp of the second error
 * @param dt2 The timeout of the second error
 * @return int32_t The difference between the two expire times
 */
int8_t _error_compare(void * a, void * b) {
    Error * e1 = *(Error **)a;
    Error * e2 = *(Error **)b;
    int32_t t1 = e1->timestamp + timeouts[e1->group];
    int32_t t2 = e2->timestamp + timeouts[e2->group];
    return t1 < t2 ? -1 : (t1 == t2 ? 0 : 1);
}

/**
 * @brief Set the error if possible and update the timer if necessary
 *
 * @param data The data of the error to set
 */
void _error_set(ErrorData data) {
    // Get error
    Error * err = &errors[data.group][data.instance];
    if (err->is_running || err->is_expired)
        return;

    // Update error info
    err->is_running = true;
    err->timestamp = data.timestamp;
    ++running_groups[data.group];

    // Add error to the running list of errors and
    // update timer if the error is the first to expire
    if (!min_heap_insert(&running_errors, &err))
        return;
    Error ** top = (Error **)min_heap_peek(&running_errors);
    if (top != NULL && *top == err)
        error_update_timer_callback(err->timestamp, timeouts[err->group]);
}

/**
 * @brief Reset the error if possible and update the timer if necessary
 *
 * @param data The data of the error to reset
 */
void _error_reset(ErrorData data) {
    // Get error
    Error * err = &errors[data.group][data.instance];
    if (!err->is_running || err->is_expired)
        return;

    // Update error info
    err->is_running = false;
    --running_groups[data.group];

    // Get the current first element
    Error * top = NULL;
    if (!min_heap_top(&running_errors, &top))
        return;

    if (top == err) {
        // If the removed error is the first in the heap
        // remove it and update (or stop) the timer
        if (!min_heap_remove(&running_errors, 0, NULL))
            return;

        if (min_heap_is_empty(&running_errors))
            error_stop_timer_callback();
        else if (min_heap_top(&running_errors, &top))
            error_update_timer_callback(top->timestamp, timeouts[top->group]);
    }
    else {
        // Find and remove the error
        ssize_t i = min_heap_find(&running_errors, &err);
        if (i < 0) return;
        min_heap_remove(&running_errors, i, NULL);
    }
}

/**
 * @brief Expire the error
 *
 * @param data The data of the error to expire
 */
void _error_expire(ErrorData data) {
    // Get error
    Error * top = NULL;
    if (!min_heap_top(&running_errors, &top))
        return;

    if (!top->is_running || top->is_expired)
        return;

    Error * prev = top;
    do {
        // Update error info
        top->is_running = false;
        top->is_expired = true;
        ++expired_groups[data.group];

        // Add error to the list of expired errors
        if (!ring_buffer_push_back(&expired_errors, top))
            break;

        // Get next error and remove the previous
        if (!min_heap_remove(&running_errors, 0, NULL))
            break;

        // Stop the timer if there are no more errors
        if (min_heap_is_empty(&running_errors)) {
            error_stop_timer_callback();
            return;
        }

        // Get next errors
        if (!min_heap_top(&running_errors, &top))
            break;
    } while(_error_compare(&top, &prev) <= 0);
    
    // Update the timer
    if (top != NULL)
        error_update_timer_callback(top->timestamp, timeouts[top->group]);
}

void error_init(void (* cs_enter)(void), void (* cs_exit)(void)) {
    ring_buffer_init(&err_buf, ErrorData, ERROR_BUFFER_SIZE, cs_enter, cs_exit);

    for (size_t i = 0; i < instances[ERROR_CELL_UNDER_VOLTAGE]; ++i)
        error_cell_under_voltage_instances[i].group = ERROR_CELL_UNDER_VOLTAGE;
    for (size_t i = 0; i < instances[ERROR_CELL_OVER_VOLTAGE]; ++i)
        error_cell_over_voltage_instances[i].group = ERROR_CELL_OVER_VOLTAGE;
    for (size_t i = 0; i < instances[ERROR_CELL_UNDER_TEMPERATURE]; ++i)
        error_cell_under_temperature_instances[i].group = ERROR_CELL_UNDER_TEMPERATURE;
    for (size_t i = 0; i < instances[ERROR_CELL_OVER_TEMPERATURE]; ++i)
        error_cell_over_temperature_instances[i].group = ERROR_CELL_OVER_TEMPERATURE;
    for (size_t i = 0; i < instances[ERROR_OVER_CURRENT]; ++i)
        error_over_current_instances[i].group = ERROR_OVER_CURRENT;
    for (size_t i = 0; i < instances[ERROR_CAN]; ++i)
        error_can_instances[i].group = ERROR_CAN;
    for (size_t i = 0; i < instances[ERROR_INT_VOLTAGE_MISMATCH]; ++i)
        error_int_voltage_mismatch_instances[i].group = ERROR_INT_VOLTAGE_MISMATCH;
    for (size_t i = 0; i < instances[ERROR_CELLBOARD_COMM]; ++i)
        error_cellboard_comm_instances[i].group = ERROR_CELLBOARD_COMM;
    for (size_t i = 0; i < instances[ERROR_CELLBOARD_INTERNAL]; ++i)
        error_cellboard_internal_instances[i].group = ERROR_CELLBOARD_INTERNAL;
    for (size_t i = 0; i < instances[ERROR_CONNECTOR_DISCONNECTED]; ++i)
        error_connector_disconnected_instances[i].group = ERROR_CONNECTOR_DISCONNECTED;
    for (size_t i = 0; i < instances[ERROR_FANS_DISCONNECTED]; ++i)
        error_fans_disconnected_instances[i].group = ERROR_FANS_DISCONNECTED;
    for (size_t i = 0; i < instances[ERROR_FEEDBACK]; ++i)
        error_feedback_instances[i].group = ERROR_FEEDBACK;
    for (size_t i = 0; i < instances[ERROR_FEEDBACK_CIRCUITRY]; ++i)
        error_feedback_circuitry_instances[i].group = ERROR_FEEDBACK_CIRCUITRY;
    for (size_t i = 0; i < instances[ERROR_EEPROM_COMM]; ++i)
        error_eeprom_comm_instances[i].group = ERROR_EEPROM_COMM;
    for (size_t i = 0; i < instances[ERROR_EEPROM_WRITE]; ++i)
        error_eeprom_write_instances[i].group = ERROR_EEPROM_WRITE;
}
size_t error_get_running(void) {
    return min_heap_size(&running_errors);
}
size_t error_get_expired(void) {
    return ring_buffer_size(&expired_errors);
}
uint16_t error_get_group_running(ErrorGroup group) {
    if (group >= ERROR_COUNT)
        return 0U;
    return running_groups[group];
}
uint16_t error_get_group_expired(ErrorGroup group) {
    if (group >= ERROR_COUNT)
        return 0U;
    return expired_groups[group];
}
size_t error_dump_running(Error * out) {
    if (out == NULL)
        return 0U;

    err_buf.cs_enter();
    // Copy data
    size_t i;
    for (i = 0; i < running_errors.size; ++i)
        memcpy(&out[i], running_errors.data[i], sizeof(Error));
    err_buf.cs_exit();
    return i;
}
size_t error_dump_expired(Error * out) {
    if (out == NULL)
        return 0U;

    err_buf.cs_enter();
    // Copy data
    size_t i;
    for (i = 0; i < expired_errors.size; ++i)
        memcpy(&out[i], expired_errors.data[i], sizeof(Error));
    err_buf.cs_exit();
    return i;
}
size_t error_dump_running_groups(ErrorGroup * out) {
    if (out == NULL)
        return 0U;
    // Copy data
    size_t cnt = 0;
    for (size_t i = 0; i < ERROR_COUNT; ++i)
        if (running_groups[i] > 0)
            out[cnt++] = i;
    return cnt;
}
size_t error_dump_expired_groups(ErrorGroup * out) {
    if (out == NULL)
        return 0U;
    // Copy data
    size_t cnt = 0;
    for (size_t i = 0; i < ERROR_COUNT; ++i)
        if (expired_groups[i] > 0)
            out[cnt++] = i;
    return cnt;
}
void error_set(ErrorGroup group, ErrorInstance instance, uint32_t timestamp) {
    if (group >= ERROR_COUNT || instance >= instances[group])
        return;

    // Push data to the buffer
    ErrorData data = {
        .group = group,
        .instance = instance,
        .timestamp = timestamp,
        .op = _error_set
    };
    if (ring_buffer_push_back(&err_buf, &data))
        error_routine();
}
void error_reset(ErrorGroup group, ErrorInstance instance) {
    if (instance >= instances[group])
        return;

    // Push data to the buffer
    ErrorData data = {
        .group = group,
        .instance = instance,
        .timestamp = 0,
        .op = _error_reset
    };
    if (ring_buffer_push_back(&err_buf, &data))
        error_routine();
}
void error_expire(void) {
    // Push data to the buffer
    ErrorData data = { .op = _error_expire };
    if (ring_buffer_push_back(&err_buf, &data))
        error_routine();
}
void error_routine(void) {
    // Avoid multiple execution of the routine
    if (routine_lock)
        return;
    routine_lock = true;

    // Check if buffer is not empty
    if (ring_buffer_is_empty(&err_buf)) {
        routine_lock = false;
        return;
    }

    // Execute the right function for the error
    ErrorData err;
    if (ring_buffer_pop_front(&err_buf, &err))
        err.op(err);

    routine_lock = false;
}

__attribute__((weak)) void error_update_timer_callback(uint32_t timestamp, uint16_t timeout) { }
__attribute__((weak)) void error_stop_timer_callback(void) { }

