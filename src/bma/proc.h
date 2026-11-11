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

#ifdef __cplusplus
	extern "C" {
#endif

/*!
\brief Runs the command pCmd as sub-process and collects the exit code into pExitCode
and all of the stdout and stderr output is piped into pOut.
When the command isn't executable, BMA_FALSE is returned.
The function blocks until the sub process is terminated.
\param pCmd The command which is the executable followed by all of its arguments. The first argument is the executable itself.
\param pExitCode Output parameter receiving the exit code of the process
\param pOut String builder receiving all of what the sub process has put out on stdout and stderr.
       On POSIX, captured bytes are forwarded unchanged; the command is expected to emit UTF-8.
       On Windows, captured bytes are assumed to be in the current code page and are converted to UTF-8.
\return BMA_TRUE when the command was executed and terminated, BMA_FALSE when the command
*/
BMA_DEF bma_bool_t bma_runCmd(const char *pCmd, int *pExitCode, bma_StrBldr *pOut);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#if (defined(BMA_PROC_IMPL) || defined(BMA_IMPL))

#if BMA_IS_WINDOWS
	#include <limits.h>
	#include <string.h>
	#include <bma/win.h>
	#include <bma/utils.h>
#elif BMA_IS_POSIX
	#include <ctype.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <limits.h>
	#include <string.h>
	#include <sys/resource.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <unistd.h>
	#if (defined(__GLIBC__) && defined(__GLIBC_PREREQ) && __GLIBC_PREREQ(2, 34)) || \
	    defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
	    defined(__DragonFly__) || defined(__APPLE__)
		#define BMA_PROC_HAS_CLOSEFROM 1
		extern void closefrom(int);
	#else
		#define BMA_PROC_HAS_CLOSEFROM 0
	#endif
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#define BMA_PROC_PIPE_READ_CHUNKS 4096U

#if BMA_IS_POSIX

BMA_DEF_VEC(bma_ProcArgVec, char*, NULL)

BMA_DEF void bma_proc_closeExtraFds(int firstFd) {
#if BMA_PROC_HAS_CLOSEFROM
	closefrom(firstFd);
#else
	long maxFd = sysconf(_SC_OPEN_MAX);
	long fd;
	if (maxFd < 0) {
		struct rlimit limit;
		if (getrlimit(RLIMIT_NOFILE, &limit) == 0 &&
		    limit.rlim_cur != RLIM_INFINITY) {
			maxFd = (limit.rlim_cur > (rlim_t)INT_MAX)
				? (long)INT_MAX
				: (long)limit.rlim_cur;
		}
		else {
			maxFd = 1024;
		}
	}
	if (maxFd > INT_MAX) {
		maxFd = INT_MAX;
	}
	for (fd = firstFd; fd < (int)maxFd; ++fd) {
		(void)close((int)fd);
	}
#endif
}

BMA_DEF bma_bool_t bma_proc_moveFdAboveStdio(int *pFd) {
	int newFd;
	if (*pFd >= 3) {
		return BMA_TRUE;
	}
	newFd = fcntl(*pFd, F_DUPFD, 3);
	if (newFd < 0) {
		return BMA_FALSE;
	}
	(void)close(*pFd);
	*pFd = newFd;
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_proc_parseCmd(
	const char *pCmd,
	bma_ProcArgVec *pArgv,
	char **ppStorage,
	bma_IMemAlloc *pAlloc
) {
	size_t cmdLen = strlen(pCmd);
	size_t i = 0;
	size_t out = 0;
	char *storage;
	storage = (char*)(*pAlloc->pRllc)(pAlloc, NULL, cmdLen + 1U, NULL);
	*ppStorage = storage;
	while (i < cmdLen) {
		bma_bool_t inQuotes = BMA_FALSE;
		bma_bool_t tokenStarted = BMA_FALSE;
		char *pArg;
		while (i < cmdLen && isspace((unsigned char)pCmd[i])) {
			++i;
		}
		if (i == cmdLen) {
			break;
		}
		pArg = storage + out;
		bma_ProcArgVec_appnd(pArgv, &pArg);
		while (i < cmdLen) {
			char c = pCmd[i];
			if (c == '\\' && i + 1U < cmdLen && pCmd[i + 1U] == '"') {
				storage[out++] = '"';
				i += 2U;
				tokenStarted = BMA_TRUE;
			}
			else if (c == '"') {
				inQuotes = !inQuotes;
				++i;
				tokenStarted = BMA_TRUE;
			}
			else if (!inQuotes && isspace((unsigned char)c)) {
				break;
			}
			else {
				storage[out++] = c;
				++i;
				tokenStarted = BMA_TRUE;
			}
		}
		if (inQuotes || !tokenStarted) {
			return BMA_FALSE;
		}
		storage[out++] = '\0';
	}
	if (bma_ProcArgVec_getSz(pArgv) == 0) {
		return BMA_FALSE;
	}
	{
		char *pArg = NULL;
		bma_ProcArgVec_appnd(pArgv, &pArg);
	}
	return BMA_TRUE;
}

BMA_DEF void bma_proc_writeExecError(int errFd, int errorCode) {
	const char *pData = (const char*)&errorCode;
	size_t written = 0;
	while (written < sizeof(errorCode)) {
		ssize_t n = write(errFd, pData + written, sizeof(errorCode) - written);
		if (n > 0) {
			written += (size_t)n;
		}
		else if (n < 0 && errno == EINTR) {
			continue;
		}
		else {
			break;
		}
	}
}

#endif /* BMA_IS_POSIX */

BMA_DEF bma_bool_t bma_runCmd(const char *pCmd, int *pExitCode, bma_StrBldr *pOut) {
#if BMA_IS_WINDOWS
	bma_bool_t result = BMA_FALSE;
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead = NULL;
	HANDLE hWrite = NULL;
	DWORD bytesRead, exitCode;
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	wchar_t* pWcmdLn = NULL;
	bma_StrBldr raw;
	bma_assert(pCmd != NULL);
	bma_assert(pExitCode != NULL);
	bma_assert(pOut != NULL);
	bma_StrBldr_ctor_ext(&raw, bma_StrBldr_getAlloc(pOut));
	*pExitCode = 0;
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
		UINT cp;
		if (bma_StrBldr_getSz(&raw) > (size_t)INT_MAX) {
			goto out;
		}
		cp = GetConsoleOutputCP();
		if (cp == 0) {
			/* parent has no console */
			cp = GetOEMCP();
		}
		if (cp != CP_UTF8) {
			wchar_t *pWide;
			int wideLen, utf8Len;
			/* Convert to native windows (UTF-16) and back to UTF-8*/
			wideLen = MultiByteToWideChar(cp, 0, bma_StrBldr_getStr(&raw), (int)bma_StrBldr_getSz(&raw), NULL, 0);
			if (wideLen <= 0) {
				goto out;
			}
			pWide = (*raw.pAlloc->pRllc)(raw.pAlloc, NULL, (size_t)wideLen * sizeof(wchar_t), NULL);
			(void)MultiByteToWideChar(cp, 0, bma_StrBldr_getStr(&raw), (int)bma_StrBldr_getSz(&raw), pWide, wideLen);
			utf8Len = WideCharToMultiByte(CP_UTF8, 0, pWide, wideLen, NULL, 0, NULL, NULL);
			if (utf8Len <= 0) {
				bma_free_ext(raw.pAlloc, pWide);
				goto out;
			}
			bma_StrBldr_rsz(&raw, (size_t)utf8Len);
			(void)WideCharToMultiByte(CP_UTF8, 0, pWide, wideLen, (char*)bma_StrBldr_getStr(&raw), utf8Len, NULL, NULL);
			bma_free_ext(raw.pAlloc, pWide);
		}
		bma_StrBldr_appndStrN(pOut, bma_StrBldr_getStr(&raw), bma_StrBldr_getSz(&raw));
	}
	result = BMA_TRUE;
out:
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
	int outPipe[2] = {-1, -1};
	int errPipe[2] = {-1, -1};
	bma_ProcArgVec argv;
	char *argvStorage = NULL;
	bma_IMemAlloc *pAlloc = bma_getDfltMemAlloc();
	pid_t pid;
	bma_bool_t result = BMA_FALSE;
	bma_bool_t readOk = BMA_TRUE;
	int execError = 0;
	size_t execErrorBytes = 0;
	int status;
	char buf[BMA_PROC_PIPE_READ_CHUNKS];
	bma_assert(pCmd != NULL);
	bma_assert(pExitCode != NULL);
	bma_assert(pOut != NULL);
	*pExitCode = 0;
	bma_ProcArgVec_ctor_ext(&argv, pAlloc);
	if (!bma_proc_parseCmd(pCmd, &argv, &argvStorage, pAlloc)) {
		goto out;
	}
	if (pipe(outPipe) != 0 || pipe(errPipe) != 0) {
		goto out;
	}
	if (!bma_proc_moveFdAboveStdio(&outPipe[0]) ||
	    !bma_proc_moveFdAboveStdio(&outPipe[1]) ||
	    !bma_proc_moveFdAboveStdio(&errPipe[0]) ||
	    !bma_proc_moveFdAboveStdio(&errPipe[1]) ||
	    fcntl(outPipe[0], F_SETFD, FD_CLOEXEC) != 0 ||
	    fcntl(outPipe[1], F_SETFD, FD_CLOEXEC) != 0 ||
	    fcntl(errPipe[0], F_SETFD, FD_CLOEXEC) != 0 ||
	    fcntl(errPipe[1], F_SETFD, FD_CLOEXEC) != 0) {
		goto out;
	}
	pid = fork();
	if (pid < 0) {
		goto out;
	}
	if (pid == 0) {
		char **pArgv = bma_ProcArgVec_getData(&argv);
		int childErrFd;
		int childErr;
		(void)close(outPipe[0]);
		(void)close(errPipe[0]);
		if (dup2(outPipe[1], STDOUT_FILENO) < 0 ||
		    dup2(outPipe[1], STDERR_FILENO) < 0) {
			childErr = errno;
			bma_proc_writeExecError(errPipe[1], childErr);
			_exit(127);
		}
		(void)close(outPipe[1]);
		childErrFd = (errPipe[1] == 3) ? 3 : dup2(errPipe[1], 3);
		if (childErrFd < 0 || fcntl(3, F_SETFD, FD_CLOEXEC) != 0) {
			childErr = errno;
			bma_proc_writeExecError(errPipe[1], childErr);
			_exit(127);
		}
		if (errPipe[1] != 3) {
			(void)close(errPipe[1]);
		}
		/* Keep only the close-on-exec error channel in addition to stdio. */
		bma_proc_closeExtraFds(4);
		execvp(pArgv[0], pArgv);
		childErr = errno;
		bma_proc_writeExecError(3, childErr);
		_exit(127);
	}
	(void)close(outPipe[1]);
	outPipe[1] = -1;
	(void)close(errPipe[1]);
	errPipe[1] = -1;
	for (;;) {
		ssize_t n = read(errPipe[0], (char*)&execError + execErrorBytes,
		                 sizeof(execError) - execErrorBytes);
		if (n > 0) {
			execErrorBytes += (size_t)n;
			if (execErrorBytes == sizeof(execError)) {
				break;
			}
		}
		else if (n == 0) {
			break;
		}
		else if (errno != EINTR) {
			readOk = BMA_FALSE;
			break;
		}
	}
	(void)close(errPipe[0]);
	errPipe[0] = -1;
	for (;;) {
		ssize_t n = read(outPipe[0], buf, sizeof(buf));
		if (n > 0) {
			bma_StrBldr_appndStrN(pOut, buf, (size_t)n);
		}
		else if (n == 0) {
			break;
		}
		else if (errno != EINTR) {
			readOk = BMA_FALSE;
			break;
		}
	}
	(void)close(outPipe[0]);
	outPipe[0] = -1;
	while (waitpid(pid, &status, 0) < 0) {
		if (errno != EINTR) {
			goto out;
		}
	}
	if (!readOk || execErrorBytes != 0 ||
	    (!WIFEXITED(status) && !WIFSIGNALED(status))) {
		goto out;
	}
	if (WIFEXITED(status)) {
		*pExitCode = WEXITSTATUS(status);
	}
	else {
		*pExitCode = 128 + WTERMSIG(status);
	}
	result = (execErrorBytes == 0) ? BMA_TRUE : BMA_FALSE;
out:
	if (outPipe[0] >= 0) (void)close(outPipe[0]);
	if (outPipe[1] >= 0) (void)close(outPipe[1]);
	if (errPipe[0] >= 0) (void)close(errPipe[0]);
	if (errPipe[1] >= 0) (void)close(errPipe[1]);
	bma_ProcArgVec_dtor(&argv, NULL);
	bma_free_ext(pAlloc, argvStorage);
	return result;

#else
	(void)pCmd;
	(void)pExitCode;
	(void)pOut;
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
#elif BMA_IS_POSIX
	BMA_EXPECT(bma_runCmd("sh -c \"echo hello\"", &exitCode, &out));
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
	/* UTF-8 for "h[ae]llo" */
	BMA_EXPECT(bma_runCmd("cmd /C \"echo h\xC3\xA4llo\"", &exitCode, &out));
#elif BMA_IS_POSIX
	BMA_EXPECT(bma_runCmd("sh -c \"echo h\xC3\xA4llo\"", &exitCode, &out));
#endif
	BMA_EXPECT(exitCode == 0);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&out), "h\xC3\xA4llo\n") == 0);
	bma_StrBldr_dtor(&out, NULL);
}

void test_quotedArgsAndExitCode(void) {
#if BMA_IS_POSIX
	bma_StrBldr out;
	int exitCode = 0;
	bma_StrBldr_ctor(&out);
	BMA_EXPECT(bma_runCmd("printf \"%s|%s\" \"two words\" \"\"", &exitCode, &out));
	BMA_EXPECT(exitCode == 0);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&out), "two words|") == 0);
	bma_StrBldr_clear(&out);
	BMA_EXPECT(bma_runCmd("sh -c \"echo out; echo err >&2\"", &exitCode, &out));
	BMA_EXPECT(exitCode == 0);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&out), "out\nerr\n") == 0);
	bma_StrBldr_clear(&out);
	BMA_EXPECT(bma_runCmd("sh -c \"exit 7\"", &exitCode, &out));
	BMA_EXPECT(exitCode == 7);
	bma_StrBldr_dtor(&out, NULL);
#endif
}

void test_missingCommand(void) {
#if BMA_IS_POSIX
	bma_StrBldr out;
	int exitCode = 0;
	bma_StrBldr_ctor(&out);
	BMA_EXPECT(!bma_runCmd("bma-command-that-does-not-exist-92841", &exitCode, &out));
	bma_StrBldr_dtor(&out, NULL);
#endif
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_echo);
	BMA_TEST(test_echoNonAscii);
	BMA_TEST(test_quotedArgsAndExitCode);
	BMA_TEST(test_missingCommand);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_PROC_TEST */

#endif /* BMA_PROC_H_INCLUDED */
