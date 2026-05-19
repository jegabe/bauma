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
#include <bma/thrd.h>

#ifdef __cplusplus
	extern "C" {
#endif

BMA_DEF void bma_LogPtr_dtor(bma_Log **ppLog, bma_IMemAlloc *pAlloc);

BMA_DEF_VEC(bma_LogPtrVec, bma_Log*, (bma_dtor_t)&bma_LogPtr_dtor)

struct bma_Log {
	char *pFullName;
	char *pName; /* points into pFullName and shared the same memory */
	bma_Log *pParent;
	bma_LogPtrVec children;
};

static bma_Rw g_createLock;
static bma_Log *g_pRoot = NULL;

BMA_DEF	void bma_Log_dtor(bma_Log *pSelf, bma_IMemAlloc *pAlloc) {
	(void)pAlloc;
	bma_LogPtrVec_dtor(&pSelf->children, NULL);
	if (pSelf != g_pRoot) {
		bma_free(pSelf->pFullName);
	}
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF void bma_LogPtr_dtor(bma_Log **ppLog, bma_IMemAlloc *pAlloc) {
	bma_assert(ppLog != NULL);
	bma_assert(*ppLog != NULL);
	bma_Log_dtor(*ppLog, pAlloc);
	bma_free_ext(pAlloc, *ppLog);
}

BMA_DEF void bma_log_init(void) {
	bma_assert(g_pRoot == NULL);
	bma_Rw_ctor(&g_createLock);
	bma_Rw_lckWrt(&g_createLock);
	g_pRoot = bma_malloc(bma_Log);
	g_pRoot->pFullName = (char*)"";
	g_pRoot->pName = g_pRoot->pFullName;
	g_pRoot->pParent = NULL;
	bma_LogPtrVec_ctor(&g_pRoot->children);
	bma_Rw_unlckWrt(&g_createLock);
}

BMA_DEF void bma_log_clnup(void) {
	bma_assert(g_pRoot != NULL);
	bma_Rw_lckWrt(&g_createLock);
	bma_Log_dtor(g_pRoot, NULL);
	bma_free(g_pRoot);
	g_pRoot = NULL;
	bma_Rw_unlckWrt(&g_createLock);
	bma_Rw_dtor(&g_createLock, NULL);
}

static bma_bool_t bma_log_sameStr(const char *p0, size_t l0, const char *p1, size_t l1) {
	return (l0 == l1) && (memcmp(p0, p1, l0) == 0);
}

BMA_DEF bma_Log *bma_Log_getOrCreate(const char *pPath, size_t pathLen, bma_bool_t create) {
	bma_bool_t end = BMA_FALSE;
	bma_Log *pResult;
	const char *pFullPath = pPath;
	size_t fullPathLen = pathLen;
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
			pNew = bma_malloc(bma_Log);
			pNew->pFullName = bma_strndup(pFullPath, fullPathLen);
			pNew->pName = pNew->pFullName + (pPath - pFullPath);
			pNew->pParent = pResult;
			bma_LogPtrVec_ctor(&pNew->children);
			bma_LogPtrVec_appnd(&pResult->children, &pNew);
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
	(void)pLog;
	(void)level;
	return BMA_TRUE;
}

static const char *bma_Log_stripPath(const char *pFileName) {
	size_t l;
	bma_assert(pFileName != NULL);
	l = strlen(pFileName);
	while (l-- > 0) {
		if (pFileName[l] == '/' || pFileName[l] == '\\') {
			return &pFileName[l + 1u];
		}
	}
	return pFileName;
}

BMA_DEF void bma_Log_log_impl_(bma_Log *pLog, bma_LogLevel level, const char *pFile, int line, const char *pFmt, ...) {
	va_list ap;
	(void)level;
	/*
	if (!bma_Log_isLoggable_impl_(pLog, level)) {
		return;
	}
	*/
	fprintf(stdout, "[%s] (file %s line %d): ", pLog->pFullName, bma_Log_stripPath(pFile), line);
	va_start(ap, pFmt);
	vfprintf(stdout, pFmt, ap);
	fprintf(stdout, "\n");
	va_end(ap);
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

void test_create(void) {
	bma_Log *pLog, *pLog2;
	bma_log_init();
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
}

void test_log(void) {
	bma_Log *pLog;
	bma_log_init();
	pLog = bma_Log_get("a.b");
	bma_Log_info_1(pLog, "The magic number %d", 42);
	bma_log_clnup();
	BMA_EXPECT(g_pRoot == NULL);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
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
