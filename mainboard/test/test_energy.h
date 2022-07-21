#ifndef TEST_ENERGY_H
#define TEST_ENERGY_H

#include <munit.h>

void *test_energy_setup(const MunitParameter params[], void *user_data);
MunitResult test_energy_insert(const MunitParameter params[], void *user_data_or_fixture);

#endif