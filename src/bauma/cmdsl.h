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
e.g. to call compilers and other build stuff. Can also be used to generate any
arbitrary string.

The syntax is:

- Variables: They can be set with the function "%set(VARIABLE_NAME)" or before evaluation
  of the template with bauma_cmdsl_setVariable().
  They are evaluated with "%get(VARIABLE_NAME)". In fact, %set and %get are just normal built-in functions
- Functions can be called with "%functionName(param1 param2 ...)". They evaluate their arguments as string
  and return a string result. Custom functions can be installed prior to parsing and evaluation.
- The % sign can also be used to escape some special characters:
  - "%%" is replaced with "%"
  - "%(" is replaced with "("
  - "%)" is replaced with ")"
  - "%," is replaced with ","
  - "%" followed by any white-space character skips all of the whitespace until a non-whitespace
        character is reached.
  - "%*" starts a comment, ignoring everything until "*%" is reached, similarly to C-style comments.
    Comment nesting is supported, so "%* ... %* ... *% ... *%" is a valid comment with another comment inside.
  - "%/" starts a comment until the end of the line, similarly to C++-style comments.

Simple example: "%get(CC) -c -o %get(OBJ) %get(SRC)" could be a template for a compiler command line, where CC, OBJ and SRC are variables
that are pre-set using bauma_cmdsl_setVariable().

When the "%" sign isn't the ideal choice, it can be replaced by calling the extended constructor with another one,
such as the dollar or at-sign.

More complex example:

%set(MYVAR,MYVALUE)% %set(MYVAR2,MYVALUE2)%/ Sets the variable to the value. THe % followed by whitespace skips that whitespace in the output
%mySpecialFunction(%get(MYVAR),%get(MYVAR2),%,)%/ Calls a custom function with the variable values as parameters
%/ The "%," is replaced by comma, so the 3rd parameter to mySpecialFunction is actually a comma.

The built-in functions are:

- %set(VAR_NAME,VALUE): Sets a variable to a value, returns an empty string
- %get(VAR_NAME): Returns the value of a variable, or an empty string if the variable isn't set yet
- %push(VAR_NAME): Pushes the current value of a variable to an internal stack, returns an empty string. Not-defined variables are pushed as empty string.
- %pop(VAR_NAME): Pops the last value of a variable from the internal stack and sets the variable to that value, returns an empty string.
- %call(FUNCTION,PARAM1,PARAM2,...): Calls a custom function with the given parameters, returns the result of that function.
  This enables calling function whose name is calclulated at run-time or by a variable. So, this is also possible:
  %call(%get(MYVAR),PARAM1,PARAM2,...) which calls whatever function name is stored inside MYVAR.
*/
#include <bauma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef BAUMA_CMDSL_DEF
	#define BAUMA_CMDSL_DEF
#endif

typedef struct bauma_cmdsl_StrWithLen {
	const char *p;
	size_t l;
	bauma_IMemAllocator *pAlloc;
} bauma_cmdsl_StrWithLen;

BAUMA_CMDSL_DEF void bauma_cmdsl_StrWithLen_destruct(bauma_cmdsl_StrWithLen *p);
BAUMA_CMDSL_DEF size_t bauma_cmdsl_StrWithLen_hash(const void *p);
BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_StrWithLen_equals(const void *pLhs, const void *pRhs);

typedef struct bauma_cmdsl_VarNameAndValue {
	bauma_cmdsl_StrWithLen name;
	bauma_StringBuilder value;
} bauma_cmdsl_VarNameAndValue;

BAUMA_CMDSL_DEF void bauma_cmdsl_VarNameAndValue_destruct(bauma_cmdsl_VarNameAndValue *p);

typedef struct bauma_Cmdsl bauma_Cmdsl;

typedef bauma_bool_t (*bauma_cmdsl_pFunction)(bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl, void *pUserData, const bauma_cmdsl_StrWithLen* pParams, size_t numOfParams);

typedef struct bauma_cmdsl_FunctionWithUserData {
	bauma_cmdsl_pFunction pFunc;
	void *pUserData; /* optional, heap-allocated */
	bauma_pDestructor pUserDataDtor;  /* optional */
} bauma_cmdsl_FunctionWithUserData;

BAUMA_CMDSL_DEF void bauma_cmdsl_FunctionWithUserData_destruct(bauma_cmdsl_FunctionWithUserData* pSelf);

typedef struct bauma_ICmdslNode bauma_ICmdslNode;

struct bauma_Cmdsl {
	bauma_HashMap functions;
	bauma_HashMap variables;
	bauma_Vector  pushedVariables;
	bauma_ICmdslNode *pRootNode;
	bauma_IMemAllocator *pAlloc;
	char escapeChar;
};

BAUMA_CMDSL_DEF void bauma_cmdsl_construct_ext(bauma_Cmdsl *pSelf, char escapeChar, bauma_IMemAllocator *pAlloc);
#define bauma_cmdsl_construct(pSelf) bauma_cmdsl_construct_ext((pSelf), '%', bauma_getDefaultMemAllocator())

BAUMA_CMDSL_DEF void bauma_cmdsl_destruct(bauma_Cmdsl *pSelf);

BAUMA_CMDSL_DEF void bauma_cmdsl_addFunction(bauma_Cmdsl *pSelf, const char *pFuncName, bauma_cmdsl_FunctionWithUserData* p);

BAUMA_CMDSL_DEF void bauma_cmdsl_setVariable(bauma_Cmdsl *pSelf, const char *pVarName, const char *pValue);
BAUMA_CMDSL_DEF const char *bauma_cmdsl_getVariable(bauma_Cmdsl *pSelf, const char *pVarName);

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_parseMem_ext(bauma_Cmdsl *pSelf, const void *pMem, size_t memSize, bauma_StringBuilder *pErrFormatter);
#define bauma_cmdsl_parseMem(pSelf, pMem, memSize) bauma_cmdsl_parseMem_ext((pSelf), (pMem), (memSize), NULL)

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_parseStr_ext(bauma_Cmdsl *pSelf, const char *pStr, bauma_StringBuilder *pErrFormatter);
#define bauma_cmdsl_parseStr(pSelf, pStr) bauma_cmdsl_parseStr_ext((pSelf), (pStr), NULL)


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

BAUMA_CMDSL_DEF void bauma_cmdsl_StrWithLen_destruct(bauma_cmdsl_StrWithLen *p) {
	(*p->pAlloc->pRealloc)(p->pAlloc, (void*)p->p, 0, NULL);
}

BAUMA_CMDSL_DEF size_t bauma_cmdsl_StrWithLen_hash(const void *p) {
	const bauma_cmdsl_StrWithLen *pStr = (const bauma_cmdsl_StrWithLen*)p;
	size_t result = 0;
	size_t i;
	bauma_cmdsl_assert(p != NULL);
	for (i=0; i<pStr->l; ++i) {
		result = (31u * result) + (size_t)((unsigned char)pStr->p[i]);
	}
	return result;
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_StrWithLen_equals(const void *pLhs, const void *pRhs) {
	const bauma_cmdsl_StrWithLen *pLhsStr = (const bauma_cmdsl_StrWithLen*)pLhs;
	const bauma_cmdsl_StrWithLen *pRhsStr = (const bauma_cmdsl_StrWithLen*)pRhs;
	bauma_cmdsl_assert(pLhs != NULL);
	bauma_cmdsl_assert(pRhs != NULL);
	if (pLhsStr->l != pRhsStr->l) {
		return BAUMA_FALSE;
	}
	return (memcmp(pLhsStr->p, pRhsStr->p, pLhsStr->l) == 0);
}

BAUMA_CMDSL_DEF void bauma_cmdsl_VarNameAndValue_destruct(bauma_cmdsl_VarNameAndValue *p) {
	bauma_cmdsl_StrWithLen_destruct(&p->name);
	bauma_StringBuilder_destruct(&p->value);
}

struct bauma_ICmdslNode {
	void (*pDestruct)(void* pSelf);
	bauma_bool_t (*pEval)(void* pSelf, bauma_StringBuilder* pDst, bauma_Cmdsl* pCmdsl);
};

typedef struct bauma_CmdslNodeAndAlloc {
	bauma_ICmdslNode *pNode;
	bauma_IMemAllocator *pAlloc;
} bauma_CmdslNodeAndAlloc;

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeAndAlloc_construct_ext(bauma_CmdslNodeAndAlloc *pSelf, bauma_ICmdslNode *pNode, bauma_IMemAllocator *pAlloc) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pNode != NULL);
	bauma_cmdsl_assert(pAlloc != NULL);
	pSelf->pNode = pNode;
	pSelf->pAlloc = pAlloc;
}

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeAndAlloc_destruct(bauma_CmdslNodeAndAlloc *p) {
	bauma_cmdsl_assert(p != NULL);
	bauma_cmdsl_assert(p->pNode != NULL);
	bauma_cmdsl_assert(p->pAlloc != NULL);
	(*p->pNode->pDestruct)(p->pNode);
	(*p->pAlloc->pRealloc)(p->pAlloc, (void*)p->pNode, 0, NULL);
}

typedef struct bauma_CmdslNodeText {
	bauma_ICmdslNode base;
	bauma_cmdsl_StrWithLen text;
} bauma_CmdslNodeText;

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeText_destruct(bauma_CmdslNodeText *pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_StrWithLen_destruct(&pSelf->text);
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_NodeText_eval(void *pSelf_, bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl) {
	bauma_CmdslNodeText *pSelf = (bauma_CmdslNodeText*)pSelf_;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pDst != NULL);
	bauma_cmdsl_assert(pCmdsl != NULL);
	bauma_StringBuilder_appendStrWithLen(pDst, pSelf->text.p, pSelf->text.l);
	return BAUMA_TRUE;
}

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeText_construct(bauma_CmdslNodeText *pSelf, const char *pText, size_t textLen, bauma_IMemAllocator *pAlloc) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pText != NULL);
	bauma_cmdsl_assert(pAlloc != NULL);
	pSelf->base.pDestruct = (bauma_pDestructor)&bauma_cmdsl_NodeText_destruct;
	pSelf->base.pEval = &bauma_cmdsl_NodeText_eval;
	pSelf->text.p = bauma_strdupn_ext(pText, textLen, pAlloc);
	pSelf->text.l = textLen;
	pSelf->text.pAlloc = pAlloc;
}

typedef struct bauma_CmdslNodeFunctionCall {
	bauma_ICmdslNode base;
	bauma_cmdsl_FunctionWithUserData *pFunc;
} bauma_CmdslNodeFunctionCall;

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeFunctionCall_destruct(bauma_CmdslNodeFunctionCall *pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
	/* pFunc is owned by the Cmdsl and will be destructed there, so do nothing here */
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_NodeFunctionCall_eval(bauma_CmdslNodeFunctionCall *pSelf, bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pDst != NULL);
	bauma_cmdsl_assert(pCmdsl != NULL);
	bauma_cmdsl_assert(pSelf->pFunc != NULL);
	return (*pSelf->pFunc->pFunc)(pDst, pCmdsl, pSelf->pFunc->pUserData, NULL, 0);
}

typedef struct bauma_CmdslNodeSequence {
	bauma_ICmdslNode base;
	bauma_Vector nodes; /* of bauma_CmdslNodeAndAlloc* */
} bauma_CmdslNodeSequence;

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeSequence_destruct(bauma_CmdslNodeSequence *pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_Vector_destruct(&pSelf->nodes);
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_NodeSequence_eval(void *pSelf_, bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl) {
	size_t i;
	bauma_CmdslNodeSequence *pSelf = (bauma_CmdslNodeSequence*)pSelf_;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pDst != NULL);
	bauma_cmdsl_assert(pCmdsl != NULL);
	for (i=0; i<bauma_Vector_getSize(&pSelf->nodes); ++i) {
		bauma_CmdslNodeAndAlloc *pNode = bauma_Vector_at(&pSelf->nodes, i, bauma_CmdslNodeAndAlloc);
		if (!(*pNode->pNode->pEval)(pNode->pNode, pDst, pCmdsl)) {
			return BAUMA_FALSE;
		}
	}
	return BAUMA_TRUE;
}

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeSequence_construct(bauma_CmdslNodeSequence *pSelf, bauma_IMemAllocator *pAlloc) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pAlloc != NULL);
	pSelf->base.pDestruct = (bauma_pDestructor)&bauma_cmdsl_NodeSequence_destruct;
	pSelf->base.pEval = &bauma_cmdsl_NodeSequence_eval;
	bauma_Vector_construct_ext(&pSelf->nodes, bauma_CmdslNodeAndAlloc, (bauma_pDestructor)&bauma_cmdsl_NodeAndAlloc_destruct, pAlloc);
}

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeSequence_append(bauma_CmdslNodeSequence *pSelf, bauma_ICmdslNode *pNode) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pNode != NULL);
	bauma_CmdslNodeAndAlloc nodeAndAlloc;
	bauma_cmdsl_NodeAndAlloc_construct_ext(&nodeAndAlloc, pNode, bauma_Vector_getAlloc(&pSelf->nodes));
	bauma_Vector_append(&pSelf->nodes, bauma_CmdslNodeAndAlloc, &nodeAndAlloc);
}

BAUMA_CMDSL_DEF void bauma_cmdsl_FunctionWithUserData_destruct(bauma_cmdsl_FunctionWithUserData* pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
	if (pSelf->pUserData != NULL) {
		if (pSelf->pUserDataDtor != NULL) {
			(*pSelf->pUserDataDtor)(pSelf->pUserData);
		}
		bauma_free(pSelf->pUserData);
	}
}

BAUMA_CMDSL_DEF void bauma_cmdsl_construct_ext(bauma_Cmdsl *pSelf, char escapeChar, bauma_IMemAllocator *pAlloc) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pAlloc != NULL);
	pSelf->escapeChar = escapeChar;
	bauma_HashMap_construct_ext(&pSelf->functions, bauma_cmdsl_StrWithLen, bauma_cmdsl_FunctionWithUserData,
	                            (bauma_pDestructor)&bauma_cmdsl_StrWithLen_destruct, (bauma_pDestructor)&bauma_cmdsl_FunctionWithUserData_destruct,
	                            &bauma_cmdsl_StrWithLen_hash, &bauma_cmdsl_StrWithLen_equals, pAlloc);
	bauma_HashMap_construct_ext(&pSelf->variables, bauma_cmdsl_StrWithLen, bauma_StringBuilder,
	                            (bauma_pDestructor)&bauma_cmdsl_StrWithLen_destruct, (bauma_pDestructor)&bauma_StringBuilder_destruct,
	                            &bauma_cmdsl_StrWithLen_hash, &bauma_cmdsl_StrWithLen_equals, pAlloc);
	bauma_Vector_construct_ext(&pSelf->pushedVariables, bauma_cmdsl_VarNameAndValue, (bauma_pDestructor)&bauma_cmdsl_VarNameAndValue_destruct, pAlloc);
	pSelf->pRootNode = NULL;
	pSelf->pAlloc = pAlloc;
}

BAUMA_CMDSL_DEF void bauma_cmdsl_destruct(bauma_Cmdsl *pSelf) {
	if (pSelf->pRootNode != NULL) {
		(*pSelf->pRootNode->pDestruct)(pSelf->pRootNode);
		(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pRootNode, 0, NULL);
	}
	bauma_Vector_destruct(&pSelf->pushedVariables);
	bauma_HashMap_destruct(&pSelf->variables);
	bauma_HashMap_destruct(&pSelf->functions);
}

BAUMA_CMDSL_DEF void bauma_cmdsl_addFunction(bauma_Cmdsl *pSelf, const char *pFuncName, bauma_cmdsl_FunctionWithUserData* p) {
	size_t funcNameLen;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pFuncName != NULL);
	bauma_cmdsl_assert(p != NULL);
	bauma_cmdsl_StrWithLen key;
	funcNameLen = strlen(pFuncName);
	key.p = bauma_strdupn(pFuncName, funcNameLen);
	key.l = funcNameLen;
	key.pAlloc = pSelf->pAlloc;
	bauma_HashMap_put(&pSelf->functions, &key, p, bauma_StrWithLen, bauma_cmdsl_FunctionWithUserData);
}

BAUMA_CMDSL_DEF void bauma_cmdsl_setVariable(bauma_Cmdsl *pSelf, const char *pVarName, const char *pValue) {
	size_t nameLen, valueLen;
	bauma_StringBuilder *pFoundValue;
	bauma_StringBuilder newContent;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pVarName != NULL);
	bauma_cmdsl_assert(pValue != NULL);
	nameLen = strlen(pVarName);
	valueLen = strlen(pValue);
	bauma_cmdsl_StrWithLen varName;
	/* For look-up, varName doesn't need heap allocation (will be faster)*/
	varName.p = pVarName;
	varName.l = nameLen;
	varName.pAlloc = NULL;
	pFoundValue = bauma_HashMap_get(&pSelf->variables, &varName, bauma_cmdsl_StrWithLen, bauma_StringBuilder);
	if (pFoundValue != NULL) {
		bauma_StringBuilder_clear(pFoundValue);
		bauma_StringBuilder_appendStrWithLen(pFoundValue, pValue, valueLen);
		return;
	}
	/* now, varName must be heap allocated to not loose the content when stored */
	varName.p = bauma_strdupn_ext(pVarName, nameLen, pSelf->pAlloc);
	varName.l = nameLen;
	varName.pAlloc = pSelf->pAlloc;
	bauma_StringBuilder_construct_ext(&newContent, pSelf->pAlloc);
	bauma_StringBuilder_appendStrWithLen(&newContent, pValue, valueLen);
	bauma_HashMap_put(&pSelf->variables, &varName, &newContent, bauma_cmdsl_StrWithLen, bauma_StringBuilder);
}

BAUMA_CMDSL_DEF const char *bauma_cmdsl_getVariable(bauma_Cmdsl *pSelf, const char *pVarName) {
	size_t nameLen;
	bauma_StringBuilder *pFoundValue;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pVarName != NULL);
	nameLen = strlen(pVarName);
	bauma_cmdsl_StrWithLen varName;
	varName.p = pVarName;
	varName.l = nameLen;
	varName.pAlloc = NULL;
	pFoundValue = bauma_HashMap_get(&pSelf->variables, &varName, bauma_cmdsl_StrWithLen, bauma_StringBuilder);
	if (pFoundValue != NULL) {
		return bauma_StringBuilder_getStr(pFoundValue);
	}
	return "";
}

BAUMA_CMDSL_DEF bauma_ICmdslNode* bauma_cmdsl_parseNode(bauma_Cmdsl *pSelf, const char *p, size_t len, bauma_StringBuilder *pErrFormatter) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(p != NULL);
	(void)p;
	(void)len;
	(void)pErrFormatter;
	return NULL;
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_parseMem_ext(bauma_Cmdsl *pSelf, const void *pMem, size_t memSize, bauma_StringBuilder *pErrFormatter) {
	/* TODO */
	(void)pSelf;
	(void)pMem;
	(void)memSize;
	(void)pErrFormatter;
	return BAUMA_FALSE;
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_parseStr_ext(bauma_Cmdsl *pSelf, const char *pStr, bauma_StringBuilder *pErrFormatter) {
	size_t len;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pStr != NULL);
	len = strlen(pStr);
	return bauma_cmdsl_parseMem_ext(pSelf, pStr, len, pErrFormatter);
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

void test_construct(void) {
	bauma_Cmdsl cmdsl;
	bauma_cmdsl_construct(&cmdsl);
	BAUMA_EXPECT(bauma_HashMap_getSize(&cmdsl.functions) == 0);
	BAUMA_EXPECT(bauma_HashMap_getSize(&cmdsl.variables) == 0);
	BAUMA_EXPECT(bauma_Vector_getSize(&cmdsl.pushedVariables) == 0);
	BAUMA_EXPECT(cmdsl.pRootNode == NULL);
	bauma_cmdsl_destruct(&cmdsl);
}

void test_setGetVariable(void) {
	bauma_Cmdsl cmdsl;
	bauma_cmdsl_construct(&cmdsl);
	bauma_cmdsl_setVariable(&cmdsl, "var1", "value1");
	BAUMA_EXPECT(strcmp(bauma_cmdsl_getVariable(&cmdsl, "var1"), "value1") == 0);
	bauma_cmdsl_setVariable(&cmdsl, "var1", "value2");
	BAUMA_EXPECT(strcmp(bauma_cmdsl_getVariable(&cmdsl, "var1"), "value2") == 0);
	bauma_cmdsl_setVariable(&cmdsl, "var2", "value3");
	BAUMA_EXPECT(strcmp(bauma_cmdsl_getVariable(&cmdsl, "var2"), "value3") == 0);
	BAUMA_EXPECT(strcmp(bauma_cmdsl_getVariable(&cmdsl, "Unknown"), "") == 0);
	BAUMA_EXPECT(bauma_HashMap_getSize(&cmdsl.variables) == 2u);
	bauma_cmdsl_destruct(&cmdsl);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BAUMA_TEST(test_construct);
	BAUMA_TEST(test_setGetVariable);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BAUMA_CMDSL_TEST */

#endif /* BAUMA_CMDSL_H_INCLUDED */
