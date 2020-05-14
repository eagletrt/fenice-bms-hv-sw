#include "munit.h"
#include "suites.h"
#include "test_list.h"

int main(int argc, char* argv[]) {
	MunitSuite main_suite[] = {test_list_suite, NULL};
	MunitSuite mainsuite = {
		"",
		NULL,
		main_suite,
		1,
		MUNIT_SUITE_OPTION_NONE,
	};
	return munit_suite_main(&mainsuite, NULL, argc, argv);
}