#if 0

#include <stdio.h>
#include "assert.h"
#include "/home/gmazzucchi/ssd/eagle/old-hv/fenice-bms-hv-sw/mainboard/Inc/error/error_simple.h"

int scattare() {
    for (size_t i = 0; i < ERROR_SIMPLE_COUNTER_THRESHOLD; i++) {
        assert(error_simple_set(ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE, 0) == 0);
    }
    assert(error_simple_routine() == 1);
}

int scattare2() {
    for (size_t i = 0; i < ERROR_SIMPLE_COUNTER_THRESHOLD; i++) {
        assert(error_simple_set(ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE, 0) == 0);
        assert(error_simple_set(ERROR_GROUP_ERROR_CELL_UNDER_TEMPERATURE, 0) == 0);
    }
    assert(error_simple_routine() == 2);
}

int resettare() {
    for (size_t i = 0; i < ERROR_SIMPLE_COUNTER_THRESHOLD - 1; i++) {
        assert(error_simple_set(ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE, 0) == 0);
    }
    assert(error_simple_routine() == 0);

    assert(error_simple_reset(ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE, 0) == 0);
    for (size_t i = 0; i < ERROR_SIMPLE_COUNTER_THRESHOLD - 1; i++) {
        assert(error_simple_set(ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE, 0) == 0);
    }
    assert(error_simple_routine() == 0);

    assert(error_simple_set(ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE, 0) == 0);
    assert(error_simple_routine() == 1);

    assert(error_simple_reset(ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE, 0) == 0);
    assert(error_simple_routine() == 1); // we are in error mode so even if we reset the error we are in error
}

int limiti1() {
    assert(error_simple_set(ERROR_GROUP_ERROR_CELL_OVER_VOLTAGE, 12032) < 0);
}

int limiti2() {
    assert(error_simple_set(N_ERROR_GROUPS + 1, 12032) < 0);
}

int dump_check1() {
    for (size_t i = 0; i < ERROR_SIMPLE_COUNTER_THRESHOLD; i++) {
        assert(error_simple_set(ERROR_GROUP_ERROR_CELLBOARD_COMM, 3) == 0);
    }
    assert(error_simple_routine() == 1);
    assert(error_simple_dump[0].group == ERROR_GROUP_ERROR_CELLBOARD_COMM);
    assert(error_simple_dump[0].instance == 3);
}

int dump_check2() {
    for (size_t i = 0; i < ERROR_SIMPLE_COUNTER_THRESHOLD; i++) {
        assert(error_simple_set(ERROR_GROUP_ERROR_FANS_DISCONNECTED, 0) == 0);
        assert(error_simple_set(ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY, 17) == 0);
    }
    assert(error_simple_routine() == 2);
    if (error_simple_dump[0].group != ERROR_GROUP_ERROR_FANS_DISCONNECTED) {
        printf("%d != %d\n", error_simple_dump[0].group, ERROR_GROUP_ERROR_FANS_DISCONNECTED);
    }
    if (error_simple_dump[0].instance != 0) {
        printf("%d != %d\n", error_simple_dump[0].instance, 0);
    }
    if (error_simple_dump[1].group != ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY) {
        printf("%d != %d\n", error_simple_dump[1].group, ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY);
    }
    if (error_simple_dump[1].instance != 17) {
        printf("%d != %d\n", error_simple_dump[1].instance, 17);
    }
}

int dump_check3() {
    for (size_t i = 0; i < ERROR_SIMPLE_COUNTER_THRESHOLD; i++) {
        assert(error_simple_set(ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE, 0) == 0);
        assert(error_simple_set(ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY, 17) == 0);
        assert(error_simple_set(ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED, 2) == 0);
    }
    assert(error_simple_routine() == 3);
    if (error_simple_dump[0].group != ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE) {
        printf("%d != %d\n", error_simple_dump[0].group, ERROR_GROUP_ERROR_CELL_UNDER_VOLTAGE);
    }
    if (error_simple_dump[0].instance != 0) {
        printf("%d != %d\n", error_simple_dump[0].instance, 0);
    }
    if (error_simple_dump[2].group != ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY) {
        printf("%d != %d\n", error_simple_dump[1].group, ERROR_GROUP_ERROR_FEEDBACK_CIRCUITRY);
    }
    if (error_simple_dump[2].instance != 17) {
        printf("%d != %d\n", error_simple_dump[1].instance, 17);
    }
    if (error_simple_dump[1].group != ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED) {
        printf("%d != %d\n", error_simple_dump[1].group, ERROR_GROUP_ERROR_CONNECTOR_DISCONNECTED);
    }
    if (error_simple_dump[1].instance != 2) {
        printf("%d != %d\n", error_simple_dump[1].instance, 2);
    }
}

int main() {
    // UNO ALLA VOLTA PLS
    // scattare();
    // scattare2();
    // resettare();
    // limiti1();
    // limiti2();
    // dump_check1();
    // dump_check2();
    dump_check3();
}

#endif