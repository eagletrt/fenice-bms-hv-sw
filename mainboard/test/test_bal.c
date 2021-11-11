#include "test_bal.h"

#include <bal.h>
#include <stdio.h>

#include "test_volt_data.h"

struct data {
	uint16_t threshold;
	uint16_t voltages[PACK_CELL_COUNT];
};

#define BAL_TEST_DELTA 10
#define COUNT_MAX 50

void* bal_setup_balanced(const MunitParameter params[], void* user_data) {
	struct data* tmp = malloc(sizeof(struct data));
	tmp->threshold = BAL_TEST_DELTA;
	volt_gen_random(tmp->voltages, tmp->threshold, munit_rand_int_range(CELL_MIN_VOLTAGE + tmp->threshold / 2, CELL_MAX_VOLTAGE - tmp->threshold / 2));

	return tmp;
}

void* bal_setup_unbalanced(const MunitParameter params[], void* user_data) {
	struct data* tmp = malloc(sizeof(struct data));
	tmp->threshold = BAL_TEST_DELTA;
	volt_gen_random(tmp->voltages, tmp->threshold * 2, munit_rand_int_range(CELL_MIN_VOLTAGE + tmp->threshold, CELL_MAX_VOLTAGE - tmp->threshold));

	return tmp;
}

void tear_down(void* fixture) {
	free(fixture);
}

/**
 * @brief Fakes a balancing cycle by randomly decreasing cell voltages.
 */
uint16_t balance(struct data* data, uint16_t max_count) {
	bms_balancing_cells cells[LTC6813_COUNT] = {0};
	// set a maximum count to prevent killer loops
	while (max_count > 0 && bal_get_cells_to_discharge(data->voltages, PACK_CELL_COUNT, data->threshold, cells, LTC6813_COUNT) > 0) {
		max_count--;

		for (uint16_t i = 0; i < PACK_CELL_COUNT; i++) {
			if (getBit(cells[i / LTC6813_CELL_COUNT], (i % LTC6813_CELL_COUNT))) {
				data->voltages[i] -= munit_rand_int_range(1, 3);
			}
		}
	}

	return max_count;
}

/**
 * @brief	tests balancing with a balanced dataset
 */
MunitResult test_balanced(const MunitParameter params[], void* user_data_or_fixture) {
	struct data* data = (struct data*)user_data_or_fixture;

	uint16_t min = volt_min(data->voltages);
	uint16_t max = volt_max(data->voltages);

	uint16_t count = balance(data, COUNT_MAX);

	uint16_t new_min = volt_min(data->voltages);
	uint16_t new_max = volt_max(data->voltages);

	munit_assert_uint16(count, ==, COUNT_MAX);
	munit_assert_uint16(max, ==, new_max);
	munit_assert_uint16(min, ==, new_min);

	return MUNIT_OK;
}

/**
 * @brief	tests balancing with an unbalanced dataset
 */
MunitResult test_unbalanced(const MunitParameter params[], void* user_data_or_fixture) {
	struct data* data = (struct data*)user_data_or_fixture;

	uint16_t min = volt_min(data->voltages);

	uint16_t count = balance(data, COUNT_MAX);

	uint16_t new_min = volt_min(data->voltages);
	uint16_t new_max = volt_max(data->voltages);

	// have we exceeded the maximum number of cycles?
	munit_assert_uint16(count, >, 0);

	// check that cells are actually balanced
	munit_assert_uint16(new_max - new_min, <=, data->threshold);

	// minimum voltage should'n have changed
	munit_assert_uint16(min, ==, new_min);

	return MUNIT_OK;
}

MunitTest test_bal_tests[] = {
	{(char*)"/balanced", test_balanced, bal_setup_balanced, tear_down, MUNIT_TEST_OPTION_NONE, NULL},
	{(char*)"/unbalanced", test_unbalanced, bal_setup_unbalanced, tear_down, MUNIT_TEST_OPTION_NONE, NULL},

	{NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, MUNIT_SUITE_OPTION_NONE}};

MunitSuite test_bal_suite = {"/balancing", test_bal_tests, NULL, 1, MUNIT_SUITE_OPTION_NONE};
