#ifndef BMA_UTILS_H_INCLUDED
#define BMA_UTILS_H_INCLUDED

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

Random utils
*/
#include <time.h>
#include <bma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct bma_MsTime {
	time_t sec;
	unsigned int msec;
} bma_MsTime;

#if BMA_IS_WINDOWS

/* Convert UTF-8 to UTF-16LE and return copy on heap */
BMA_DEF wchar_t *bma_strWdup_ext(const char *pStr, bma_IMemAlloc *pAlloc);

#define bma_strWdup(pStr) bma_strWdup_ext(pStr, bma_getDfltMemAlloc())

/* Convert UTF-16LE to UTF-8 and return copy on heap */
BMA_DEF char *bma_wStrDup_ext(const wchar_t *pStr, bma_IMemAlloc *pAlloc);

#define bma_wStrDup(pStr) bma_wStrDup_ext(pStr, bma_getDfltMemAlloc())

/*!
\brief Returns size of a Windows double-null-terminated string, as one
returned by the function GetEnvironmentStringsW(),
without counting the two terminating null's
*/
BMA_DEF size_t bma_dblNllTrmStrLen(const wchar_t *p);

#endif

#ifdef __cplusplus
	} /* extern "C" */
#endif

#if (defined(BMA_UTILS_IMPL) || defined(BMA_IMPL))

#include <wchar.h>

#if BMA_IS_WINDOWS
	#include <bma/win.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#if BMA_IS_WINDOWS

BMA_DEF wchar_t *bma_strWdup_ext(const char *pStr, bma_IMemAlloc *pAlloc) {
	size_t len;
	int numChars;
	wchar_t *pResult;
	bma_assert(pStr != NULL);
	bma_assert(pAlloc != NULL);
	len = strlen(pStr);
	if (len == 0) {
		pResult = (*pAlloc->pRllc)(pAlloc, NULL, sizeof(wchar_t), NULL);
		*pResult = L'\0';
		return pResult;
	}
	numChars = MultiByteToWideChar(CP_UTF8, 0, pStr, (int)len, NULL, 0);
	if (numChars <= 0) bma_exit_err("MultiByteToWideChar failed");
	pResult = (*pAlloc->pRllc)(pAlloc, NULL, (size_t)((numChars + 1) * sizeof(wchar_t)), NULL);
	(void)MultiByteToWideChar(CP_UTF8, 0, pStr, (int)len, pResult, numChars);
	pResult[numChars] = L'\0';
	return pResult;
}

BMA_DEF char *bma_wStrDup_ext(const wchar_t *pStr, bma_IMemAlloc *pAlloc) {
	size_t len;
	int numChars;
	char *pResult;
	bma_assert(pStr != NULL);
	bma_assert(pAlloc != NULL);
	len = wcslen(pStr);
	if (len == 0) {
		pResult = (*pAlloc->pRllc)(pAlloc, NULL, sizeof(char), NULL);
		*pResult = '\0';
		return pResult;
	}
	numChars = WideCharToMultiByte(CP_UTF8, 0, pStr, (int)len, NULL, 0, NULL, NULL);
	if (numChars <= 0) bma_exit_err("WideCharToMultiByte failed");
	pResult = (*pAlloc->pRllc)(pAlloc, NULL, (size_t)((numChars + 1) * sizeof(char)), NULL);
	(void)WideCharToMultiByte(CP_UTF8, 0, pStr, (int)len, pResult, numChars, NULL, NULL);
	pResult[numChars] = '\0';
	return pResult;
}

BMA_DEF size_t bma_dblNllTrmStrLen(const wchar_t *p) {
	size_t len = 0;
	bma_assert(p != NULL);
	for (;;) {
		if ((p[0] == '\0') && (p[1] == '\0')) return len;
		++p;
		++len;
	}
}

#endif

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_UTILS_IMPL || BMA_IMPL */

#ifdef BMA_UTILS_TEST

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

void test_dummy(void) {
	BMA_EXPECT(1 == 1);
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_dummy);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_FS_TEST */

#endif /* BMA_FS_H_INCLUDED */
