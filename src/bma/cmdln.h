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

/* Vector type for parsed integer-vector parameters */
BMA_DEF_VEC(bma_CmdLnIntVec, int, NULL)
/* Vector type for parsed size_t-vector parameters */
BMA_DEF_VEC(bma_CmdLnSizeTVec, size_t, NULL)
/* Vector type for parsed string-vector parameters */
BMA_DEF_VEC(bma_CmdLnStrVec, char*, &bma_memblck_dtor)

/*! \brief Type of command line options */
typedef unsigned int bma_CmdLineOptType;

/*! \brief Optional "flag" type of argument without parameter such as "--verbose" or "-v" */
#define BMA_CMDLN_OPT_TYPE_FLAG       ((bma_CmdLineOptType)0)
/*! \brief Boolean parameter with true/false argument following the parameter name, like "--boolParam false" */
#define BMA_CMDLN_OPT_TYPE_BOOL       ((bma_CmdLineOptType)1)
/*! \brief Integer parameter with an integer argument following the parameter name, like "--intParam 42" */
#define BMA_CMDLN_OPT_TYPE_INT        ((bma_CmdLineOptType)2)
/*! \brief Vector of integers (bma_CmdLnIntVec). If "reqred" flag is set,
           minimum one occurence is requird. Example "--intVec 1 --intVec 2"
	   If the target field isn't initialized (all bytes zero),
	   it will be constructed using bma_CmdLnIntVec_ctor */
#define BMA_CMDLN_OPT_TYPE_INT_VEC    ((bma_CmdLineOptType)3)
/*! \brief Same as BMA_CMDLN_OPT_TYPE_INT but targetting size_t values */
#define BMA_CMDLN_OPT_TYPE_SIZE_T     ((bma_CmdLineOptType)4)
/*! \brief Same as BMA_CMDLN_OPT_TYPE_INT_VEC but targetting size_t values */
#define BMA_CMDLN_OPT_TYPE_SIZE_T_VEC ((bma_CmdLineOptType)5)
/*! \brief String parameter. The target string pointer will be freed and afterwards willed with a heap-allocated copy of the argument */
#define BMA_CMDLN_OPT_TYPE_STR        ((bma_CmdLineOptType)6)
/*! \brief Vector of string parameters */
#define BMA_CMDLN_OPT_TYPE_STR_VEC    ((bma_CmdLineOptType)7)

/*!
  \brief Definition of a single command line argument
*/
typedef struct bma_CmdLnArgDef {
	bma_CmdLineOptType type; /*!< \brief Type of the argument, see BMA_CMDLN_OPT_TYPE_* */
	const char *pShort;      /*!< \brief Short option name (invoked with a single dash)
	                                     or NULL when no short variant is used */
	const char *pLong;       /*!< \brief Long option name (invoked with a double dash) */
	const char *pDesc;       /*!< \brief Human readable description of the argument */
	bma_bool_t reqred;       /*!< \brief Whether the argument is required */
	size_t memOffs;          /*!< \brief Offset in the output structure where to put the parsed value */
} bma_CmdLnArgDef;

/*!
  \brief Definition of a single command line command. A command can be stand-alone
         or have any number of sub commands. This way, a command line can be structured hierarchically.
*/
typedef struct bma_CmdLnCmdDef {
	const char *pName;                      /*!< \brief Name of the command, must be [A-Za-z_-][A-Za-z0-9_-]* */
	const char *pDesc;                      /*!< \brief Human readable description of the command */
	size_t enumOffs;                        /*!< \brief Offset in the output structure of type int or unsigned int
	                                                    where a specific value is to be set when the command is detected */
	unsigned int enumVal;                   /*!< \brief What value to be put at that offset when that commmand is detected */
	const struct bma_CmdLnCmdDef *pSubCmds; /*!< \brief Pointer to an array of sub-command definitions, NULL if no sub-commands */
	size_t numSubCmds;                      /*!< \brief Number of sub-command definitions in the array */
	const struct bma_CmdLnArgDef *pArgs;    /*!< \brief Pointer to an array of argument definitions, NULL if no arguments */
	size_t numArgs;
} bma_CmdLnCmdDef;

BMA_DEF bma_bool_t bma_CmdLn_parse(int argc, char **argv, const bma_CmdLnCmdDef *pDef, void *pOutStruct);
BMA_DEF void bma_CmdLn_parseOrExit(int argc, char **argv, const bma_CmdLnCmdDef *pDef, void *pOutStruct);

/*!
  \brief Returns pStr if pStr is not NULL, otherwise returns an empty string.
  To be used to access parsed command line strings where structure has been initialized
  with NULL or memset to retrieve a valid null terminated string in any case.
*/
BMA_DEF const char *bma_CmdLn_getStr(const char *pStr);

BMA_DEF bma_bool_t bma_CmdLn_isFieldNull(const void *pField, size_t fieldSz);

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

BMA_DEF bma_bool_t bma_CmdLn_vecHasStr(bma_CmdLnStrVec *pVec, const char *pStr) {
	size_t i;
	for (i=0; i<bma_CmdLnStrVec_getSz(pVec); ++i) {
		const char *pExisting = *bma_CmdLnStrVec_at(pVec, i);
		if (strcmp(pExisting, pStr) == 0) {
			return BMA_TRUE;
		}
	}
	return BMA_FALSE;
}

BMA_DEF void bma_CmdLn_addStrToVec(bma_CmdLnStrVec *pVec, const char *pStr) {
	char *p;
	if (bma_CmdLn_vecHasStr(pVec, pStr)) return;
	p = bma_strdup(pStr);
	bma_CmdLnStrVec_appnd(pVec, &p);
}

BMA_DEF bma_bool_t bma_CmdLn_parse(int argc, char **argv, const bma_CmdLnCmdDef *pDef, void *pOutStruct) {
	int i = 1;
	int firstArg;
	size_t j;
	bma_bool_t result = BMA_FALSE;
	const bma_CmdLnCmdDef *pCurrDef;
	bma_CmdLnStrVec detectedArgs;
	bma_assert(argc > 0);
	bma_assert(argv != NULL);
	bma_assert(pDef != NULL);
	bma_assert(pOutStruct != NULL);
	bma_CmdLnStrVec_ctor(&detectedArgs);

	pCurrDef = pDef;
	/* Find the correct sub-command, if any. */
	while(pCurrDef->numSubCmds > 0) {
		if (i >= argc) {
			fprintf(stderr, "Expected sub-command after '%s'\n", argv[i - 1]);
			goto out;
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
			goto out;
		}
	}
	/* Scan arguments */
	firstArg = i;
	for (i=firstArg; i<argc; i++) {
		const char *pArgKey = argv[i];
		bma_bool_t isLong = bma_strtsWth(pArgKey, "--");
		bma_bool_t isShort = bma_strtsWth(pArgKey, "-") && !isLong;
		const char *pArgValue = NULL;
		if (!isLong && !isShort) {
			fprintf(stderr, "Expected option starting with '-' or '--', got '%s'\n", pArgKey);
			goto out;
		}
		if (isLong) {
			pArgKey += 2; /* Skip the "--" prefix */
		}
		else {
			pArgKey += 1; /* Skip the "-" prefix */
		}
		const bma_CmdLnArgDef *pFoundArgDef = NULL;
		for (j=0; j<pCurrDef->numArgs; ++j) {
			const bma_CmdLnArgDef *pArgDef = &pCurrDef->pArgs[j];
			if ((isLong && (strcmp(pArgKey, pArgDef->pLong) == 0)) ||
			    (isShort && (pArgDef->pShort != NULL) && (strcmp(pArgKey, pArgDef->pShort) == 0))) {
				pFoundArgDef = pArgDef;
				break;
			}
		}
		if (pFoundArgDef == NULL) {
			fprintf(stderr, "Unknown option '%s'\n", argv[i]);
			goto out;
		}
		bma_CmdLn_addStrToVec(&detectedArgs, pFoundArgDef->pLong);
		if (pFoundArgDef->type != BMA_CMDLN_OPT_TYPE_FLAG) { /* Flags have no value, each other type has a value*/
			if (i + 1 >= argc) {
				fprintf(stderr, "Expected value for argument '%s'\n", pFoundArgDef->pDesc);
				goto out;
			}
			pArgValue = argv[i + 1];
		}
		switch(pFoundArgDef->type) {
			case BMA_CMDLN_OPT_TYPE_FLAG: {
				bma_bool_t *pField = bma_lndr_cast(bma_bool_t, ((char*)pOutStruct) + pFoundArgDef->memOffs);
				*pField = BMA_TRUE;
				break;
			}
			case BMA_CMDLN_OPT_TYPE_BOOL: {
				bma_bool_t isTrue = BMA_FALSE;
				bma_bool_t isFalse = BMA_FALSE;
				isTrue = bma_CmdLn_strCaseEq(pArgValue, "true");
				if (!isTrue) isFalse = bma_CmdLn_strCaseEq(pArgValue, "false");
				if ((!isTrue) && (!isFalse)) {
					fprintf(stderr, "Expected boolean value for argument '%s', got '%s'\n", pFoundArgDef->pDesc, pArgValue);
					goto out;
				}
				bma_bool_t *pField = bma_lndr_cast(bma_bool_t, ((char*)pOutStruct) + pFoundArgDef->memOffs);
				*pField = isTrue;
				break;
			}
			case BMA_CMDLN_OPT_TYPE_INT: {
				char *endPtr;
				errno = 0;
				long l = strtol(pArgValue, &endPtr, 10);
				if ((*endPtr != '\0') || (errno == ERANGE) || (l < INT_MIN) || (l > INT_MAX)) {
					fprintf(stderr, "Expected integer value for argument '%s', got '%s'\n", pFoundArgDef->pDesc, pArgValue);
					goto out;
				}
				int *pField = bma_lndr_cast(int, ((char*)pOutStruct) + pFoundArgDef->memOffs);
				*pField = (int)l;
				break;
			}
			case BMA_CMDLN_OPT_TYPE_INT_VEC: {
				char *endPtr;
				errno = 0;
				long l = strtol(pArgValue, &endPtr, 10);
				int k;
				if ((*endPtr != '\0') || (errno == ERANGE) || (l < INT_MIN) || (l > INT_MAX)) {
					fprintf(stderr, "Expected integer value for argument '%s', got '%s'\n", pFoundArgDef->pDesc, pArgValue);
					goto out;
				}
				k = (int)l;
				bma_CmdLnIntVec *pField = bma_lndr_cast(bma_CmdLnIntVec, ((char*)pOutStruct) + pFoundArgDef->memOffs);
				if (bma_CmdLn_isFieldNull(pField, sizeof(*pField))) {
					bma_CmdLnIntVec_ctor(pField);
				}
				bma_CmdLnIntVec_appnd(pField, &k);
				break;
			}
			case BMA_CMDLN_OPT_TYPE_SIZE_T: {
				char *endPtr;
				errno = 0;
				#ifdef ULLONG_MAX
					unsigned long long l = strtoull(pArgValue, &endPtr, 10);
				#else
					unsigned long l = strtoul(pArgValue, &endPtr, 10);
				#endif
				if ((*endPtr != '\0') || (errno == ERANGE) || (l > BMA_SIZE_MAX)) {
					fprintf(stderr, "Expected size_t value for argument '%s', got '%s'\n", pFoundArgDef->pDesc, pArgValue);
					goto out;
				}
				size_t *pField = bma_lndr_cast(size_t, ((char*)pOutStruct) + pFoundArgDef->memOffs);
				*pField = (size_t)l;
				break;
			}
			case BMA_CMDLN_OPT_TYPE_SIZE_T_VEC: {
				char *endPtr;
				errno = 0;
				#ifdef ULLONG_MAX
					unsigned long long l = strtoull(pArgValue, &endPtr, 10);
				#else
					unsigned long l = strtoul(pArgValue, &endPtr, 10);
				#endif
				size_t k;
				if ((*endPtr != '\0') || (errno == ERANGE) || (l > BMA_SIZE_MAX)) {
					fprintf(stderr, "Expected size_t value for argument '%s', got '%s'\n", pFoundArgDef->pDesc, pArgValue);
					goto out;
				}
				k = (size_t)l;
				bma_CmdLnSizeTVec *pField = bma_lndr_cast(bma_CmdLnSizeTVec, ((char*)pOutStruct) + pFoundArgDef->memOffs);
				if (bma_CmdLn_isFieldNull(pField, sizeof(*pField))) {
					bma_CmdLnSizeTVec_ctor(pField);
				}
				bma_CmdLnSizeTVec_appnd(pField, &k);
				break;
			}
			case BMA_CMDLN_OPT_TYPE_STR: {
				char **pField = bma_lndr_cast(char*, ((char*)pOutStruct) + pFoundArgDef->memOffs);
				bma_free(*pField);
				*pField = bma_strdup(pArgValue);
				break;
			}
			case BMA_CMDLN_OPT_TYPE_STR_VEC: {
				bma_CmdLnStrVec *pField = bma_lndr_cast(bma_CmdLnStrVec, ((char*)pOutStruct) + pFoundArgDef->memOffs);
				char *pStr;
				if (bma_CmdLn_isFieldNull(pField, sizeof(*pField))) {
					bma_CmdLnStrVec_ctor(pField);
				}
				pStr = bma_strdup(pArgValue);
				bma_CmdLnStrVec_appnd(pField, &pStr);
				break;
			}
			default: {
				fprintf(stderr, "'%s': only types INT, SIZE_T and STR are supported for positional arguments, no vectors\n", pFoundArgDef->pDesc);
				goto out;
			}
		}
		if (pArgValue != NULL) {
			++i; /* Skip the value argument */
		}
	}
	for (j=0; j<pCurrDef->numArgs; ++j) {
		const bma_CmdLnArgDef *pArgDef = &pCurrDef->pArgs[j];
		if (pArgDef->reqred && !bma_CmdLn_vecHasStr(&detectedArgs, pArgDef->pLong)) {
			fprintf(stderr, "Mandatory argument '%s' not found.\n", pArgDef->pDesc);
			goto out;
		}
	}
	result = BMA_TRUE;
out:
	bma_CmdLnStrVec_dtor(&detectedArgs, NULL);
	return result;
}

BMA_DEF void bma_CmdLn_parseOrExit(int argc, char **argv, const bma_CmdLnCmdDef *pDef, void *pOutStruct) {
	if (!bma_CmdLn_parse(argc, argv, pDef, pOutStruct)) {
		exit(1);
	}
}

BMA_DEF const char *bma_CmdLn_getStr(const char *pStr) {
	if (pStr == NULL) {
		return "";
	}
	return pStr;
}

BMA_DEF bma_bool_t bma_CmdLn_isFieldNull(const void *pField, size_t fieldSz) {
	const unsigned char *p = (const unsigned char *)pField;
	size_t i;
	for (i=0; i<fieldSz; ++i) {
		if (p[i] != 0) {
			return BMA_FALSE;
		}
	}
	return BMA_TRUE;
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

typedef struct SimpleArgs {
	bma_bool_t f; /* Flag parameter */
	bma_bool_t b;
	int i;
	size_t s;
	char *pStr;
} SimpleArgs;

void SimpleArgs_dtor(SimpleArgs *p, bma_IMemAlloc *pAlloc) {
	(void)pAlloc;
	bma_free(p->pStr);
}

static const bma_CmdLnArgDef kSimpleArgs[] = {
	{
		 /* type    */ BMA_CMDLN_OPT_TYPE_FLAG
		,/* pShort  */ "f"
		,/* pLong   */ "flag"
		,/* pDesc   */ "Flag parameter"
		,/* reqred  */ BMA_TRUE
		,/* memOffs */ offsetof(SimpleArgs, f)
	}
	,{
		 /* type    */ BMA_CMDLN_OPT_TYPE_BOOL
		,/* pShort  */ "b"
		,/* pLong   */ "bool"
		,/* pDesc   */ "Boolean parameter"
		,/* reqred  */ BMA_TRUE
		,/* memOffs */ offsetof(SimpleArgs, b)
	}
	,{
		 /* type    */ BMA_CMDLN_OPT_TYPE_INT
		,/* pShort  */ "i"
		,/* pLong   */ "int"
		,/* pDesc   */ "Integer parameter"
		,/* reqred  */ BMA_TRUE
		,/* memOffs */ offsetof(SimpleArgs, i)
	}
	,{
		 /* type    */ BMA_CMDLN_OPT_TYPE_SIZE_T
		,/* pShort  */ "z"
		,/* pLong   */ "size"
		,/* pDesc   */ "Size_t parameter"
		,/* reqred  */ BMA_TRUE
		,/* memOffs */ offsetof(SimpleArgs, s)
	}
	,{
		 /* type    */ BMA_CMDLN_OPT_TYPE_STR
		,/* pShort  */ "s"
		,/* pLong   */ "str"
		,/* pDesc   */ "String parameter"
		,/* reqred  */ BMA_TRUE
		,/* memOffs */ offsetof(SimpleArgs, pStr)
	}
};

static const bma_CmdLnCmdDef kSimpleArgsDef = {
	 /* pName      */ ""
	,/* pDesc      */ "The command"
	,/* enumOffs   */ 0 /* no tagged enum in output struct */
	,/* enumVal    */ 0 /* not used */
	,/* pSubCmds   */ NULL /* no sub-commands*/
	,/* numSubCmds */ 0
	,/* pArgs   */ kSimpleArgs
	,/* numArgs */ bma_ary_sz(kSimpleArgs)
};

void test_simpleArgsShort(void) {
	bma_bool_t couldParse;
	SimpleArgs target;
	char *argv[] = {
		 "prog"
		,"-f"
		,"-b"
		,"TrUe"
		,"-i"
		,"42"
		,"-z"
		,"43"
		,"-s"
		,"Hello"
		,NULL
	};
	memset(&target, 0, sizeof(target));
	couldParse = bma_CmdLn_parse((int)(bma_ary_sz(argv) - 1u), argv, &kSimpleArgsDef, &target);
	BMA_EXPECT(couldParse);
	BMA_EXPECT(target.b);
	BMA_EXPECT(target.i == 42);
	BMA_EXPECT(target.s == 43);
	BMA_EXPECT(strcmp(bma_CmdLn_getStr(target.pStr), "Hello") == 0);
	SimpleArgs_dtor(&target, NULL);
}

void test_simpleArgsLong(void) {
	bma_bool_t couldParse;
	SimpleArgs target;
	char *argv[] = {
		 "prog"
		,"--bool"
		,"TrUe"
		,"--str"
		,"Hello"
		,"--flag"
		,"--size"
		,"43"
		,"--int"
		,"42"
		,NULL
	};
	memset(&target, 0, sizeof(target));
	couldParse = bma_CmdLn_parse((int)(bma_ary_sz(argv) - 1u), argv, &kSimpleArgsDef, &target);
	BMA_EXPECT(couldParse);
	BMA_EXPECT(target.b);
	BMA_EXPECT(target.i == 42);
	BMA_EXPECT(target.s == 43);
	BMA_EXPECT(strcmp(bma_CmdLn_getStr(target.pStr), "Hello") == 0);
	SimpleArgs_dtor(&target, NULL);
}

typedef struct VecArgs {
	bma_CmdLnIntVec   iv; /* Vector of integers */
	bma_CmdLnSizeTVec zv; /* Vector of size_t   */
	bma_CmdLnStrVec   sv; /* Vector of strings  */
} VecArgs;

void VecArgs_dtor(VecArgs *p, bma_IMemAlloc *pAlloc) {
	(void)pAlloc;
	bma_CmdLnStrVec_dtor(&p->sv, NULL);
	bma_CmdLnSizeTVec_dtor(&p->zv, NULL);
	bma_CmdLnIntVec_dtor(&p->iv, NULL);
}

static const bma_CmdLnArgDef kVecArgs[] = {
	{
		 /* type    */ BMA_CMDLN_OPT_TYPE_INT_VEC
		,/* pShort  */ "i"
		,/* pLong   */ "int"
		,/* pDesc   */ "Integer vector parameter"
		,/* reqred  */ BMA_TRUE
		,/* memOffs */ offsetof(VecArgs, iv)
	}
	,{
		 /* type    */ BMA_CMDLN_OPT_TYPE_SIZE_T_VEC
		,/* pShort  */ "z"
		,/* pLong   */ "size"
		,/* pDesc   */ "Size_t vector parameter"
		,/* reqred  */ BMA_TRUE
		,/* memOffs */ offsetof(VecArgs, zv)
	}
	,{
		 /* type    */ BMA_CMDLN_OPT_TYPE_STR_VEC
		,/* pShort  */ "s"
		,/* pLong   */ "str"
		,/* pDesc   */ "String vector parameter"
		,/* reqred  */ BMA_TRUE
		,/* memOffs */ offsetof(VecArgs, sv)
	}
};

static const bma_CmdLnCmdDef kVecArgsDef = {
	 /* pName      */ ""
	,/* pDesc      */ "The command"
	,/* enumOffs   */ 0 /* no tagged enum in output struct */
	,/* enumVal    */ 0 /* not used */
	,/* pSubCmds   */ NULL /* no sub-commands*/
	,/* numSubCmds */ 0
	,/* pArgs   */ kVecArgs
	,/* numArgs */ bma_ary_sz(kVecArgs)
};

void test_vecArgsShort(void) {
	bma_bool_t couldParse;
	VecArgs target;
	char *argv[] = {
		 "prog" 
		,"-i"
		,"1"
		,"-i"
		,"2"
		,"-i"
		,"3"
		,"-z"
		,"4"
		,"-z"
		,"5"
		,"-z"
		,"6"
		,"-s"
		,"Hello"
		,"-s"
		,"World"
		,NULL
	};
	memset(&target, 0, sizeof(target));
	couldParse = bma_CmdLn_parse((int)(bma_ary_sz(argv) - 1u), argv, &kVecArgsDef, &target);
	BMA_EXPECT(couldParse);
	BMA_EXPECT(bma_CmdLnIntVec_getSz(&target.iv) == 3u);
	BMA_EXPECT(*bma_CmdLnIntVec_at(&target.iv, 0) == 1);
	BMA_EXPECT(*bma_CmdLnIntVec_at(&target.iv, 1) == 2);
	BMA_EXPECT(*bma_CmdLnIntVec_at(&target.iv, 2) == 3);
	BMA_EXPECT(bma_CmdLnSizeTVec_getSz(&target.zv) == 3u);
	BMA_EXPECT(*bma_CmdLnSizeTVec_at(&target.zv, 0) == 4);
	BMA_EXPECT(*bma_CmdLnSizeTVec_at(&target.zv, 1) == 5);
	BMA_EXPECT(*bma_CmdLnSizeTVec_at(&target.zv, 2) == 6);
	BMA_EXPECT(bma_CmdLnStrVec_getSz(&target.sv) == 2u);
	BMA_EXPECT(strcmp(*bma_CmdLnStrVec_at(&target.sv, 0), "Hello") == 0);
	BMA_EXPECT(strcmp(*bma_CmdLnStrVec_at(&target.sv, 1), "World") == 0);
	VecArgs_dtor(&target, NULL);
}

void test_vecArgsLong(void) {
	bma_bool_t couldParse;
	VecArgs target;
	char *argv[] = {
		 "prog" 
		,"--int"
		,"1"
		,"--size"
		,"4"
		,"--str"
		,"Hello"
		,"--int"
		,"2"
		,"--size"
		,"5"
		,"--int"
		,"3"
		,"--size"
		,"6"
		,"--str"
		,"World"
		,NULL
	};
	memset(&target, 0, sizeof(target));
	couldParse = bma_CmdLn_parse((int)(bma_ary_sz(argv) - 1u), argv, &kVecArgsDef, &target);
	BMA_EXPECT(couldParse);
	BMA_EXPECT(bma_CmdLnIntVec_getSz(&target.iv) == 3u);
	BMA_EXPECT(*bma_CmdLnIntVec_at(&target.iv, 0) == 1);
	BMA_EXPECT(*bma_CmdLnIntVec_at(&target.iv, 1) == 2);
	BMA_EXPECT(*bma_CmdLnIntVec_at(&target.iv, 2) == 3);
	BMA_EXPECT(bma_CmdLnSizeTVec_getSz(&target.zv) == 3u);
	BMA_EXPECT(*bma_CmdLnSizeTVec_at(&target.zv, 0) == 4);
	BMA_EXPECT(*bma_CmdLnSizeTVec_at(&target.zv, 1) == 5);
	BMA_EXPECT(*bma_CmdLnSizeTVec_at(&target.zv, 2) == 6);
	BMA_EXPECT(bma_CmdLnStrVec_getSz(&target.sv) == 2u);
	BMA_EXPECT(strcmp(*bma_CmdLnStrVec_at(&target.sv, 0), "Hello") == 0);
	BMA_EXPECT(strcmp(*bma_CmdLnStrVec_at(&target.sv, 1), "World") == 0);
	VecArgs_dtor(&target, NULL);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_simpleArgsShort);
	BMA_TEST(test_simpleArgsLong);
	BMA_TEST(test_vecArgsShort);
	BMA_TEST(test_vecArgsLong);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_CMDSL_TEST */

#endif /* BMA_CMDSL_H_INCLUDED */
