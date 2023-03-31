#include "test_energy.h"

#include <energy/energy.h>
#include <pack/current.h>
#include <pack/voltage.h>
#include <stdio.h>

void *energy_setup(const MunitParameter params[], void *user_data) {
    energy_t energy;
    energy_init(&energy);

    return energy;
}

void energy_tear_down(void *fixture) {
    energy_deinit((energy_t *)&fixture);
}

MunitResult test_constant(const MunitParameter params[], void *user_data_or_fixture) {
    energy_t energy = (energy_t)user_data_or_fixture;

    voltage_t volt    = 100;  // 100V
    current_t current = 1;    // 1A

    uint32_t time = 0;
    float j       = 0.0f;
    energy_set_count(energy, j);
    energy_set_time(energy, time);
    energy_sample_energy(energy, current * volt, time);

    time += 1000;  // 1s
    j += 100.0f;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    time += 1000;  // 1s
    j += 100.0f;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    for (size_t i = 2; i < 1000; i++) {
        time += 1000;
        j += 100.0f;
        energy_sample_energy(energy, current * volt, time);

        munit_assert_float(energy_get_joule(energy), ==, j);
        munit_assert_double_equal(energy_get_wh(energy), j / 3600, 7);
    }
    return MUNIT_OK;
}

MunitResult test_var_voltage(const MunitParameter params[], void *user_data_or_fixture) {
    energy_t energy   = (energy_t)user_data_or_fixture;
    voltage_t volt    = 0;  // 100V
    current_t current = 1;  // 1A
    uint32_t time     = 0;
    float j           = 0.0f;

    energy_set_count(energy, j);
    energy_set_time(energy, time);
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    volt = 10;  // 10V
    time += 1000;
    j += 5;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    volt = 30;  // 30V
    time += 1000;
    j += 20;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    volt = 20;  // 20V
    time += 1000;
    j += 25;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    volt = 100;  // 20V
    time += 1000;
    j += 60;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    return MUNIT_OK;
}

MunitResult test_var_current(const MunitParameter params[], void *user_data_or_fixture) {
    energy_t energy   = (energy_t)user_data_or_fixture;
    voltage_t volt    = 100;   // 100V
    current_t current = 0.0f;  // 1A
    uint32_t time     = 0;
    float j           = 0.0f;

    energy_set_count(energy, j);
    energy_set_time(energy, time);
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    current = 10.0f;  // 10A
    time += 1000;
    j += 500;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    current = 30.0f;  // 30A
    time += 1000;
    j += 2000;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    current = 20.0f;  // 20A
    time += 1000;
    j += 2500;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    current = 100.0f;  // 100A
    time += 1000;
    j += 6000;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    current = -12.0f;  // -12A
    time += 1000;
    j += 4400;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    current = -22.0f;  // -22A
    time += 1000;
    j += -1700;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);

    current = -25.0f;  // -25A
    time += 1000;
    j += -2350;
    energy_sample_energy(energy, current * volt, time);
    munit_assert_float(energy_get_joule(energy), ==, j);
    munit_assert_double_equal(energy_get_wh(energy), j / 3600.0f, 7);
    return MUNIT_OK;
}

MunitTest test_energy_tests[] = {
    {(char *)"/constant", test_constant, energy_setup, energy_tear_down, MUNIT_TEST_OPTION_NONE, NULL},
    {(char *)"/variable_voltage", test_var_voltage, energy_setup, energy_tear_down, MUNIT_TEST_OPTION_NONE, NULL},
    {(char *)"/variable_current", test_var_current, energy_setup, energy_tear_down, MUNIT_TEST_OPTION_NONE, NULL},

    {NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, MUNIT_SUITE_OPTION_NONE}};

MunitSuite test_energy_suite = {"/energy", test_energy_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE};
