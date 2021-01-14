#include "test_bal.h"

#include <bal.h>
#include <stdio.h>

#include "test_volt_data.h"

struct data {
	bal_handle conf;
	uint16_t threshold;
	uint16_t voltages[PACK_CELL_COUNT];
};

#define BAL_TEST_DELTA 20

void* bal_setup_balanced(const MunitParameter params[], void* user_data) {
	struct data* tmp = malloc(sizeof(struct data));
	tmp->threshold = BAL_TEST_DELTA;
	volt_gen_random(tmp->voltages, 2, munit_rand_int_range(CELL_MIN_VOLTAGE, CELL_MAX_VOLTAGE));

	return tmp;
}

void* bal_setup(const MunitParameter params[], void* user_data) {
	struct data* tmp = malloc(sizeof(struct data));
	tmp->threshold = BAL_TEST_DELTA;
	volt_gen_random(tmp->voltages, tmp->threshold * 2, munit_rand_int_range(CELL_MIN_VOLTAGE + tmp->threshold, CELL_MAX_VOLTAGE - tmp->threshold));

	return tmp;
}

void tear_down(void* fixture) {
	free(fixture);
}

MunitResult test_balance(const MunitParameter params[], void* user_data_or_fixture) {
	struct data* data = (struct data*)user_data_or_fixture;

	uint8_t indexes[PACK_CELL_COUNT];
	uint16_t min = volt_min(data->voltages);

	const uint8_t max_count = 50;
	uint8_t count = 0;
	while (count++ <= max_count && bal_compute_indexes(data->voltages, indexes, data->threshold)) {
		for (size_t i = 0; i < PACK_CELL_COUNT; i++) {
			if (indexes[i] != BAL_NULL_INDEX) {
				data->voltages[indexes[i]] -= munit_rand_int_range(1, 3);
			}
		}
	}
	uint16_t new_min = volt_min(data->voltages);
	uint16_t new_max = volt_max(data->voltages);

	munit_assert_true(new_max - new_min <= data->threshold);
	munit_assert_uint8(count, <, max_count);
	munit_assert_uint16(min, ==, new_min);

	return MUNIT_OK;
}

MunitTest test_bal_tests[] = {
	{(char*)"/standard", test_balance, bal_setup, tear_down, MUNIT_TEST_OPTION_NONE, NULL},
	{(char*)"/balanced", test_balance, bal_setup_balanced, tear_down, MUNIT_TEST_OPTION_NONE, NULL},

	{NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, MUNIT_SUITE_OPTION_NONE}};

MunitSuite test_bal_suite = {
	"/balancing",
	test_bal_tests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE};
