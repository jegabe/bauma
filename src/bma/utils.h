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
#include <bma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef _WIN32

/* Convert UTF-8 to UTF-16LE and return copy on heap */
BMA_DEF wchar_t *bma_strWdup(const char *pStr);

/* Convert UTF-16LE to UTF-8 and return copy on heap */
BMA_DEF char *bma_wStrDup(const wchar_t *pStr);

#endif

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_UTILS_IMPL

#include <wchar.h>

#ifdef _WIN32
	/* Windows.h is tremendously huge. Trying to get it down a bit. */
	#ifndef BMA_FULL_WINDOWS_H
		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN
			#define BMA_UNDEF_WIN32_LEAN_AND_MEAN
		#endif
		#ifndef NOMINMAX
			#define NOMINMAX
			#define BMA_UNDEF_NOMINMAX
		#endif
	#endif
	#include <Windows.h>
	/* Clean-up of temporary macro definitions */
	#ifdef BMA_UNDEF_WIN32_LEAN_AND_MEAN
		#undef WIN32_LEAN_AND_MEAN
		#undef BMA_UNDEF_WIN32_LEAN_AND_MEAN
	#endif
	#ifdef BMA_UNDEF_NOMINMAX
		#undef NOMINMAX
		#undef BMA_UNDEF_NOMINMAX
	#endif
#else
	#include <stdlib.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef _WIN32

BMA_DEF wchar_t *bma_strWdup(const char *pStr) {
	size_t len;
	int numChars;
	wchar_t *pResult;
	bma_IMemAlloc *pAlloc = bma_getDfltMemAlloc();
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

BMA_DEF char *bma_wStrDup(const wchar_t *pStr) {
	size_t len;
	int numChars;
	char *pResult;
	bma_IMemAlloc *pAlloc = bma_getDfltMemAlloc();
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

#endif

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_UTILS_IMPL */

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

void test_dummy(void) {
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

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
