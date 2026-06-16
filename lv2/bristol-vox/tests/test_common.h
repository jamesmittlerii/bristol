#ifndef BRISTOL_VOX_TEST_COMMON_H
#define BRISTOL_VOX_TEST_COMMON_H

#include <math.h>
#include <stdio.h>
#include <stdint.h>

extern int g_tests_run;
extern int g_tests_failed;
extern const char *g_current_test;

void test_counter_fail(void);
void test_counter_run(const char *name);
int test_report(void);

#define TEST(name) static void name(void)

#define ASSERT(cond) do { \
	if (!(cond)) { \
		fprintf(stderr, "\n  FAIL in %s at %s:%d: %s\n", \
			g_current_test ? g_current_test : "?", \
			__FILE__, __LINE__, #cond); \
		test_counter_fail(); \
		return; \
	} \
} while (0)

#define RUN_TEST(name) do { \
	int failed_before = g_tests_failed; \
	printf("  %s ... ", #name); \
	fflush(stdout); \
	test_counter_run(#name); \
	name(); \
	printf("%s\n", failed_before == g_tests_failed ? "ok" : "FAIL"); \
} while (0)

static inline float
test_buffer_peak(const float *buf, uint32_t n)
{
	float peak = 0.0f;
	uint32_t i;

	for (i = 0; i < n; i++) {
		float v = fabsf(buf[i]);
		if (v > peak)
			peak = v;
	}
	return peak;
}

static inline void
test_suite_begin(const char *name)
{
	printf("\n%s\n", name);
}

#endif /* BRISTOL_VOX_TEST_COMMON_H */
