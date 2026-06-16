/*
 * Bristol Vox LV2 test runner.
 */
#include "test_common.h"

int g_tests_run = 0;
int g_tests_failed = 0;
const char *g_current_test = NULL;

extern void test_engine_suite(void);
extern void test_plugin_suite(void);

void
test_counter_fail(void)
{
	g_tests_failed++;
}

void
test_counter_run(const char *name)
{
	g_current_test = name;
	g_tests_run++;
}

int
test_report(void)
{
	printf("\n%d tests run, %d failed.\n", g_tests_run, g_tests_failed);
	return g_tests_failed == 0 ? 0 : 1;
}

int
main(void)
{
	printf("Bristol Vox LV2 tests\n");
	test_engine_suite();
	test_plugin_suite();
	return test_report();
}
