#ifndef BMA_PROC_H_INCLUDED
#define BMA_PROC_H_INCLUDED

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

Sub process management
*/
#include <bma/ccal.h>

#include "utils.h"

#ifdef __cplusplus
	extern "C" {
#endif

/*!
\brief Runs the command pCmd as sub-process and collects the exit code into pExitCode
and all of the stdout and stderr output is piped into pStdOutStdErr.
When the command isn't executable, BMA_FALSE is returned.
The function blocks until the sub process is terminated.
\param pCmd The command which is the executable followed by all of its arguments. The first argument is the executable itself.
\param pExitCode Output parameter receiving the exit code of the process
\param pStdOutStdErr String builder receiving all of what the sub process has put out on stdout and stderr.
       It must be constructed as empty object before calling this function.
*/
BMA_DEF bma_bool_t bma_runCmd(const char *pCmd, int *pExitCode, bma_StrBldr *pStdOutStdErr);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#if (defined(BMA_PROC_IMPL) || defined(BMA_IMPL))

#if BMA_IS_WINDOWS
	#include <limits.h>
	#include <string.h>
	#include <bma/win.h>
#elif BMA_IS_POSIX
	/* TODO */
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#define BMA_PROC_PIPE_READ_CHUNKS 4096U

BMA_DEF bma_bool_t bma_runCmd(const char *pCmd, int *pExitCode, bma_StrBldr *pStdOutStdErr) {
#if BMA_IS_WINDOWS
	bma_bool_t result = BMA_FALSE;
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead = NULL;
	HANDLE hWrite = NULL;
	DWORD bytesRead, exitCode;
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	wchar_t* pWcmdLn = NULL;
	wchar_t* pWide = NULL;
	bma_StrBldr raw;
	UINT cp;
	int wideLen, utf8Len;
	bma_assert(pCmd != NULL);
	bma_assert(pExitCode != NULL);
	bma_assert(pStdOutStdErr != NULL);
	bma_StrBldr_ctor_ext(&raw, bma_StrBldr_getAlloc(pStdOutStdErr));
	*pExitCode = 0;
	if (bma_StrBldr_getSz(pStdOutStdErr) > 0) {
		goto out;
	}
	memset(&sa, 0, sizeof(sa));
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		goto out;
	}
	/* Parent must not pass the read end to child. */
	if (!SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0)) {
		goto out;
	}
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hWrite;
	si.hStdError = hWrite;
	si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	pWcmdLn = bma_strWdup(pCmd);
	if (!CreateProcessW(NULL,
	                    pWcmdLn,
	                    NULL,
	                    NULL,
	                    TRUE,
	                    0,
	                    NULL,
	                    NULL,
	                    &si,
	                    &pi)) {
		goto out;
	}
	CloseHandle(pi.hThread);
	CloseHandle(hWrite);
	hWrite = NULL;
	/* ReadFile returns FALSE once the sub process hung up on the pipe */
	for (;;) {
		size_t oldSz = bma_StrBldr_getSz(&raw);
		bma_StrBldr_rsz(&raw, oldSz + BMA_PROC_PIPE_READ_CHUNKS);
		if (ReadFile(hRead,
	                     (char*)bma_StrBldr_getStr(&raw) + oldSz,
	                     BMA_PROC_PIPE_READ_CHUNKS,
	                     &bytesRead,
	                     NULL)) {
			oldSz += (size_t)bytesRead;
			bma_StrBldr_rsz(&raw, oldSz);
		}
		else {
			bma_StrBldr_rsz(&raw, oldSz);
			bma_StrBldr_rplc(&raw, "\r\n", "\n");
			break;
		}
	}
	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &exitCode);
	BMA_STATIC_ASSERT(sizeof(exitCode) == sizeof(*pExitCode));
	*pExitCode = (int)exitCode;
	CloseHandle(pi.hProcess);
	if (bma_StrBldr_getSz(&raw) > 0) {
		if (bma_StrBldr_getSz(&raw) > (size_t)INT_MAX) {
			goto out;
		}
		cp = GetConsoleOutputCP();
		if (cp == 0) {
			/* parent has no console */
			cp = GetOEMCP();
		}
		if (cp == CP_UTF8) {
			bma_StrBldr_appndStrN(pStdOutStdErr, bma_StrBldr_getStr(&raw), bma_StrBldr_getSz(&raw));
		}
		else {
			wideLen = MultiByteToWideChar(cp, 0, bma_StrBldr_getStr(&raw), (int)bma_StrBldr_getSz(&raw), NULL, 0);
			if (wideLen <= 0) {
				goto out;
			}
			pWide = (*raw.pAlloc->pRllc)(raw.pAlloc, NULL, (size_t)wideLen * sizeof(wchar_t), NULL);
			(void)MultiByteToWideChar(cp, 0, bma_StrBldr_getStr(&raw), (int)bma_StrBldr_getSz(&raw), pWide, wideLen);
			utf8Len = WideCharToMultiByte(CP_UTF8, 0, pWide, wideLen, NULL, 0, NULL, NULL);
			if (utf8Len <= 0) {
				goto out;
			}
			bma_StrBldr_rsz(pStdOutStdErr, (size_t)utf8Len);
			(void)WideCharToMultiByte(CP_UTF8, 0, pWide, wideLen, (char*)bma_StrBldr_getStr(pStdOutStdErr), utf8Len, NULL, NULL);
		}
	}
	result = BMA_TRUE;
out:
	if (pWide != NULL) {
		bma_free_ext(raw.pAlloc, pWide);
	}
	bma_StrBldr_dtor(&raw, NULL);
	if (pWcmdLn != NULL) {
		bma_free(pWcmdLn);
	}
	if (hRead != NULL) {
		CloseHandle(hRead);
	}
	if (hWrite != NULL) {
		CloseHandle(hWrite);
	}
	return result;
#elif BMA_IS_POSIX
#else
	(void)pExe;
	(void)pArgs;
	(void)pExitCode;
	(void)pStdOutStdErr;
	return BMA_FALSE;
#endif
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_PROC_IMPL || BMA_IMPL */

#ifdef BMA_PROC_TEST

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

void test_echo(void) {
	bma_StrBldr out;
	int exitCode = 0;
	bma_StrBldr_ctor(&out);
#if BMA_IS_WINDOWS
	BMA_EXPECT(bma_runCmd("cmd /C \"echo hello\"", &exitCode, &out));
#endif
	BMA_EXPECT(exitCode == 0);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&out), "hello\n") == 0);
	bma_StrBldr_dtor(&out, NULL);
}

void test_echoNonAscii(void) {
	bma_StrBldr out;
	int exitCode = 0;
	bma_StrBldr_ctor(&out);
#if BMA_IS_WINDOWS
	/* UTF-8 for "hällo" */
	BMA_EXPECT(bma_runCmd("cmd /C \"echo h\xC3\xA4llo\"", &exitCode, &out));
#endif
	BMA_EXPECT(exitCode == 0);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&out), "h\xC3\xA4llo\n") == 0);
	bma_StrBldr_dtor(&out, NULL);
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_echo);
	BMA_TEST(test_echoNonAscii);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_PROC_TEST */

#endif /* BMA_PROC_H_INCLUDED */
