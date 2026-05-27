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

BMA_DEF void bma_log_init_ext(bma_IMemAlloc *pAlloc);
#define bma_log_init() bma_log_init_ext(bma_getDfltMemAlloc())
BMA_DEF void bma_log_clnup(void);
BMA_DEF bma_Log *bma_Log_get_impl_(const char *pPath);
BMA_DEF bma_bool_t bma_Log_isLoggable_impl_(bma_Log *pLog, bma_LogLevel level);
BMA_DEF void bma_Log_log_impl_(bma_Log *pLog, bma_LogLevel level, const char *pFile, int line, const char *pFmt, ...);

BMA_DEF void bma_Log_addCnslSink(const char* pPath, bma_LogLevel level);

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
#include <time.h>
#include <bma/thrd.h>

#ifdef __cplusplus
	extern "C" {
#endif

BMA_DEF void bma_LogPtr_dtor(bma_Log **ppLog, bma_IMemAlloc *pAlloc);

BMA_DEF_VEC(bma_LogPtrVec, bma_Log*, (bma_dtor_t)&bma_LogPtr_dtor)

typedef struct bma_LogMsg {
	bma_LogLevel level;
	const char *pPath;
	size_t pathLen;
	const char *pFile;
	size_t fileLen;
	int line;
	const char *pDateTime;
	size_t dateTimeLen;
	const char *pMsg;
	size_t msgLen;
} bma_LogMsg;

typedef struct bma_LogSink {
	void (*pDtor)(void* pThis, bma_IMemAlloc* pOptAlloc);
	bma_LogLevel (*pGetMinLevel)(const void *pThis);
	void (*pHandle)(void* pThis, bma_LogMsg *pMsg);
} bma_LogSink;

BMA_DEF void bma_LogSinkPtr_dtor(bma_LogSink **ppSink, bma_IMemAlloc *pAlloc) {
	bma_assert(ppSink != NULL);
	bma_assert(*ppSink != NULL);
	(*ppSink)->pDtor(*ppSink, pAlloc);
	bma_free_ext(pAlloc, *ppSink);
}

BMA_DEF_VEC(bma_LogSinkPtrVec, bma_LogSink*, (bma_dtor_t)&bma_LogSinkPtr_dtor)

struct bma_Log {
	char *pName;
	bma_Log *pParent;
	bma_LogPtrVec children;
	bma_LogSinkPtrVec sinks;
	bma_atmc_t minLevel;
};

static bma_Rw g_createLock;
static bma_Log *g_pRoot = NULL;
static bma_IMemAlloc *g_pAlloc = NULL;
static bma_bool_t g_stdOutIsTerminal = BMA_FALSE;

BMA_DEF	void bma_Log_dtor(bma_Log *pSelf, bma_IMemAlloc *pAlloc) {
	(void)pAlloc;
	bma_LogSinkPtrVec_dtor(&pSelf->sinks, NULL);
	bma_LogPtrVec_dtor(&pSelf->children, NULL);
	if (pSelf != g_pRoot) {
		bma_free_ext(g_pAlloc, pSelf->pName);
	}
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF void bma_LogPtr_dtor(bma_Log **ppLog, bma_IMemAlloc *pAlloc) {
	(void)pAlloc;
	bma_assert(ppLog != NULL);
	bma_assert(*ppLog != NULL);
	bma_Log_dtor(*ppLog, NULL);
	bma_free_ext(g_pAlloc, *ppLog);
}

BMA_DEF void bma_log_init_ext(bma_IMemAlloc *pAlloc) {
#if BMA_WIN_THRDS
	HANDLE hConsole;
	DWORD mode;
	BOOL isConsole;
#endif
	bma_assert(g_pRoot == NULL);
	bma_assert(g_pAlloc == NULL);
	bma_Rw_ctor(&g_createLock);
	bma_Rw_lckWrt(&g_createLock);
	g_pAlloc = pAlloc;
	g_pRoot = bma_malloc_ext(g_pAlloc, bma_Log);
	g_pRoot->pName = (char*)"";
	g_pRoot->pParent = NULL;
	bma_LogPtrVec_ctor_ext(&g_pRoot->children, g_pAlloc);
	bma_LogSinkPtrVec_ctor_ext(&g_pRoot->sinks, g_pAlloc);
	g_pRoot->minLevel = (bma_atmc_t)BMA_LOG_LEVEL_OFF;
#if BMA_WIN_THRDS
	/* By default, Windows console is neither UTF-8 nor color enabled. Let's fix that. */
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	mode = 0;
	isConsole = GetConsoleMode(hConsole, &mode);
	if (isConsole) {
		/* Enable color rendering with ANSI escape sequences */
		SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		g_stdOutIsTerminal = BMA_TRUE;
		SetConsoleCP(CP_UTF8);
		SetConsoleOutputCP(CP_UTF8);
	}
#elif BMA_POSIX_THRDS
	if (isatty(STDOUT_FILENO) != 0) {
		g_stdOutIsTerminal = BMA_TRUE;
	}
#endif
	bma_Rw_unlckWrt(&g_createLock);
}

BMA_DEF void bma_log_clnup(void) {
	bma_assert(g_pRoot != NULL);
	bma_Rw_lckWrt(&g_createLock);
	bma_Log_dtor(g_pRoot, NULL);
	bma_free_ext(g_pAlloc, g_pRoot);
	g_pRoot = NULL;
	g_pAlloc = NULL;
	g_stdOutIsTerminal = BMA_FALSE;
	bma_Rw_unlckWrt(&g_createLock);
	bma_Rw_dtor(&g_createLock, NULL);
}

BMA_DEF bma_bool_t bma_log_sameStr(const char *p0, size_t l0, const char *p1, size_t l1) {
	return (l0 == l1) && (memcmp(p0, p1, l0) == 0);
}

BMA_DEF bma_Log *bma_Log_getOrCreate(const char *pPath, size_t pathLen, bma_bool_t create) {
	bma_bool_t end = BMA_FALSE;
	bma_Log *pResult;
	bma_assert(pPath != NULL);
	bma_assert(g_pRoot != NULL);
	pResult = g_pRoot;
	if (pathLen == 0) { /* root node requested */
		return pResult;
	}
	while (!end) {
		size_t l, i;
		bma_bool_t found;
		const char* p = (const char*)memchr(pPath, '.', pathLen);
		if (p != NULL) {
			l = (size_t)(p - pPath);
		}
		else {
			l = pathLen;
			end = BMA_TRUE; /* no further dot char -> no further sub path */
		}
		found = BMA_FALSE;
		for (i=0; i<bma_LogPtrVec_getSz(&pResult->children); ++i) {
			bma_Log *pChild = *bma_LogPtrVec_at(&pResult->children, i);
			bma_assert(pChild != NULL);
			if (bma_log_sameStr(pChild->pName, strlen(pChild->pName), pPath, l)) {
				pResult = pChild;
				found = BMA_TRUE;
				break;
			}
		}
		if (!found) {
			bma_Log *pNew;
			if (!create) return NULL;
			pNew = bma_malloc_ext(g_pAlloc, bma_Log);
			pNew->pName = bma_strndup_ext(pPath, l, g_pAlloc);
			pNew->pParent = pResult;
			bma_LogPtrVec_ctor_ext(&pNew->children, g_pAlloc);
			bma_LogPtrVec_appnd(&pResult->children, &pNew);
			bma_LogSinkPtrVec_ctor_ext(&pNew->sinks, g_pAlloc);
			pNew->minLevel = (bma_atmc_t)BMA_LOG_LEVEL_OFF;
			pResult = pNew;
		}
		if (!end) {
			pPath += (l + 1u); /* 1u: jump behind the dot char */
			pathLen -= (l + 1u);
		}
	}
	return pResult;
}

BMA_DEF bma_Log *bma_Log_get_impl_(const char *pPath) {
	bma_Log *pResult;
	size_t pathLen;
	bma_assert(pPath != NULL);
	bma_assert(g_pRoot != NULL);
	pathLen = strlen(pPath);
	/* Read-lock, check if exists, if not -> write-lock and create */
	bma_Rw_lckRd(&g_createLock);
	pResult = bma_Log_getOrCreate(pPath, pathLen, BMA_FALSE);
	bma_Rw_unlckRd(&g_createLock);
	if (pResult != NULL) return pResult;
	bma_Rw_lckWrt(&g_createLock);
	pResult = bma_Log_getOrCreate(pPath, pathLen, BMA_TRUE);
	bma_Rw_unlckWrt(&g_createLock);
	bma_assert(pResult != NULL);
	return pResult;
}

BMA_DEF bma_bool_t bma_Log_isLoggable_impl_(bma_Log *pLog, bma_LogLevel level) {
	bma_atmc_t currLvl = bma_atmc_cas(&pLog->minLevel, 0, 0);
	bma_bool_t result = (bma_bool_t)(level >= (bma_LogLevel)currLvl);
	return result;
}

BMA_DEF const char *bma_Log_stripPath(const char *pFileName, size_t *pOutFileNameLen) {
	size_t l, n;
	bma_assert(pFileName != NULL);
	l = n = strlen(pFileName);
	while (l-- > 0) {
		if (pFileName[l] == '/' || pFileName[l] == '\\') {
			*pOutFileNameLen = (n - l) - 1u;
			return &pFileName[l + 1u];
		}
	}
	*pOutFileNameLen = n;
	return pFileName;
}

BMA_DEF void bma_Log_getFullName(bma_Log *pLog, bma_StrBldr *pDst) {
	bma_assert(pLog != NULL);
	bma_assert(pDst != NULL);
	if (pLog->pParent != NULL) {
		bma_Log_getFullName(pLog->pParent, pDst);
	}
	if (bma_StrBldr_getSz(pDst) > 0) {
		bma_StrBldr_appndChr(pDst, '.', 1u);
	}
	bma_StrBldr_appndStr(pDst, pLog->pName);
}

BMA_DEF void bma_Log_callSinks(bma_Log *pLog, bma_LogMsg *pMsg) {
	size_t i;
	bma_assert(pLog != NULL);
	bma_assert(pMsg != NULL);
	if (pLog->pParent != NULL) {
		bma_Log_callSinks(pLog->pParent, pMsg);
	}
	for (i=0; i<bma_LogSinkPtrVec_getSz(&pLog->sinks); ++i) {
		bma_LogSink *pSink = *bma_LogSinkPtrVec_at(&pLog->sinks, i);
		(*pSink->pHandle)(pSink, pMsg);
	}
}

BMA_DEF void bma_Log_log_impl_(bma_Log *pLog, bma_LogLevel level, const char *pFile, int line, const char *pFmt, ...) {
	va_list ap;
	bma_StrBldr pathBldr;
	bma_StrBldr msgBldr;
	time_t currTime;
	struct tm tmBuf;
	struct tm *currTmStrct;
	char timeBuf[32];
	bma_LogMsg msg;
	int i;
	bma_atmc_t currLvl = bma_atmc_cas(&pLog->minLevel, 0, 0);
	if (level < (bma_LogLevel)currLvl) {
		/* fast exit important to be performant when log level not reached */
		return;
	}
	currTime = time(NULL);
	/* Now, format once and then pass to all sinks. Saves time at the sinks */
	bma_StrBldr_ctor_ext(&pathBldr, g_pAlloc);
	bma_StrBldr_rsrv(&pathBldr, 128);
	bma_Log_getFullName(pLog, &pathBldr);
	bma_StrBldr_ctor_ext(&msgBldr, g_pAlloc);
	bma_StrBldr_rsz(&msgBldr, 255);
	for (;;) {
		va_start(ap, pFmt);
		i = vsnprintf((char*)bma_StrBldr_getStr(&msgBldr), (bma_StrBldr_getSz(&msgBldr)) + 1u, pFmt, ap);
		va_end(ap);
		if (i < 0) bma_exit_err("vsnprintf() failed");
		if ((size_t)i <= bma_StrBldr_getSz(&msgBldr)) {
			if ((size_t)i < bma_StrBldr_getSz(&msgBldr)) {
				bma_StrBldr_rsz(&msgBldr, (size_t)i);
			}
			break;
		}
		/* else, buffer was too small: */
		bma_StrBldr_rsz(&msgBldr, (size_t)i + 1u);
		/* and try again */
	}
	memset(&msg, 0, sizeof(msg));
	msg.level = level;
	msg.pPath = bma_StrBldr_getStr(&pathBldr);
	msg.pathLen = bma_StrBldr_getSz(&pathBldr);	
	msg.pFile = bma_Log_stripPath(pFile, &msg.fileLen);
	msg.line = line;
#if BMA_WIN_THRDS
	if (localtime_s(&tmBuf, &currTime) != 0) bma_exit_err("localtime_s() failed");
	currTmStrct = &tmBuf;
#elif BMA_POSIX_THRDS
	currTmStrct = localtime_r(&currTime, &tmBuf);
	if (currTmStrct == NULL) bma_exit_err("localtime_r() failed");
#else /* old thread-unsafe C89 API */
	currTmStrct = localtime(&currTime);
	if (currTmStrct == NULL) bma_exit_err("localtime() failed");
	(void)tmBuf;
#endif
	msg.dateTimeLen = strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%dT%H:%M:%S", currTmStrct);
	bma_assert(msg.dateTimeLen > 0);
	bma_assert(msg.dateTimeLen == strlen(timeBuf));
	msg.pDateTime = timeBuf;
	msg.pMsg = bma_StrBldr_getStr(&msgBldr);
	msg.msgLen = bma_StrBldr_getSz(&msgBldr);
	bma_Rw_lckRd(&g_createLock); /* to safely iterate sinks and parent loggers */
	bma_Log_callSinks(pLog, &msg);
	bma_Rw_unlckRd(&g_createLock);
	bma_StrBldr_dtor(&msgBldr, NULL);
	bma_StrBldr_dtor(&pathBldr, NULL);
}

BMA_DEF void bma_Log_reclcLvl(bma_Log *pLog, bma_LogLevel lvl) {
	size_t i;
	for (i=0; i<bma_LogSinkPtrVec_getSz(&pLog->sinks); ++i) {
		bma_LogLevel sinkLvl;
		bma_LogSink *pSink = *bma_LogSinkPtrVec_at(&pLog->sinks, i);
		bma_assert(pSink != NULL);
		sinkLvl = (pSink->pGetMinLevel)(pSink);
		if (sinkLvl < lvl) {
			lvl = sinkLvl;
		}
	}
	(void)bma_atmc_exchg(&pLog->minLevel, (bma_atmc_t)lvl);
	for (i=0; i<bma_LogPtrVec_getSz(&pLog->children); ++i)
	{
		bma_Log *pChild = *bma_LogPtrVec_at(&pLog->children, i);
		bma_assert(pChild != NULL);
		bma_Log_reclcLvl(pChild, lvl);
	}
}

BMA_DEF void bma_Log_reclcLvls(void) {
	assert(g_pRoot != NULL);
	/* read-lock g_createLock for safe iteration over sub-loggers and sinks */
	bma_Rw_lckRd(&g_createLock);
	bma_Log_reclcLvl(g_pRoot, BMA_LOG_LEVEL_OFF);
	bma_Rw_unlckRd(&g_createLock);
}

typedef struct bma_LogCnslSink {
	bma_LogSink base;
	bma_LogLevel level;
} bma_LogCnslSink;

BMA_DEF void bma_LogCnslSink_dtor(bma_LogCnslSink *pSelf, bma_IMemAlloc *pAlloc) {
	(void)pSelf;
	(void)pAlloc;
}

BMA_DEF bma_LogLevel bma_LogCnslSink_getLevel(const void *pSelf_) {
	const bma_LogCnslSink *pSelf = (const bma_LogCnslSink*)pSelf_;
	bma_assert(pSelf != NULL);
	return pSelf->level;
}

BMA_DEF void bma_LogCnslSink_handle(void *pSelf_, bma_LogMsg *pMsg) {
	bma_LogCnslSink *pSelf = (bma_LogCnslSink*)pSelf_;
	bma_assert(pSelf != NULL);
	bma_assert(pMsg != NULL);
	if (pMsg->level < pSelf->level) return;
	printf("[");
	switch (pMsg->level) {
		case BMA_LOG_LEVEL_DEBUG:
			if (g_stdOutIsTerminal) printf("\x1b[34m"); /* blue */
			printf("DEBUG");
			break;
		case BMA_LOG_LEVEL_INFO:
			if (g_stdOutIsTerminal) printf("\x1b[32m"); /* green */
			printf("INFO");
			break;
		case BMA_LOG_LEVEL_WARN:
			if (g_stdOutIsTerminal) printf("\x1b[33m"); /* yellow */
			printf("WARN");
			break;
		case BMA_LOG_LEVEL_ERROR:
			if (g_stdOutIsTerminal) printf("\x1b[31m"); /* red */
			printf("ERROR");
			break;
		case BMA_LOG_LEVEL_FATAL:
			if (g_stdOutIsTerminal) printf("\x1b[31m"); /* red */
			printf("FATAL");
			break;
		default:
			break;
	}
	if (g_stdOutIsTerminal) printf("\x1b[0m"); /* reset color */
	printf(" path %s] (%s, file %s, line %d): %s\n", pMsg->pPath, pMsg->pDateTime, pMsg->pFile, pMsg->line, pMsg->pMsg);
}

BMA_DEF void bma_Log_addCnslSink(const char* pPath, bma_LogLevel level) {
	bma_LogCnslSink *pSink;
	bma_Log *pLog = bma_Log_get(pPath);
	bma_assert(pLog != NULL);
	pSink = bma_malloc_ext(g_pAlloc, bma_LogCnslSink);
	pSink->base.pDtor = (bma_dtor_t)&bma_LogCnslSink_dtor;
	pSink->base.pGetMinLevel = &bma_LogCnslSink_getLevel;
	pSink->base.pHandle = &bma_LogCnslSink_handle;
	pSink->level = level;
	bma_Rw_lckWrt(&g_createLock);
	bma_LogSinkPtrVec_appnd(&pLog->sinks, (bma_LogSink**)&pSink);
	bma_Rw_unlckWrt(&g_createLock);
	bma_Log_reclcLvls();
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_LOG_IMPL */

#ifdef BMA_LOG_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(_WIN32) && !defined(NDEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
#endif

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

void test_create(void) {
	bma_Log *pLog, *pLog2;
	bma_log_init();
	BMA_EXPECT(g_pAlloc == bma_getDfltMemAlloc());
	pLog = bma_Log_get("");
	BMA_EXPECT(pLog == g_pRoot);
	BMA_EXPECT(pLog->pParent == NULL);
	pLog = bma_Log_get("a.b");
	pLog2 = bma_Log_get("a.b");
	BMA_EXPECT(pLog != NULL);
	BMA_EXPECT(pLog == pLog2);
	BMA_EXPECT(strcmp(pLog->pName, "b") == 0);
	pLog = pLog->pParent;
	BMA_EXPECT(pLog != NULL);
	BMA_EXPECT(strcmp(pLog->pName, "a") == 0);
	pLog = pLog->pParent;
	BMA_EXPECT(pLog != NULL);
	BMA_EXPECT(strcmp(pLog->pName, "") == 0);
	bma_log_clnup();
	BMA_EXPECT(g_pRoot == NULL);
	BMA_EXPECT(g_pAlloc == NULL);
}

void test_log(void) {
	bma_Log *pLog;
	bma_log_init();
	pLog = bma_Log_get("a.b");
	bma_Log_addCnslSink("", BMA_LOG_LEVEL_DEBUG);
	bma_Log_info_1(pLog, "The magic number %d", 42);
	bma_log_clnup();
	BMA_EXPECT(g_pRoot == NULL);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
#if defined(_WIN32) && !defined(NDEBUG)
	int flags_ = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	_CrtSetDbgFlag(flags_ | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	(void)argc;
	(void)argv;
	BMA_TEST(test_create);
	BMA_TEST(test_log);
	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_LOG_TEST */

#endif /* BMA_LOG_H_INCLUDED */
