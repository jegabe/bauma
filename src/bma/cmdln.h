#ifndef BMA_CMDLN_H_INCLUDED
#define BMA_CMDLN_H_INCLUDED

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

Bau 'ma's command line parser.

A command line is defined by a set of sub-commands or sub-sub commands (optional).
For each ((sub-)sub) command, there can be a set of positional arguments plus
a set of options. Options can either be short and prefixed with a single dash
or long and prefixed with a double dash.

A definition is done by a const in-memory chain of descriptors. The parser will read
that descriptor and attempt to parse the command line according to that. The descriptor
knows the offsets and types which should be stored and uses that information to fill
an output structure. In case of an error, it will print a help message.

The pre-defined option "--help" or "-h" will be supported by default and can't be changed
by the user



*/
#include <bma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef unsigned int bma_CmdLineOptType;

#define BMA_CMDLN_OPT_TYPE_BOOL       ((bma_CmdLineOptType)0)
#define BMA_CMDLN_OPT_TYPE_INT        ((bma_CmdLineOptType)1)
#define BMA_CMDLN_OPT_TYPE_INT_VEC    ((bma_CmdLineOptType)2)
#define BMA_CMDLN_OPT_TYPE_SIZE_T     ((bma_CmdLineOptType)3)
#define BMA_CMDLN_OPT_TYPE_SIZE_T_VEC ((bma_CmdLineOptType)4)
#define BMA_CMDLN_OPT_TYPE_STR        ((bma_CmdLineOptType)5)
#define BMA_CMDLN_OPT_TYPE_STR_VEC    ((bma_CmdLineOptType)6)

typedef struct bma_CmdLnPosArgDef {
	bma_CmdLineOptType type;
	const char *pDesc;
	bma_bool_t reqred;
	size_t memOffs;
} bma_CmdLnPosArgDef;

typedef struct bma_CmdLnOptArgDef {
	bma_CmdLineOptType type;
	const char *pShort;
	const char *pLong;
	const char *pDesc;
	bma_bool_t reqred;
	size_t memOffs;
} bma_CmdLnOptArgDef;

typedef struct bma_CmdLnCmdDef {
	const char *pName;
	const char *pDesc;
	size_t enumOffs; /* points to an int- or unsigned int sized field, or 0 if not used */
	unsigned int enumVal; /* value to be set in the output struct at that offset*/
	const struct bma_CmdLnCmdDef *pSubCmds;
	size_t numSubCmds;
	const struct bma_CmdLnPosArgDef *pPosArgs;
	size_t numPosArgs;
	const struct bma_CmdLnOptArgDef *pOptArgs;
	size_t numOptArgs;
} bma_CmdLnCmdDef;

BMA_DEF bma_bool_t bma_CmdLn_parse(int argc, char **argv, const bma_CmdLnCmdDef *pDef, void *pOutStruct);
BMA_DEF void bma_CmdLn_parseOrExit(int argc, char **argv, const bma_CmdLnCmdDef *pDef, void *pOutStruct);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_CMDLN_IMPL

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#ifdef __cplusplus
	extern "C" {
#endif

BMA_DEF bma_bool_t bma_CmdLn_strCaseEq(const char *pStr1, const char *pStr2) {
	size_t len1 = strlen(pStr1);
	size_t len2 = strlen(pStr2);
	size_t i;
	if (len1 != len2) return BMA_FALSE;
	for (i=0; i<len1; ++i) {
		if (tolower((unsigned char)pStr1[i]) != tolower((unsigned char)pStr2[i])) {
			return BMA_FALSE;
		}
	}
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_CmdLn_parse(int argc, char **argv, const bma_CmdLnCmdDef *pDef, void *pOutStruct) {
	int i = 1;
	int firstArg;
	size_t j;
	const bma_CmdLnCmdDef *pCurrDef;
	bma_assert(argc > 0);
	bma_assert(argv != NULL);
	bma_assert(pDef != NULL);
	bma_assert(pOutStruct != NULL);
	pCurrDef = pDef;
	/* Find the correct sub-command, if any. */
	while(pCurrDef->numSubCmds > 0) {
		if (i >= argc) {
			fprintf(stderr, "Expected sub-command after '%s'\n", argv[i - 1]);
			return BMA_FALSE;
		}
		const char *pArg = argv[i++];
		size_t j;
		bma_bool_t foundSubCmd = BMA_FALSE;
		for (j=0; j<pCurrDef->numSubCmds; ++j) {
			const bma_CmdLnCmdDef *pSubCmdDef = &pCurrDef->pSubCmds[j];
			if (strcmp(pArg, pSubCmdDef->pName) == 0) {
				pCurrDef = pSubCmdDef;
				if (pCurrDef->enumOffs > 0) {
					unsigned int *pEnumField = bma_lndr_cast(unsigned int, ((char*)pOutStruct) + pCurrDef->enumOffs);
					*pEnumField = pCurrDef->enumVal;
				}
				foundSubCmd = BMA_TRUE;
				break;
			}
		}
		if (!foundSubCmd) {
			fprintf(stderr, "Unknown sub-command '%s'\n", pArg);
			return BMA_FALSE;
		}
	}
	/* Scan positional arguments */
	firstArg = i;
	for (j=0; j<pCurrDef->numPosArgs; ++j) {
		const bma_CmdLnPosArgDef *pPosArgDef = &pCurrDef->pPosArgs[j];
		for (i=firstArg; i<argc; ++i) {
			const char *pArg = argv[i];
			bma_bool_t found = BMA_FALSE;
			if (!bma_strtsWth(pArg, "--") && !bma_strtsWth(pArg, "-")) {
				switch(pPosArgDef->type) {
					case BMA_CMDLN_OPT_TYPE_BOOL: {
						if (!bma_CmdLn_strCaseEq(pArg, "true") && !bma_CmdLn_strCaseEq(pArg, "false")) {
							fprintf(stderr, "Expected boolean value for argument '%s', got '%s'\n", pPosArgDef->pDesc, pArg);
							return BMA_FALSE;
						}
						bma_bool_t b = bma_CmdLn_strCaseEq(pArg, "true");
						bma_bool_t *pField = bma_lndr_cast(bma_bool_t, ((char*)pOutStruct) + pPosArgDef->memOffs);
						*pField = b;
						break;
					}
					case BMA_CMDLN_OPT_TYPE_INT: {
						char *endPtr;
						errno = 0;
						long l = strtol(pArg, &endPtr, 10);
						if ((*endPtr != '\0') || (errno == ERANGE) || (l < INT_MIN) || (l > INT_MAX)) {
							fprintf(stderr, "Expected integer value for argument '%s', got '%s'\n", pPosArgDef->pDesc, pArg);
							return BMA_FALSE;
						}
						int *pField = bma_lndr_cast(int, ((char*)pOutStruct) + pPosArgDef->memOffs);
						*pField = (int)l;
						break;
					}
					case BMA_CMDLN_OPT_TYPE_SIZE_T: {
						char *endPtr;
						errno = 0;
						#ifdef ULLONG_MAX
							unsigned long long l = strtoull(pArg, &endPtr, 10);
						#else
							unsigned long l = strtoul(pArg, &endPtr, 10);
						#endif
						if ((*endPtr != '\0') || (errno == ERANGE) || (l > ((size_t)-1))) {
							fprintf(stderr, "Expected size_t value for argument '%s', got '%s'\n", pPosArgDef->pDesc, pArg);
							return BMA_FALSE;
						}
						size_t *pField = bma_lndr_cast(size_t, ((char*)pOutStruct) + pPosArgDef->memOffs);
						*pField = (size_t)l;
						break;
					}
					case BMA_CMDLN_OPT_TYPE_STR: {
						char **pField = bma_lndr_cast(char*, ((char*)pOutStruct) + pPosArgDef->memOffs);
						if (*pField == NULL) {
							fprintf(stderr, "Target field for argument '%s', must not be NULL", pPosArgDef->pDesc);
						}
						bma_free(*pField);
						*pField = bma_strdup(pArg);
						break;
					}
					default: {
						fprintf(stderr, "'%s': only types INT, SIZE_T and STR are supported for positional arguments, no vectors\n", pPosArgDef->pDesc);
						return BMA_FALSE;
					}
				}
				found = BMA_TRUE;
				break;
			}
			if (!found && pPosArgDef->reqred) {
				fprintf(stderr, "Expected positional argument '%s'\n", pPosArgDef->pDesc);
				return BMA_FALSE;
			}
		}
	}

		if (bma_strtsWth(bma_StrBldr *pSelf, const char *pStr))

		if (firstArg + i >= argc) {
			fprintf(stderr, "Expected positional argument '%s'\n", pCurrDef->pPosArgs[i].pDesc);
			return BMA_FALSE;
		}

	return BMA_TRUE;
}

BMA_DEF void bma_CmdLn_parseOrExit(int argc, char **argv, const bma_CmdLnCmdDef *pDef, void *pOutStruct) {
	if (!bma_CmdLn_parse(argc, argv, pDef, pOutStruct)) {
		exit(1);
	}
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_CMDLN_IMPL */

#ifdef BMA_CMDLN_TEST

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

/* Simple use case: No sub-(sub-) commands, only positional arguments */

typedef struct OnlyPosArgs {
	char *pInputFile;
	char *pOutputFile;
} OnlyPosArgs;

static const bma_CmdLnPosArgDef kPosOnlyCmdLnPosArgs[] = {
	{
	 /* type    */ BMA_CMDLN_OPT_TYPE_STR
	,/* pDesc   */ "Input file path"
	,/* reqred  */ BMA_TRUE
	,/* memOffs */ offsetof(OnlyPosArgs, pInputFile)
	}
	,{
	 /* type    */ BMA_CMDLN_OPT_TYPE_STR
	,/* pDesc   */ "Output file path"
	,/* reqred  */ BMA_FALSE
	,/* memOffs */ offsetof(OnlyPosArgs, pOutputFile)
	}
};

static const bma_CmdLnCmdDef kPosOnlyCmdLnDef = {
	 /* pName */   ""
	,/* pDesc */   "The command"
	,/* enumOffs   */ 0 /* no tagged enum in output struct */
	,/* enumVal    */ 0 /* not used */
	,/* pSubCmds   */ NULL /* no sub-commands*/
	,/* numSubCmds */ 0
	,/* pPosArgs   */ kPosOnlyCmdLnPosArgs
	,/* numPosArgs */ bma_ary_sz(kPosOnlyCmdLnPosArgs)
	,/* pOptArgs   */ NULL
	,/* numOptArgs */ 0
};

void test_onlyPosArgs(void) {
	bma_bool_t couldParse;
	OnlyPosArgs target;
	char *argv[] = {"prog", "input.txt", "output.txt", NULL};
	target.pInputFile = bma_strdup("");
	target.pOutputFile = bma_strdup("");
	couldParse = bma_CmdLn_parse((int)(bma_ary_sz(argv) - 1u), argv, &kPosOnlyCmdLnDef, &target);
	BMA_EXPECT(couldParse);
	BMA_EXPECT(strcmp(target.pInputFile, "input.txt") == 0);
	BMA_EXPECT(strcmp(target.pOutputFile, "output.txt") == 0);
	bma_free(target.pOutputFile);
	bma_free(target.pInputFile);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_onlyPosArgs);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_CMDSL_TEST */

#endif /* BMA_CMDSL_H_INCLUDED */
