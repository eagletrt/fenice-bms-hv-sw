#include "test_soc.h"

#include <soc.h>
#include <stdio.h>

void* soc_setup(const MunitParameter params[], void* user_data) {
	soc_t soc;
	soc_init(&soc);

	return soc;
}

void soc_tear_down(void* fixture) {
	soc_deinit((soc_t*)&fixture);
}

MunitResult test_constant(const MunitParameter params[], void* user_data_or_fixture) {
	soc_t soc = (soc_t)user_data_or_fixture;

	voltage_t volt = 100 * 10;	 // 100V
	current_t current = 1 * 10;	 // 1A

	soc_reset_time(soc, 0);
	soc_sample_current(soc, current, volt, 1000);

	munit_assert_double(soc_get_joule(soc), ==, 5.0);
	munit_assert_double_equal(soc_get_wh(soc), 0.0013888, 7);

	for (size_t i = 2; i < 100; i++) {
		soc_sample_current(soc, current, volt, i * 1000);

		double j = 5.0 + 10 * (i - 1);
		munit_assert_double(soc_get_joule(soc), ==, j);
		munit_assert_double_equal(soc_get_wh(soc), j / 3600, 7);
	}
	return MUNIT_OK;
}

MunitTest test_soc_tests[] = {
	{(char*)"/constant", test_constant, soc_setup, soc_tear_down, MUNIT_TEST_OPTION_NONE, NULL},

	{NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, MUNIT_SUITE_OPTION_NONE}};

MunitSuite test_soc_suite = {
	"/soc",
	test_soc_tests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE};
