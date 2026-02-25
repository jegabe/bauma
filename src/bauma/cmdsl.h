#ifndef BAUMA_CMDSL_H_INCLUDED
#define BAUMA_CMDSL_H_INCLUDED

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

A minimalistic string template engine to generate command line arguments,
e.g. to call compilers and other build stuff.

*/

#include <bauma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef BAUMA_CMDSL_DEF
	#define BAUMA_CMDSL_DEF
#endif

typedef struct bauma_Cmdsl bauma_Cmdsl;

typedef bauma_bool_t (*bauma_cmdsl_pFunction)(bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl, void *pUserData, const bauma_StrWithLen* pParams, size_t numOfParams);

typedef struct bauma_cmdsl_FunctionWithUserData {
	bauma_cmdsl_pFunction pFunc;
	void *pUserData; /* optional, heap-allocated */
	bauma_pDestructor pUserDataDtor;  /* optional */
} bauma_cmdsl_FunctionWithUserData;

BAUMA_CMDSL_DEF bauma_cmdsl_FunctionWithUserData_destruct(bauma_cmdsl_FunctionWithUserData* pSelf);

typedef struct bauma_ICmdslNode bauma_ICmdslNode;

struct bauma_ICmdslNode {
	void (*pDestruct)(bauma_ICmdslNode* pSelf);
	bauma_bool_t (*pEval)(bauma_ICmdslNode* pSelf, bauma_StringBuilder* pDst, bauma_Cmdsl* pCmdsl);
};

struct bauma_Cmdsl {
	bauma_HashMap functions;
	bauma_HashMap variables;
	bauma_Vector  pushedVariables;
	bauma_ICmdslNode *pRootNode;
	bauma_IMemAllocator *pAlloc;
};

BAUMA_CMDSL_DEF void bauma_cmdsl_construct_ext(bauma_Cmdsl *pSelf, bauma_IMemAllocator *pAlloc);
#define bauma_cmdsl_construct(pSelf) bauma_cmdsl_construct_ext((pSelf), bauma_getDefaultMemAllocator())

BAUMA_CMDSL_DEF void bauma_cmdsl_destruct(bauma_Cmdsl *pSelf);

BAUMA_CMDSL_DEF void bauma_cmdsl_addFunction(bauma_Cmdsl *pSelf, const char *pFuncName, bauma_cmdsl_FunctionWithUserData* p);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BAUMA_CMDSL_IMPLEMENTATION

#include <string.h>

#ifdef bauma_cmdsl_custom_assert
	#define bauma_cmdsl_assert(x) bauma_cmdsl_custom_assert(x)
#else
	#include <assert.h>
	#define bauma_cmdsl_assert(x) assert(x)
#endif


#ifdef __cplusplus
	extern "C" {
#endif

typedef struct baume_CmdslNodeText {
	bauma_ICmdslNode base;
	bauma_StrWithLen text;
} bauma_CmdslNodeText;

typedef struct baume_CmdslNodeFunctionCall {
	bauma_ICmdslNode base;
	bauma_cmdsl_FunctionWithUserData *pFunc;
} baume_CmdslNodeFunctionCall;


BAUMA_CMDSL_DEF bauma_cmdsl_FunctionWithUserData_destruct(bauma_cmdsl_FunctionWithUserData* pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
	if (pSelf->pUserData != NULL) {
		if (pSelf->pUserDataDtor != NULL) {
			(*pSelf->pUserDataDtor)(pSelf->pUserData);
		}
		bauma_free(pSelf->pUserData);
	}
}

BAUMA_CMDSL_DEF void bauma_cmdsl_construct_ext(bauma_Cmdsl *pSelf, bauma_IMemAllocator *pAlloc) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pAlloc != NULL);
	bauma_HashMap_construct_ext(&pSelf->functions, bauma_StrWithLen, bauma_cmdsl_FunctionWithUserData,
	                            (bauma_pDestructor)&bauma_StrWithLen_destruct,  (bauma_pDestructor)&bauma_cmdsl_FunctionWithUserData_destruct,
	                            &bauma_StrWithLen_hash, &bauma_StrWithLen_equals, pAlloc);
	bauma_HashMap_construct_ext(&pSelf->variables, bauma_StrWithLen, bauma_StringBuilder,
	                            (bauma_pDestructor)&bauma_StrWithLen_destruct, (bauma_pDestructor)&bauma_StringBuilder_destruct,
	                            &bauma_StrWithLen_hash, &bauma_StrWithLen_equals, pAlloc);
	pSelf->pRootNode = NULL;
	pSelf->pAlloc = pAlloc;
}

BAUMA_CMDSL_DEF void bauma_cmdsl_destruct(bauma_Cmdsl *pSelf) {
	if (pSelf->pRootNode != NULL) {
		(*pSelf->pRootNode->pDestruct)(pSelf->pRootNode);
		(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pRootNode, 0, NULL);
	}
	bauma_HashMap_destruct(&pSelf->variables);
	bauma_HashMap_destruct(&pSelf->functions);
}

BAUMA_CMDSL_DEF void bauma_cmdsl_addFunction(bauma_Cmdsl *pSelf, const char *pFuncName, bauma_cmdsl_FunctionWithUserData* p) {
	size_t funcNameLen;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pFuncName != NULL);
	bauma_cmdsl_assert(p != NULL);
	bauma_StrWithLen key;
	funcNameLen = strlen(pFuncName);
	key.p = bauma_strdupn(pFuncName, funcNameLen);
	key.l = funcNameLen;
	bauma_HashMap_put(&pSelf->functions, &key, p, bauma_StrWithLen, bauma_cmdsl_FunctionWithUserData);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BAUMA_CMDSL_IMPLEMENTATION */

#ifdef BAUMA_CMDSL_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
	extern "C" {
#endif

void bauma_test_exit_fail(const char *exp, const char *file, int line) {
	fprintf(stderr, "Expectation '%s' failed (file %s line %d)\n", exp, file, line);
	fflush(stderr);
	exit(1);
}

#define BAUMA_EXPECT(x) \
	do { \
		if(!(x)) { \
			bauma_test_exit_fail(#x, __FILE__, __LINE__); \
		} \
	} while(0)

#define BAUMA_TEST(f) \
	do { \
		printf("Executing '" #f "()'...\n"); \
		f(); \
	} while(0)

void test_dummy(void) {
	BAUMA_EXPECT(BAUMA_TRUE);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BAUMA_TEST(test_dummy);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BAUMA_CMDSL_TEST */

#endif /* BAUMA_CMDSL_H_INCLUDED */
