#include "test_list.h"

#include "../Inc/common/list.h"

static void* test_list_setup(const MunitParameter params[], void* user_data) {
	return NULL;
}

static MunitResult test_list_insert(const MunitParameter params[], void* user_data_or_fixture) {
	//munit_assert_true(1);
	int a = 2;

	munit_assert_int(a, >=, 0);

	return MUNIT_OK;
}

MunitTest test_list_tests[] = {
	{(char*)"/insert", test_list_insert, test_list_setup, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

MunitSuite test_list_suite = {
	"/list",
	test_list_tests,
	NULL,
	1,
	MUNIT_SUITE_OPTION_NONE};
