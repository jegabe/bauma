#ifndef BMA_LOG_H_INCLUDED
#define BMA_LOG_H_INCLUDED

/*
==============================================================================
MIT License

Copyright (c) 2026 Jens Ganter-Benzing

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
==============================================================================

Bau 'ma's logging framework

*/
#include <bma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef unsigned char bma_LogLevel;

#define BMA_LOG_LEVEL_DEBUG ((bma_LogLevel)0)
#define BMA_LOG_LEVEL_INFO  ((bma_LogLevel)1)
#define BMA_LOG_LEVEL_WARN  ((bma_LogLevel)2)
#define BMA_LOG_LEVEL_ERROR ((bma_LogLevel)3)
#define BMA_LOG_LEVEL_FATAL ((bma_LogLevel)4)
#define BMA_LOG_LEVEL_OFF   ((bma_LogLevel)5)
#define BMA_LOG_LEVEL_NUM_LEVELS 6

typedef struct bma_Log bma_Log;

BMA_DEF void bma_log_init(void);
BMA_DEF void bma_log_clnup(void);
BMA_DEF bma_Log *bma_Log_get_impl_(const char *pPath);
BMA_DEF bma_bool_t bma_Log_isLoggable_impl_(bma_Log *pLog, bma_LogLevel level);
BMA_DEF void bma_Log_log_impl_(bma_Log *pLog, bma_LogLevel level, const char *pFile, int line, const char *pFmt, ...);

#ifdef BMA_LOG_DISABLE

#define bma_Log_get(pPath) ((bma_Log*)NULL)
#define bma_Log_isLoggable(pLog, level) (BMA_FALSE)

#define bma_Log_debug_0(pLog, pFmt) ((void)pLog)
#define bma_Log_debug_1(pLog, pFmt, p0) ((void)pLog)
#define bma_Log_debug_2(pLog, pFmt, p0, p1) ((void)pLog)
#define bma_Log_debug_3(pLog, pFmt, p0, p1, p2) ((void)pLog)
#define bma_Log_debug_4(pLog, pFmt, p0, p1, p2, p3) ((void)pLog)
#define bma_Log_debug_5(pLog, pFmt, p0, p1, p2, p3, p4) ((void)pLog)
#define bma_Log_debug_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) ((void)pLog)
#define bma_Log_debug_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) ((void)pLog)
#define bma_Log_debug_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) ((void)pLog)

#define bma_Log_info_0(pLog, pFmt) ((void)pLog)
#define bma_Log_info_1(pLog, pFmt, p0) ((void)pLog)
#define bma_Log_info_2(pLog, pFmt, p0, p1) ((void)pLog)
#define bma_Log_info_3(pLog, pFmt, p0, p1, p2) ((void)pLog)
#define bma_Log_info_4(pLog, pFmt, p0, p1, p2, p3) ((void)pLog)
#define bma_Log_info_5(pLog, pFmt, p0, p1, p2, p3, p4) ((void)pLog)
#define bma_Log_info_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) ((void)pLog)
#define bma_Log_info_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) ((void)pLog)
#define bma_Log_info_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) ((void)pLog)

#define bma_Log_warn_0(pLog, pFmt) ((void)pLog)
#define bma_Log_warn_1(pLog, pFmt, p0) ((void)pLog)
#define bma_Log_warn_2(pLog, pFmt, p0, p1) ((void)pLog)
#define bma_Log_warn_3(pLog, pFmt, p0, p1, p2) ((void)pLog)
#define bma_Log_warn_4(pLog, pFmt, p0, p1, p2, p3) ((void)pLog)
#define bma_Log_warn_5(pLog, pFmt, p0, p1, p2, p3, p4) ((void)pLog)
#define bma_Log_warn_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) ((void)pLog)
#define bma_Log_warn_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) ((void)pLog)
#define bma_Log_warn_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) ((void)pLog)

#define bma_Log_error_0(pLog, pFmt) ((void)pLog)
#define bma_Log_error_1(pLog, pFmt, p0) ((void)pLog)
#define bma_Log_error_2(pLog, pFmt, p0, p1) ((void)pLog)
#define bma_Log_error_3(pLog, pFmt, p0, p1, p2) ((void)pLog)
#define bma_Log_error_4(pLog, pFmt, p0, p1, p2, p3) ((void)pLog)
#define bma_Log_error_5(pLog, pFmt, p0, p1, p2, p3, p4) ((void)pLog)
#define bma_Log_error_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) ((void)pLog)
#define bma_Log_error_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) ((void)pLog)
#define bma_Log_error_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) ((void)pLog)

#define bma_Log_fatal_0(pLog, pFmt) ((void)pLog)
#define bma_Log_fatal_1(pLog, pFmt, p0) ((void)pLog)
#define bma_Log_fatal_2(pLog, pFmt, p0, p1) ((void)pLog)
#define bma_Log_fatal_3(pLog, pFmt, p0, p1, p2) ((void)pLog)
#define bma_Log_fatal_4(pLog, pFmt, p0, p1, p2, p3) ((void)pLog)
#define bma_Log_fatal_5(pLog, pFmt, p0, p1, p2, p3, p4) ((void)pLog)
#define bma_Log_fatal_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) ((void)pLog)
#define bma_Log_fatal_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) ((void)pLog)
#define bma_Log_fatal_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) ((void)pLog)

#else /* BMA_LOG_DISABLE not defined */

#define bma_Log_get(pPath) bma_Log_get_impl_(pPath)
#define bma_Log_isLoggable(pLog, level) bma_Log_isLoggable_impl_(pLog, level)

#define bma_Log_debug_0(pLog, pFmt) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, (pFmt))
#define bma_Log_debug_1(pLog, pFmt, p0) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, (pFmt), (p0))
#define bma_Log_debug_2(pLog, pFmt, p0, p1) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, (pFmt), (p0), (p1))
#define bma_Log_debug_3(pLog, pFmt, p0, p1, p2) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2))
#define bma_Log_debug_4(pLog, pFmt, p0, p1, p2, p3) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3))
#define bma_Log_debug_5(pLog, pFmt, p0, p1, p2, p3, p4) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4))
#define bma_Log_debug_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5))
#define bma_Log_debug_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6))
#define bma_Log_debug_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_DEBUG, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6), (p7))

#define bma_Log_info_0(pLog, pFmt) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_INFO, __FILE__, __LINE__, (pFmt))
#define bma_Log_info_1(pLog, pFmt, p0) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_INFO, __FILE__, __LINE__, (pFmt), (p0))
#define bma_Log_info_2(pLog, pFmt, p0, p1) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_INFO, __FILE__, __LINE__, (pFmt), (p0), (p1))
#define bma_Log_info_3(pLog, pFmt, p0, p1, p2) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_INFO, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2))
#define bma_Log_info_4(pLog, pFmt, p0, p1, p2, p3) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_INFO, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3))
#define bma_Log_info_5(pLog, pFmt, p0, p1, p2, p3, p4) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_INFO, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4))
#define bma_Log_info_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_INFO, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5))
#define bma_Log_info_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_INFO, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6))
#define bma_Log_info_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_INFO, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6), (p7))

#define bma_Log_warn_0(pLog, pFmt) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_WARN, __FILE__, __LINE__, (pFmt))
#define bma_Log_warn_1(pLog, pFmt, p0) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_WARN, __FILE__, __LINE__, (pFmt), (p0))
#define bma_Log_warn_2(pLog, pFmt, p0, p1) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_WARN, __FILE__, __LINE__, (pFmt), (p0), (p1))
#define bma_Log_warn_3(pLog, pFmt, p0, p1, p2) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_WARN, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2))
#define bma_Log_warn_4(pLog, pFmt, p0, p1, p2, p3) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_WARN, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3))
#define bma_Log_warn_5(pLog, pFmt, p0, p1, p2, p3, p4) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_WARN, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4))
#define bma_Log_warn_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_WARN, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5))
#define bma_Log_warn_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_WARN, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6))
#define bma_Log_warn_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_WARN, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6), (p7))

#define bma_Log_error_0(pLog, pFmt) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, (pFmt))
#define bma_Log_error_1(pLog, pFmt, p0) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, (pFmt), (p0))
#define bma_Log_error_2(pLog, pFmt, p0, p1) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, (pFmt), (p0), (p1))
#define bma_Log_error_3(pLog, pFmt, p0, p1, p2) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2))
#define bma_Log_error_4(pLog, pFmt, p0, p1, p2, p3) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3))
#define bma_Log_error_5(pLog, pFmt, p0, p1, p2, p3, p4) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4))
#define bma_Log_error_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5))
#define bma_Log_error_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6))
#define bma_Log_error_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_ERROR, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6), (p7))

#define bma_Log_fatal_0(pLog, pFmt) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, (pFmt))
#define bma_Log_fatal_1(pLog, pFmt, p0) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, (pFmt), (p0))
#define bma_Log_fatal_2(pLog, pFmt, p0, p1) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, (pFmt), (p0), (p1))
#define bma_Log_fatal_3(pLog, pFmt, p0, p1, p2) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2))
#define bma_Log_fatal_4(pLog, pFmt, p0, p1, p2, p3) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3))
#define bma_Log_fatal_5(pLog, pFmt, p0, p1, p2, p3, p4) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4))
#define bma_Log_fatal_6(pLog, pFmt, p0, p1, p2, p3, p4, p5) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5))
#define bma_Log_fatal_7(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6))
#define bma_Log_fatal_8(pLog, pFmt, p0, p1, p2, p3, p4, p5, p6, p7) bma_Log_log_impl_((pLog), BMA_LOG_LEVEL_FATAL, __FILE__, __LINE__, (pFmt), (p0), (p1), (p2), (p3), (p4), (p5), (p6), (p7))

#endif /* BMA_LOG_DISABLE */

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_LOG_IMPL

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
	extern "C" {
#endif

struct bma_Log {
	/* TODO */
};

BMA_DEF void bma_Log_dtor(bma_Log *pLog, bma_IMemAlloc *pAlloc) {
	(void)pLog;
	(void)pAlloc;
	/* TODO */
}

BMA_DEF_STRTREE(bma_LogTree, '.', bma_Log, (bma_dtor_t)&bma_Log_dtor)

static bma_LogTree g_logTree;

BMA_DEF void bma_log_init(void) {
	bma_LogTree_ctor(&g_logTree);
}

BMA_DEF void bma_log_clnup(void) {
	bma_LogTree_dtor(&g_logTree, NULL);
}

BMA_DEF bma_Log *bma_Log_get_impl_(const char *pPath) {
	bma_Log *pLog, newLog;
	pLog = bma_LogTree_get(&g_logTree, pPath);
	if (pLog != NULL) return pLog;
	pLog = bma_LogTree_put(&g_logTree, pPath, &newLog);
	return pLog;
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_LOG_IMPL */

#ifdef BMA_LOG_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
	extern "C" {
#endif

void bma_test_exit_fail(const char *exp, const char *file, int line) {
	fprintf(stderr, "Expectation '%s' failed (file %s line %d)\n", exp, file, line);
	fflush(stderr);
	exit(1);
}

#define BMA_EXPECT(x) \
	do { \
		if(!(x)) { \
			bma_test_exit_fail(#x, __FILE__, __LINE__); \
		} \
	} while(0)

#define BMA_TEST(f) \
	do { \
		printf("Executing '" #f "()'...\n"); \
		f(); \
	} while(0)


#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	/* TODO
	BMA_TEST(test_strdup);
	*/
	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_LOG_TEST */

#endif /* BMA_LOG_H_INCLUDED */
