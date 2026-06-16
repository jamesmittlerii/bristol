/*
 * Platform shims for Bristol LV2 builds (Windows / MinGW).
 * Included before all translation units via -include.
 */
#ifndef BRISTOL_LV2_PLATFORM_H
#define BRISTOL_LV2_PLATFORM_H

#if defined(_WIN32) || defined(__MINGW32__)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#ifndef u_char
typedef unsigned char u_char;
#endif
#ifndef u_int64_t
typedef uint64_t u_int64_t;
#endif

#ifndef bzero
#define bzero(s, n) memset((void *)(s), 0, (size_t)(n))
#endif

/* MinGW provides usleep/sleep via unistd.h; do not redefine them here. */

#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wdeprecated-non-prototype"
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"
#endif

#endif /* Windows / MinGW */

#endif /* BRISTOL_LV2_PLATFORM_H */

