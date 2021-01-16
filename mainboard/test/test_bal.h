#ifndef TEST_BAL_H
#define TEST_BAL_H

#include <munit.h>

void* test_bal_setup(const MunitParameter params[], void* user_data);
MunitResult test_bal_insert(const MunitParameter params[], void* user_data_or_fixture);

#endif