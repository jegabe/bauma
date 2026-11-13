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
\brief Set of operating system environment variables.
Implementation is platform dependent to be able to
quickly launch new processes with a modified environment
without too much conversion involved.
Don't use the members directly, use the bma_Env_* functions instead.
*/
typedef struct bma_Env {
#if BMA_IS_WINDOWS
	wchar_t* pEnvBlock; /* double null terminated string list as created by GetEnvironmentStringsW() API */
	size_t sz;
	size_t cap; /* capacity of pEnvBlock including the double null terminator */
#elif BMA_IS_POSIX
	char **pEnvp; /* NULL terminated array of strings as created by execve() API */
	size_t num; /* number of strings in pEnvp excluding the final NULL pointer */
#else
	#error "Unsupported platform"
#endif
} bma_Env;

#if BMA_IS_WINDOWS
	/*! \brief Separator for multiple values in one env. var., like "Path" in Windows */
	#define BMA_ENV_VAL_SEP ";"
#elif BMA_IS_POSIX
	/*! \brief Separator for multiple values in one env. var., like "PATH" in POSIX */
	#define BMA_ENV_VAL_SEP ":"
#endif

BMA_DEF void bma_Env_ctor(bma_Env *pEnv);
BMA_DEF void bma_Env_dtor(bma_Env *pEnv, bma_IMemAlloc *pAlloc);
BMA_DEF void bma_Env_rdFrmOs(bma_Env *pEnv);
BMA_DEF void bma_Env_get(bma_Env *pEnvVars, const char *pName, bma_StrBldr *pOutValue);
BMA_DEF void bma_Env_set(bma_Env *pEnvVars, const char *pName, const char *pValue);
/* Joins the existing value (if any) and pValue with ';'(Win)/':'(POSIX) and stores the result */
BMA_DEF void bma_EnvVars_jn(bma_Env *pEnv, const char *pName, const char *pValue, bma_bool_t prepend);

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
	#include <wchar.h>
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
	#ifdef __cplusplus
		extern "C" char **environ;
	#else
		extern char **environ;
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

#if BMA_IS_WINDOWS

static const wchar_t bma_kNullStr[2] = { L'\0', L'\0' };
#define BMA_ENV_NULLSTR ((wchar_t*)&bma_kNullStr[0])
#define BMA_ENV_EXTRA_CAP 256U

/* Grows pEnvBlock so that it can hold at least minCap wchar_t's */
static void bma_Env_ensureCap(bma_Env *pEnv, size_t minCap) {
	bma_IMemAlloc *pAlloc;
	wchar_t *pOld, *pNew;
	size_t realSz = 0;
	if (minCap <= pEnv->cap) return BMA_TRUE;
	pAlloc = bma_getDfltMemAlloc();
	if (pEnv->pEnvBlock == BMA_ENV_NULLSTR) {
		pOld = NULL;
	}
	else {
		pOld = pEnv->pEnvBlock;
	}
	pNew = (wchar_t*)(*pAlloc->pRllc)(pAlloc, pOld, (minCap + BMA_ENVVARS_EXTRA_CAP) * sizeof(wchar_t), &realSz);
	pEnv->pEnvBlock = pNew;
	pEnv->cap = realSz / sizeof(wchar_t);
}

#elif BMA_IS_POSIX

static const char *const bma_kNullEnv[1] = { NULL };
#define BMA_NULL_ENV ((char**)&bma_kNullEnv[0])

#endif /* BMA_IS_... */

BMA_DEF void bma_Env_ctor(bma_Env *pEnv) {
#if BMA_IS_WINDOWS
	bma_assert(pEnv != NULL);
	pEnv->pEnvBlock = BMA_ENV_NULLSTR;
	pEnv->sz = 0;
	pEnv->cap = 0;
#elif BMA_IS_POSIX
	pEnv->pEnvp = BMA_NULL_ENV;
	pEnv->num = 0;
#endif
}

BMA_DEF void bma_Env_dtor(bma_Env *pEnv, bma_IMemAlloc *pAlloc_) {
#if BMA_IS_WINDOWS
	bma_IMemAlloc *pAlloc;
	bma_assert(pEnv != NULL);
	(void)pAlloc_;
	pAlloc = bma_getDfltMemAlloc();
	if (pEnv->pEnvBlock != BMA_ENV_NULLSTR) {
		pAlloc = bma_getDfltMemAlloc();
		(*pAlloc->pRllc)(pAlloc, pEnv->pEnvBlock, 0, NULL);
	}
#elif BMA_IS_POSIX
	bma_IMemAlloc *pAlloc;
	size_t i;
	bma_assert(pEnv != NULL);
	(void)pAlloc_;
	pAlloc = bma_getDfltMemAlloc();
	i = pEnv->num;
	while(i-- > 0) {
		(*pAlloc->pRllc)(pAlloc, pEnv->pEnvp[i], 0, NULL);
	}
	(*pAlloc->pRllc)(pAlloc, pEnv->pEnvp, 0, NULL);
#endif
#if BMA_DBG
	memset(pEnv, 0xFF, sizeof(*pEnv));
#endif
}

BMA_DEF void bma_Env_rdFrmOs(bma_Env *pEnv) {
#if BMA_IS_WINDOWS
	bma_IMemAlloc *pAlloc;
	const wchar_t *pCurr, *p;
	size_t len;
	bma_assert(pEnv != NULL);
	pAlloc = bma_getDfltMemAlloc();
	pCurr = GetEnvironmentStringsW();
	bma_assert(pCurr != NULL);
	len = bma_dblNllTrmStrLen(pCurr);
	bma_Env_ensureCap(pEnv, len + 1U);
	memcpy(pEnv->pEnvBlock, pCurr, (len + 1U) * sizeof(wchar_t));
	pEnv->sz = len;
#elif BMA_IS_POSIX
	bma_IMemAlloc *pAlloc;
	char **p;
	size_t i;
	size_t len = 0;
	bma_assert(pEnv != NULL);
	pAlloc = bma_getDfltMemAlloc();
	if (pEnv->pEnvp != BMA_NULL_ENV) {
		i = pEnv->num;
		while(i-- > 0) {
			(*pAlloc->pRllc)(pAlloc, pEnv->pEnvp[i], 0, NULL);
		}
		(*pAlloc->pRllc)(pAlloc, pEnv->pEnvp, 0, NULL);
	}
	for (p = environ; *p != NULL; p++) {
		++len;
	}
	pEnv->pEnvp = (char**)(pAlloc->pRllc)(pAlloc, NULL, (len + 1U) * sizeof(char*), NULL);
	p = environ;
	for (i=0; i<len; ++i) {
		pEnv->pEnvp[i] = bma_strdup_ext(*p, pAlloc);
		++p;
	}
	pEnv->pEnvp[len] = NULL;
	pEnv->num = len;
#endif
}

BMA_DEF void bma_Env_get(bma_Env *pEnv, const char *pName, bma_StrBldr *pOutValue) {
#if BMA_IS_WINDOWS
	wchar_t *pWideName, *p;
	size_t wideNameLen;
	bma_assert(pEnv != NULL);
	bma_assert(pName != NULL);
	bma_assert(pOutValue != NULL);
	pWideName = bma_strWdup(pName);
	wideNameLen = wcslen(pWideName);
	for (p = pEnv->pEnvBlock; (p[0] != L'\0'); p = p + wcslen(p) + 1U) {
		const wchar_t *pEq;
		size_t len;
		if (p[0] == L'=') continue;
		pEq = wcschr(p, L'=');
		if (pEq == NULL) continue;
		len = (size_t)(pEq - p);
		/* Windows env var names are case insensitive */
		if ((len == wideNameLen) && (_wcsnicmp(p, pWideName, wideNameLen) == 0)) {
			char *pVal;
			++pEq; /* skip "=" to go to value */
			pVal = bma_wStrDup(pEq);
			bma_StrBldr_appndStr(pOutValue, pVal);
			bma_free(pVal);
			goto out;
		}
	}
out:
	bma_free(pWideName);
#elif BMA_IS_POSIX
	size_t i, nameLen;
	char *pEq;
	char *pCurr;
	bma_assert(pEnv != NULL);
	bma_assert(pName != NULL);
	bma_assert(pOutValue != NULL);
	nameLen = strlen(pName);
	for (i=0; i<pEnv->num; ++i) {
		pCurr = pEnv->pEnvp[i];
		pEq = strchr(pCurr, '=');
		if (pEq == NULL) continue;
		if (((size_t)(pEq - pCurr) == nameLen) && (memcmp(pCurr, pName, nameLen) == 0)) {
			++pEq; /* skip the '=' to go to the value */
			bma_StrBldr_appndStr(pOutValue, pEq);
			return;
		}
	}
#endif
}

BMA_DEF void bma_Env_set(bma_Env *pEnv, const char *pName, const char *pValue) {
#if BMA_IS_WINDOWS
	wchar_t *pWideName, *p;
	wchar_t *pWideNewVal;
	size_t wideNameLen, newValLen;
	size_t addLen;
	wchar_t *pDst;
	bma_assert(pEnv != NULL);
	pWideName = bma_strWdup(pName);
	pWideNewVal = bma_strWdup(pValue);
	wideNameLen = wcslen(pWideName);
	newValLen = wcslen(pWideNewVal);
	for (p = pEnv->pEnvBlock; (p[0] != L'\0'); p = p + wcslen(p) + 1U) {
		const wchar_t *pEq;
		size_t len;
		if (p[0] == L'=') continue;
		pEq = wcschr(p, L'=');
		if (pEq == NULL) continue;
		len = (size_t)(pEq - p);
		/* Windows env var names are case insensitive */
		if ((len == wideNameLen) && (_wcsnicmp(p, pWideName, wideNameLen) == 0)) {
			size_t valOffs = (size_t)(pEq - pEnv->pEnvBlock) + 1U; /* skip "=" */
			size_t oldValLen = wcslen(pEnv->pEnvBlock + valOffs);
			size_t tailLen = pEnv->sz + 2U - (valOffs + oldValLen); /* from the var's terminator to the end */
			wchar_t *pVal;
			if (newValLen > oldValLen) {
				bma_Env_ensureCap(pEnv, pEnv->sz + 2U + (newValLen - oldValLen));
			}
			pVal = pEnv->pEnvBlock + valOffs;
			memmove(pVal + newValLen, pVal + oldValLen, tailLen * sizeof(wchar_t));
			memcpy(pVal, pWideNewVal, newValLen * sizeof(wchar_t));
			pEnv->sz = pEnv->sz - oldValLen + newValLen;
			goto out;
		}
	}
	/* not found: append "name=value\0" before the final terminator */
	size_t addLen = wideNameLen + 1U + newValLen + 1U;
	wchar_t *pDst;
	bma_Env_ensureCap(pEnv, pEnv->sz + addLen + 1U);
	pDst = pEnv->pEnvBlock + pEnv->sz;
	memcpy(pDst, pWideName, wideNameLen * sizeof(wchar_t));
	pDst[wideNameLen] = L'=';
	memcpy(pDst + wideNameLen + 1U, pWideNewVal, newValLen * sizeof(wchar_t));
	pDst[addLen - 1U] = L'\0';
	pDst[addLen] = L'\0';
	pEnv->sz += addLen;
out:
	bma_free(pWideNewVal);
	bma_free(pWideName);
#elif BMA_IS_POSIX
#endif
}

/* Joins the existing value (if any) and pValue with ';' and stores the result */
BMA_DEF void bma_EnvVars_jn(bma_Env *pEnv, const char *pName, const char *pValue, bma_bool_t prepend) {
	bma_StrBldr joined, old;
	bma_assert(pEnv != NULL);
	bma_StrBldr_ctor(&old);
	bma_Env_get(pEnv, pName, &old);
	if (bma_StrBldr_getSz(&old) == 0U) {
		bma_Env_set(pEnv, pName, pValue);
	}
	else {
		bma_StrBldr_ctor(&joined);
		bma_StrBldr_appndStr(&joined, prepend ? pValue : bma_StrBldr_getStr(&old));
		bma_StrBldr_appndStr(&joined, BMA_ENV_VAL_SEP);
		bma_StrBldr_appndStr(&joined, prepend ? bma_StrBldr_getStr(&old) : pValue);
		bma_Env_set(pEnv, pName, bma_StrBldr_getStr(&joined));
		bma_StrBldr_dtor(&joined, NULL);
	}
	bma_StrBldr_dtor(&old, NULL);
}

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
	wchar_t *pWide = NULL;
	bma_IMemAlloc *pAlloc = bma_StrBldr_getAlloc(pOut);
	bma_StrBldr raw;
	bma_assert(pCmd != NULL);
	bma_assert(pExitCode != NULL);
	bma_assert(pOut != NULL);
	bma_assert(pAlloc != NULL);
	bma_StrBldr_ctor_ext(&raw, pAlloc);
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
	pWcmdLn = bma_strWdup_ext(pCmd, pAlloc);
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
			int wideLen, utf8Len;
			/* Convert to native windows (UTF-16) and back to UTF-8*/
			wideLen = MultiByteToWideChar(cp, 0, bma_StrBldr_getStr(&raw), (int)bma_StrBldr_getSz(&raw), NULL, 0);
			if (wideLen <= 0) {
				goto out;
			}
			pWide = (*pAlloc->pRllc)(pAlloc, NULL, (size_t)wideLen * sizeof(wchar_t), NULL);
			(void)MultiByteToWideChar(cp, 0, bma_StrBldr_getStr(&raw), (int)bma_StrBldr_getSz(&raw), pWide, wideLen);
			utf8Len = WideCharToMultiByte(CP_UTF8, 0, pWide, wideLen, NULL, 0, NULL, NULL);
			if (utf8Len <= 0) {
				goto out;
			}
			bma_StrBldr_rsz(&raw, (size_t)utf8Len);
			(void)WideCharToMultiByte(CP_UTF8, 0, pWide, wideLen, (char*)bma_StrBldr_getStr(&raw), utf8Len, NULL, NULL);
		}
		bma_StrBldr_appndStrN(pOut, bma_StrBldr_getStr(&raw), bma_StrBldr_getSz(&raw));
	}
	result = BMA_TRUE;
out:
	bma_free_ext(pAlloc, pWide);
	bma_StrBldr_dtor(&raw, NULL);
	bma_free_ext(pAlloc, pWcmdLn);
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
	char *pBuf;
	bma_assert(pCmd != NULL);
	bma_assert(pExitCode != NULL);
	bma_assert(pOut != NULL);
	*pExitCode = 0;
	bma_ProcArgVec_ctor_ext(&argv, pAlloc);
	pBuf = (char*)(*pAlloc->pRllc)(pAlloc, NULL, BMA_PROC_PIPE_READ_CHUNKS, NULL);
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
		ssize_t n = read(outPipe[0], pBuf, BMA_PROC_PIPE_READ_CHUNKS);
		if (n > 0) {
			bma_StrBldr_appndStrN(pOut, pBuf, (size_t)n);
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
	bma_free_ext(pAlloc, pBuf);
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
#include <bma/fs.h>

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

void test_envGetPath(void) {
	bma_Env vars;
	bma_StrBldr value;
	bma_StrBldr_ctor(&value);
	bma_Env_ctor(&vars);
	bma_Env_rdFrmOs(&vars);
	bma_Env_get(&vars, "PATH", &value);
	BMA_EXPECT(bma_StrBldr_getSz(&value) > 0);
	bma_StrBldr_clear(&value);
	bma_Env_get(&vars, "unknown_env_var", &value);
	BMA_EXPECT(bma_StrBldr_getSz(&value) == 0);
	bma_Env_dtor(&vars, NULL);
	bma_StrBldr_dtor(&value, NULL);
}

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

void test_getVsIncDir(void) {
#if BMA_IS_WINDOWS
	bma_StrBldr cmd;
	bma_StrBldr out;
	int exitCode = 0;
	char *pProgFiles;
	char *pVsInsDir;
	bma_StrBldr_ctor(&cmd);
	bma_StrBldr_ctor(&out);
	pProgFiles = getenv("ProgramFiles(x86)"); /* TODO need UTF_8 getenv */
	BMA_EXPECT(pProgFiles != NULL);
	bma_StrBldr_appndChr(&cmd, '\"', 1U);
	bma_StrBldr_appndStr(&cmd, pProgFiles);
	bma_entrPth(&cmd, "Microsoft Visual Studio");
	bma_entrPth(&cmd, "Installer");
	bma_entrPth(&cmd, "vswhere.exe");
	bma_StrBldr_appndStr(&cmd, "\" -latest -property installationPath");
	BMA_EXPECT(bma_runCmd(bma_StrBldr_getStr(&cmd), &exitCode, &out));
	pVsInsDir = bma_StrBldr_rlse(&out); /* now "out" is empty again */
	bma_trmWhtSpc(pVsInsDir); /* get rid of the newline */
	bma_StrBldr_clear(&cmd);
	bma_StrBldr_appndStr(&cmd, "cmd.exe /C \"call ");
	bma_StrBldr_appndStr(&cmd, pVsInsDir);
	bma_free(pVsInsDir);
	bma_entrPth(&cmd, "Common7");
	bma_entrPth(&cmd, "Tools");
	bma_entrPth(&cmd, "VsDevCmd.bat -arch=x64 -host_arch=x64");
	bma_StrBldr_appndStr(&cmd, " >NUL && set INCLUDE\"");
	BMA_EXPECT(bma_runCmd(bma_StrBldr_getStr(&cmd), &exitCode, &out));
	BMA_EXPECT(bma_strtsWth(bma_StrBldr_getStr(&out), "INCLUDE="));
	bma_StrBldr_dtor(&out, NULL);
	bma_StrBldr_dtor(&cmd, NULL);
#endif
}

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_envGetPath);
	BMA_TEST(test_echo);
	BMA_TEST(test_echoNonAscii);
	BMA_TEST(test_quotedArgsAndExitCode);
	BMA_TEST(test_missingCommand);
	BMA_TEST(test_getVsIncDir);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_PROC_TEST */

#endif /* BMA_PROC_H_INCLUDED */
