#include "main.h"

#include <munit.h>

#include "test_bal.h"
#include "test_suites.h"

int main(int argc, char* argv[]) {
	MunitSuite main_suite[] = {test_bal_suite, test_soc_suite, {NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, MUNIT_SUITE_OPTION_NONE}};
	MunitSuite mainsuite = {
		"",
		NULL,
		main_suite,
		1000,
		MUNIT_SUITE_OPTION_NONE,
	};
	return munit_suite_main(&mainsuite, NULL, argc, argv);
}