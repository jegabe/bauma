#ifndef BMA_FS_H_INCLUDED
#define BMA_FS_H_INCLUDED

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

File system utilities
*/
#include <stdio.h>
#include <bma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

BMA_DEF FILE *bma_fopen_rd(const char *pPath);
BMA_DEF FILE *bma_fopen_wrt(const char *pPath);

BMA_DEF bma_bool_t bma_rdFile(const char *pPath, bma_StrBldr *pOut);
BMA_DEF bma_bool_t bma_wrtFile(const char *pPath, const char *pData, size_t dataSz);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_FS_IMPL

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
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef _WIN32

BMA_DEF wchar_t *bma_fsStrWdup(const char *pStr) {
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

#endif

BMA_DEF FILE *bma_fopen_rd(const char *pPath) {
	#ifdef _WIN32
		wchar_t *pWidePath;
		bma_assert(pPath != NULL);
		/* Under windows, the default narrow encoding isn't UTF-8,
		   so we use the wchar_t variant to get the file names right */
		pWidePath = bma_fsStrWdup(pPath);
		FILE *pFile = _wfopen(pWidePath, L"rb");
		bma_free(pWidePath);
		return pFile;
	#else
		bma_assert(pPath != NULL);
		return fopen(pPath, "rb");
	#endif
}

BMA_DEF FILE *bma_fopen_wrt(const char *pPath) {
	#ifdef _WIN32
		wchar_t *pWidePath;
		bma_assert(pPath != NULL);
		/* Under windows, the default narrow encoding isn't UTF-8,
		   so we use the wchar_t variant to get the file names right */
		pWidePath = bma_fsStrWdup(pPath);
		FILE *pFile = _wfopen(pWidePath, L"wb");
		bma_free(pWidePath);
		return pFile;
	#else
		bma_assert(pPath != NULL);
		return fopen(pPath, "wb");
	#endif
}

BMA_DEF bma_bool_t bma_rdFile(const char *pPath, bma_StrBldr *pOut) {
	FILE *pFile;
#ifdef _WIN32
	long long fileSize;
#else
	long fileSize;
#endif
	size_t sFileSize;
	size_t numRead;
	bma_assert(pPath != NULL);
	bma_assert(pOut != NULL);
	pFile = bma_fopen_rd(pPath);
	if (pFile == NULL) return BMA_FALSE;
	if (fseek(pFile, 0, SEEK_END) != 0) {
		(void)fclose(pFile);
		return BMA_FALSE;
	}
#ifdef _WIN32
	fileSize = _ftelli64(pFile);
#else	
	fileSize = ftell(pFile);
#endif
	if (fileSize < 0) {
		(void)fclose(pFile);
		return BMA_FALSE;
	}
	(void)fseek(pFile, 0L, SEEK_SET);
	sFileSize = (size_t)fileSize;
	bma_StrBldr_rsz(pOut, sFileSize);
	numRead = fread((char*)bma_StrBldr_getStr(pOut), 1, sFileSize, pFile);
	(void)fclose(pFile);
	if (numRead != sFileSize) {
		return BMA_FALSE;
	}
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_wrtFile(const char *pPath, const char *pData, size_t dataSz) {
	FILE *pFile;
	size_t numWritten;
	bma_assert(pPath != NULL);
	bma_assert((pData != NULL) || (dataSz == 0));
	pFile = bma_fopen_wrt(pPath);
	if (pFile == NULL) return BMA_FALSE;
	numWritten = fwrite(pData, 1, dataSz, pFile);
	(void)fclose(pFile);
	return (numWritten == dataSz);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_FS_IMPLEMENTATION */

#ifdef BMA_FS_TEST

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

void test_readWholeFile(void) {
	bma_StrBldr b;
	bma_StrBldr_ctor(&b);
#ifdef _WIN32
	BMA_EXPECT(bma_rdFile("C:\\Windows\\System32\\kernel32.dll", &b));
#else
	BMA_EXPECT(bma_rdFile("/etc/hosts", &b));
#endif
	BMA_EXPECT(bma_StrBldr_getStr(&b) != NULL);
	BMA_EXPECT(bma_StrBldr_getSz(&b) > 0);
	bma_StrBldr_dtor(&b, NULL);
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_readWholeFile);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_FS_TEST */

#endif /* BMA_FS_H_INCLUDED */
