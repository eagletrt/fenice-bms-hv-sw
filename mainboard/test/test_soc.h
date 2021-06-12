#ifndef TEST_SOC_H
#define TEST_SOC_H

#include <munit.h>

void* test_soc_setup(const MunitParameter params[], void* user_data);
MunitResult test_soc_insert(const MunitParameter params[], void* user_data_or_fixture);

#endif