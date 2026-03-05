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
#include <bma/ccal.h>

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

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_addFunction(bauma_Cmdsl *pSelf, const char *pFuncName, bauma_cmdsl_FunctionWithUserData* p);

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_setVariable(bauma_Cmdsl *pSelf, const char *pVarName, const char *pValue);
BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_setVariable_ext(bauma_Cmdsl *pSelf, const char *pVarName, size_t nameLen, const char *pValue, size_t valueLen);


BAUMA_CMDSL_DEF const char *bauma_cmdsl_getVariable(bauma_Cmdsl *pSelf, const char *pVarName);
BAUMA_CMDSL_DEF const bauma_StringBuilder *bauma_cmdsl_getVariable_ext(bauma_Cmdsl *pSelf, const char *pVarName, size_t nameLen);

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_parseMem_ext(bauma_Cmdsl *pSelf, const void *pMem, size_t memSize, bauma_StringBuilder *pErrFormatter);
#define bauma_cmdsl_parseMem(pSelf, pMem, memSize) bauma_cmdsl_parseMem_ext((pSelf), (pMem), (memSize), NULL)

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_parseStr_ext(bauma_Cmdsl *pSelf, const char *pStr, bauma_StringBuilder *pErrFormatter);
#define bauma_cmdsl_parseStr(pSelf, pStr) bauma_cmdsl_parseStr_ext((pSelf), (pStr), NULL)


#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BAUMA_CMDSL_IMPLEMENTATION

#include <string.h>
#include <ctype.h>

#ifdef bauma_cmdsl_custom_assert
	#define bauma_cmdsl_assert(x) bauma_cmdsl_custom_assert(x)
#else
	#include <assert.h>
	#define bauma_cmdsl_assert(x) assert(x)
#endif


#ifdef __cplusplus
	extern "C" {
#endif

#define BAUMA_CMDSL_MAX_NUM_PARAMS 8u

typedef struct bauma_cmdsl_ParseSource {
	const char *pBegin; /* for error reporting to calculate error offset */
	const char *pStr; /* incremented while parsing */
	size_t len; /* decremented while parsing */
} bauma_cmdsl_ParseSource;

/*
typedef bauma_bool_t (*bauma_cmdsl_pFunction)(bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl, void *pUserData, const bauma_cmdsl_StrWithLen* pParams, size_t numOfParams);
*/

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_funcSet(bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl, void *pUserData, const bauma_cmdsl_StrWithLen* pParams, size_t numOfParams) {
	(void)pUserData;
	if (numOfParams != 2) {
		bauma_StringBuilder_clear(pDst);
		bauma_StringBuilder_appendStr(pDst, "Wrong number of parameters for function %set(), expected 2 but got ");
		bauma_StringBuilder_appendUnsigned(pDst, numOfParams);
		return BAUMA_FALSE;
	}
	bauma_cmdsl_setVariable_ext(pCmdsl, pParams[0].p, pParams[0].l, pParams[1].p, pParams[1].l);
	return BAUMA_TRUE;
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_funcGet(bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl, void *pUserData, const bauma_cmdsl_StrWithLen* pParams, size_t numOfParams) {
	(void)pUserData;
	const bauma_StringBuilder *pValue;
	if (numOfParams != 1) {
		bauma_StringBuilder_clear(pDst);
		bauma_StringBuilder_appendStr(pDst, "Wrong number of parameters for function %get(), expected 1 but got ");
		bauma_StringBuilder_appendUnsigned(pDst, numOfParams);
		return BAUMA_FALSE;
	}
	pValue = bauma_cmdsl_getVariable_ext(pCmdsl, pParams[0].p, pParams[0].l);
	if (pValue != NULL) {
		bauma_StringBuilder_appendStrWithLen(pDst, bauma_StringBuilder_getStr(pValue), bauma_StringBuilder_getSize(pValue));
	}
	return BAUMA_TRUE;
}

BAUMA_CMDSL_DEF void bauma_cmdsl_StrWithLen_destruct(bauma_cmdsl_StrWithLen *p) {
	(*p->pAlloc->pRealloc)(p->pAlloc, (void*)p->p, 0, NULL);
#if BAUMA_DEBUG
	memset(p, 0xFF, sizeof(*p));
#endif
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
#if BAUMA_DEBUG
	memset(p, 0xFF, sizeof(*p));
#endif
}

struct bauma_ICmdslNode {
	void (*pDestruct)(void* pSelf);
	bauma_bool_t (*pEval)(void* pSelf, bauma_StringBuilder* pDst, bauma_Cmdsl* pCmdsl);
};

typedef struct bauma_CmdslNodeAndAlloc {
	bauma_ICmdslNode *pNode;
	bauma_IMemAllocator *pAlloc;
} bauma_CmdslNodeAndAlloc;

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeAndAlloc_destruct(bauma_CmdslNodeAndAlloc *p) {
	bauma_cmdsl_assert(p != NULL);
	bauma_cmdsl_assert(p->pNode != NULL);
	bauma_cmdsl_assert(p->pAlloc != NULL);
	(*p->pNode->pDestruct)(p->pNode);
	(*p->pAlloc->pRealloc)(p->pAlloc, (void*)p->pNode, 0, NULL);
#if BAUMA_DEBUG
	memset(p, 0xFF, sizeof(*p));
#endif
}

typedef struct bauma_CmdslNodeText {
	bauma_ICmdslNode base;
	bauma_cmdsl_StrWithLen text;
} bauma_CmdslNodeText;

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeText_destruct(bauma_CmdslNodeText *pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_StrWithLen_destruct(&pSelf->text);
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_NodeText_eval(void *pSelf_, bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl) {
	bauma_CmdslNodeText *pSelf = (bauma_CmdslNodeText*)pSelf_;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pDst != NULL);
	bauma_cmdsl_assert(pCmdsl != NULL);
	bauma_StringBuilder_appendStrWithLen(pDst, pSelf->text.p, pSelf->text.l);
	return BAUMA_TRUE;
}

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeText_construct(bauma_CmdslNodeText *pSelf, const char *pText, size_t textLen, bauma_bool_t xferOwnership, bauma_IMemAllocator *pAlloc) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pText != NULL);
	bauma_cmdsl_assert(pAlloc != NULL);
	pSelf->base.pDestruct = (bauma_pDestructor)&bauma_cmdsl_NodeText_destruct;
	pSelf->base.pEval = &bauma_cmdsl_NodeText_eval;
	if (xferOwnership) {
		pSelf->text.p = pText;
	}
	else {
		pSelf->text.p = bauma_strdupn_ext(pText, textLen, pAlloc);
	}
	pSelf->text.l = textLen;
	pSelf->text.pAlloc = pAlloc;
}

typedef struct bauma_CmdslNodeEmpty {
	bauma_ICmdslNode base;
} bauma_CmdslNodeEmpty;

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeEmpty_destruct(bauma_CmdslNodeEmpty *pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_NodeEmpty_eval(void *pSelf, bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pDst != NULL);
	bauma_cmdsl_assert(pCmdsl != NULL);
	(void)pSelf;
	(void)pDst;
	(void)pCmdsl;
	return BAUMA_TRUE;
}

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeEmpty_construct(bauma_CmdslNodeEmpty *pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
	pSelf->base.pDestruct = (bauma_pDestructor)&bauma_cmdsl_NodeEmpty_destruct;
	pSelf->base.pEval = &bauma_cmdsl_NodeEmpty_eval;
}


typedef struct bauma_CmdslNodeFunctionCall {
	bauma_ICmdslNode base;
	bauma_cmdsl_FunctionWithUserData *pFunc;
	bauma_Vector args;
} bauma_CmdslNodeFunctionCall;

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeFunctionCall_destruct(bauma_CmdslNodeFunctionCall *pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_Vector_destruct(&pSelf->args);
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_NodeFunctionCall_eval(void *pSelf_, bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl) {
	bauma_CmdslNodeFunctionCall *pSelf = (bauma_CmdslNodeFunctionCall*)pSelf_;
	bauma_FunctionWithUserData *pFunc;
	bauma_cmdsl_StrWithLen params[BAUMA_CMDSL_MAX_NUM_PARAMS];
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pDst != NULL);
	bauma_cmdsl_assert(pCmdsl != NULL);
	bauma_cmdsl_assert(pSelf->pFunc != NULL);
	pFunc = pSelf->pFunc;
	(*pFunc->pFunc)(pDst, pCmdsl, pFunc->pUserData, TODO hier weitermachen);	


/*
typedef bauma_bool_t (*bauma_cmdsl_pFunction)(bauma_StringBuilder *pDst, bauma_Cmdsl* pCmdsl, void *pUserData, const bauma_cmdsl_StrWithLen* pParams, size_t numOfParams);
*/
	return BAUMA_FALSE;
}

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeFunctionCall_construct(bauma_CmdslNodeFunctionCall *pSelf, bauma_cmdsl_FunctionWithUserData *pFunc, bauma_IMemAllocator *pAlloc) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pFunc != NULL);
	bauma_cmdsl_assert(pAlloc != NULL);
	pSelf->base.pDestruct = (bauma_pDestructor)&bauma_cmdsl_NodeFunctionCall_destruct;
	pSelf->base.pEval = &bauma_cmdsl_NodeFunctionCall_eval;
	pSelf->pFunc = pFunc;
	bauma_Vector_construct_ext(&pSelf->args, bauma_CmdslNodeAndAlloc, (bauma_pDestructor)&bauma_cmdsl_NodeAndAlloc_destruct, pAlloc);
}

typedef struct bauma_CmdslNodeSequence {
	bauma_ICmdslNode base;
	bauma_Vector nodes; /* of bauma_CmdslNodeAndAlloc* */
} bauma_CmdslNodeSequence;

BAUMA_CMDSL_DEF void bauma_cmdsl_NodeSequence_destruct(bauma_CmdslNodeSequence *pSelf) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_Vector_destruct(&pSelf->nodes);
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
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
	nodeAndAlloc.pNode = pNode;
	nodeAndAlloc.pAlloc = bauma_Vector_getAlloc(&pSelf->nodes);
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
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BAUMA_CMDSL_DEF void bauma_cmdsl_construct_ext(bauma_Cmdsl *pSelf, char escapeChar, bauma_IMemAllocator *pAlloc) {
	bauma_cmdsl_FunctionWithUserData fu;
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
	/* add built-in functions */
	fu.pFunc = &bauma_cmdsl_funcSet;
	fu.pUserData = NULL;
	fu.pUserDataDtor = NULL;
	bauma_cmdsl_addFunction(pSelf, "set", &fu);
	fu.pFunc = &bauma_cmdsl_funcGet;
	fu.pUserData = NULL;
	fu.pUserDataDtor = NULL;
	bauma_cmdsl_addFunction(pSelf, "get", &fu);
}

BAUMA_CMDSL_DEF void bauma_cmdsl_destruct(bauma_Cmdsl *pSelf) {
	if (pSelf->pRootNode != NULL) {
		(*pSelf->pRootNode->pDestruct)(pSelf->pRootNode);
		(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pRootNode, 0, NULL);
	}
	bauma_Vector_destruct(&pSelf->pushedVariables);
	bauma_HashMap_destruct(&pSelf->variables);
	bauma_HashMap_destruct(&pSelf->functions);
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_isValidIdentifier(const char* pStr, size_t len) {
	size_t i;
	char c;
	if (len == 0) return BAUMA_FALSE;
	c = *pStr++;
	if (!(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || (c == '_'))) return BAUMA_FALSE;
	for (i=1; i<len; ++i) {
		c = *pStr++;
		if (!(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '_'))) return BAUMA_FALSE;
	}
	return BAUMA_TRUE;
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_addFunction(bauma_Cmdsl *pSelf, const char *pFuncName, bauma_cmdsl_FunctionWithUserData* p) {
	size_t funcNameLen;
	bauma_cmdsl_FunctionWithUserData *pExistingFunc;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pFuncName != NULL);
	bauma_cmdsl_assert(p != NULL);
	bauma_cmdsl_assert(p->pFunc != NULL);
	bauma_cmdsl_StrWithLen key;
	funcNameLen = strlen(pFuncName);
	if (!bauma_cmdsl_isValidIdentifier(pFuncName, funcNameLen)) {
		return BAUMA_FALSE;
	}
	/* for look-up, no dyn. allocation is needed */
	key.p = pFuncName;
	key.l = funcNameLen;
	key.pAlloc = NULL;
	pExistingFunc = bauma_HashMap_get(&pSelf->functions, &key, bauma_cmdsl_StrWithLen, bauma_cmdsl_FunctionWithUserData);
	if (pExistingFunc != NULL) {
		/* function already exists */
		return BAUMA_FALSE;
	}
	/* to put into map, dyn. allocate so string isn't lost */
	key.p = bauma_strdupn_ext(pFuncName, funcNameLen, pSelf->pAlloc);
	key.pAlloc = pSelf->pAlloc;
	bauma_HashMap_put(&pSelf->functions, &key, p, bauma_cmdsl_StrWithLen, bauma_cmdsl_FunctionWithUserData);
	return BAUMA_TRUE;
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_setVariable_ext(bauma_Cmdsl *pSelf, const char *pVarName, size_t nameLen, const char *pValue, size_t valueLen) {
	bauma_StringBuilder *pFoundValue;
	bauma_StringBuilder newContent;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pVarName != NULL);
	bauma_cmdsl_assert(pValue != NULL);
	if (!bauma_cmdsl_isValidIdentifier(pVarName, nameLen)) {
		return BAUMA_FALSE;
	}
	bauma_cmdsl_StrWithLen varName;
	/* For look-up, varName doesn't need heap allocation (will be faster)*/
	varName.p = pVarName;
	varName.l = nameLen;
	varName.pAlloc = NULL;
	pFoundValue = bauma_HashMap_get(&pSelf->variables, &varName, bauma_cmdsl_StrWithLen, bauma_StringBuilder);
	if (pFoundValue != NULL) {
		bauma_StringBuilder_clear(pFoundValue);
		bauma_StringBuilder_appendStrWithLen(pFoundValue, pValue, valueLen);
		return BAUMA_TRUE;
	}
	/* now, varName must be heap allocated to not loose the content when stored */
	varName.p = bauma_strdupn_ext(pVarName, nameLen, pSelf->pAlloc);
	varName.l = nameLen;
	varName.pAlloc = pSelf->pAlloc;
	bauma_StringBuilder_construct_ext(&newContent, pSelf->pAlloc);
	bauma_StringBuilder_appendStrWithLen(&newContent, pValue, valueLen);
	bauma_HashMap_put(&pSelf->variables, &varName, &newContent, bauma_cmdsl_StrWithLen, bauma_StringBuilder);
	return BAUMA_TRUE;
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_setVariable(bauma_Cmdsl *pSelf, const char *pVarName, const char *pValue) {
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pVarName != NULL);
	bauma_cmdsl_assert(pValue != NULL);
	return bauma_cmdsl_setVariable_ext(pSelf, pVarName, strlen(pVarName), pValue, strlen(pValue));
}

BAUMA_CMDSL_DEF const char *bauma_cmdsl_getVariable(bauma_Cmdsl *pSelf, const char *pVarName) {
	const bauma_StringBuilder *pValue;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pVarName != NULL);
	pValue = bauma_cmdsl_getVariable_ext(pSelf, pVarName, strlen(pVarName));
	if (pValue != NULL) {
		return bauma_StringBuilder_getStr(pValue);
	}
	return "";
}

BAUMA_CMDSL_DEF const bauma_StringBuilder *bauma_cmdsl_getVariable_ext(bauma_Cmdsl *pSelf, const char *pVarName, size_t nameLen) {
	bauma_cmdsl_StrWithLen varName;
	varName.p = pVarName;
	varName.l = nameLen;
	varName.pAlloc = NULL;
	return bauma_HashMap_get(&pSelf->variables, &varName, bauma_cmdsl_StrWithLen, bauma_StringBuilder);
}


BAUMA_CMDSL_DEF bauma_bool_t bauam_cmdsl_isNonFunctionCallEscape(bauma_Cmdsl *pSelf, bauma_cmdsl_ParseSource* pStr) {
	char c;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pStr != NULL);
	if (pStr->len < 2) return BAUMA_FALSE;
	if (pStr->pStr[0] != pSelf->escapeChar) return BAUMA_FALSE;
	c = pStr->pStr[1];
	if ((c == pSelf->escapeChar) || (strchr("(),*/", c) != NULL) || isspace(c)) {
		/* string escape or comment but no function call */
		return BAUMA_TRUE;
	}
	return BAUMA_FALSE;
}

BAUMA_CMDSL_DEF bauma_ICmdslNode *bauma_cmdsl_parseElement(bauma_Cmdsl *pSelf, bauma_cmdsl_ParseSource* pStr, bauma_StringBuilder *pErrFormatter, bauma_bool_t insideFunction);

static char CMDSL_ERR_AT_OFFSET[] = "Err at offset ";

BAUMA_CMDSL_DEF void bauma_cmdsl_printErrAtOffs(bauma_StringBuilder *pErrFormatter, bauma_cmdsl_ParseSource* pStr, const char *pMsg) {
	bauma_StringBuilder_appendStr(pErrFormatter, CMDSL_ERR_AT_OFFSET);
	bauma_StringBuilder_appendUnsigned(pErrFormatter, (size_t)(pStr->pStr - pStr->pBegin));
	bauma_StringBuilder_appendStr(pErrFormatter, ": ");
	bauma_StringBuilder_appendStr(pErrFormatter, pMsg);
}

BAUMA_CMDSL_DEF bauma_ICmdslNode *bauma_cmdsl_parseText(bauma_Cmdsl *pSelf, bauma_cmdsl_ParseSource* pStr, bauma_StringBuilder *pErrFormatter, bauma_bool_t insideFunction) {
	bauma_StringBuilder b;
	bauma_CmdslNodeText *pText;
	char *p;
	size_t l;
	bauma_StringBuilder_construct_ext(&b, pSelf->pAlloc);
	while(pStr->len > 0) {
		char c = *pStr->pStr;
		if (insideFunction && ((c == ',') || (c == ')'))) {
			break;
		}
		if (c != pSelf->escapeChar) {
			bauma_StringBuilder_appendChar(&b, (unsigned char)*pStr->pStr, 1u);
		}
		else
		{
			if (pStr->len < 2u) {
				if (pErrFormatter != NULL) {
					bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Found escape sequence but string is at end");
				}
				goto err;
			}
			c = pStr->pStr[1];
			pStr->pStr += 2u;
			pStr->len -= 2u;
			if ((c == pSelf->escapeChar) || (strchr("(),", c) != NULL)) {
				/* %% or %( or %) or %, */
				bauma_StringBuilder_appendChar(&b, c, 1u);
			}
			else if (isspace(c)) {
				/* %[space] */
				/* run forward to next non-space char */
				while((pStr->len > 0) && (isspace(*pStr->pStr))) {	
					++pStr->pStr;
					--pStr->len;
				}
			}
			else if (c == '*') {
				size_t nestCnt = 1u;
				/* %* comment */
				while(pStr->len >= 2) {
					if ((pStr->pStr[0] == pSelf->escapeChar) && (pStr->pStr[1] == '*')) {
						/* nested comment detected */
						++nestCnt;
						pStr->pStr += 2u;
						pStr->len -= 2u;
					}
					else if ((pStr->pStr[0] == '*') && (pStr->pStr[1] == pSelf->escapeChar)) {
						if (nestCnt == 0) {
							if (pErrFormatter != NULL) {
								bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Found superfluous comment end sequence");
							}
							goto err;
						}
						pStr->pStr += 2u;
						pStr->len -= 2u;
						--nestCnt;
						if (nestCnt == 0) break;
					}
					else {
						++pStr->pStr;
						--pStr->len;
					}
				}
			}
			else if (c == '/') {
				while(pStr->len > 0) {
					char c = *pStr->pStr;
					if ((c == '\r') || (c == '\n')) {
						/* newline detected */
						++pStr->pStr;
						--pStr->len;
						if ((c == '\r') && (pStr->len > 0) && (pStr->pStr[0] == '\n')) {
							/* CRLF madness */
							++pStr->pStr;
							--pStr->len;
						}
						break;
					}
					++pStr->pStr;
					--pStr->len;
				}
			}
			continue;
		}
		++pStr->pStr;
		--pStr->len;
	}
	l = bauma_StringBuilder_getSize(&b);
	p = bauma_StringBuilder_release(&b);
	pText = (bauma_CmdslNodeText*)(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, NULL, sizeof(bauma_CmdslNodeText), NULL);
	bauma_cmdsl_NodeText_construct(pText, p, l, BAUMA_TRUE, pSelf->pAlloc);
	return (bauma_ICmdslNode*)pText;
err:
	bauma_StringBuilder_destruct(&b);
	return NULL;
}

BAUMA_CMDSL_DEF bauma_ICmdslNode *bauma_cmdsl_parseFunctionCall(bauma_Cmdsl *pSelf, bauma_cmdsl_ParseSource* pStr, bauma_StringBuilder *pErrFormatter) {
	const char *p;
	size_t n;
	bauma_cmdsl_StrWithLen funcName;
	bauma_cmdsl_FunctionWithUserData* pFunc;
	bauma_CmdslNodeFunctionCall* pFuncCall;
	if ((pStr->len == 0) || (*pStr->pStr != pSelf->escapeChar)) {
		if (pErrFormatter != NULL) {
			bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "reached end while parsing function call");
		}
		return NULL;
	}
	++pStr->pStr;
	--pStr->len;
	p = memchr(pStr->pStr, '(', pStr->len);
	if (p == NULL) {
		if (pErrFormatter != NULL) {
			bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "'(' not found");
		}
		return NULL;
	}
	n = (size_t)(p - pStr->pStr); /* length of function name */
	if (n == 0) {
		if (pErrFormatter != NULL) {
			bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Function with zero name not allowed");
		}
		return NULL;
	}
	funcName.p = pStr->pStr;
	funcName.l = n;
	funcName.pAlloc = NULL;
	if (!bauma_cmdsl_isValidIdentifier(funcName.p, funcName.l)) {
		if (pErrFormatter != NULL) {
			bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Invalid function name: ");
			bauma_StringBuilder_appendStrWithLen(pErrFormatter, funcName.p, funcName.l);
		}
		return NULL;
	}
	pFunc = bauma_HashMap_get(&pSelf->functions, &funcName, bauma_cmdsl_StrWithLen, bauma_cmdsl_FunctionWithUserData);
	if (pFunc == NULL) {
		if (pErrFormatter != NULL) {
			bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Function not found: ");
			bauma_StringBuilder_appendStrWithLen(pErrFormatter, funcName.p, funcName.l);
		}
		return NULL;
	}
	++n; /* jump after '(' */
	pStr->pStr += n;
	pStr->len -= n;
	if (pStr->len == 0) {
		/* Not even a closing ')' found */
		if (pErrFormatter != NULL) {
			bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "reached end while parsing parameters");
		}
		return NULL;
	}
	pFuncCall = (bauma_CmdslNodeFunctionCall*)(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, NULL, sizeof(bauma_CmdslNodeFunctionCall), NULL);
	bauma_cmdsl_NodeFunctionCall_construct(pFuncCall, pFunc, pSelf->pAlloc);
	while(pStr->len > 0) {
		bauma_CmdslNodeAndAlloc arg;
		char c = *pStr->pStr;
		arg.pAlloc = pSelf->pAlloc;
		if (c == ')') {
			++pStr->pStr;
			--pStr->len;
			goto out;
		}
		if (c == ',') {
			/* empty parameter */
			bauma_CmdslNodeEmpty *pEmpty;
			pEmpty = (bauma_CmdslNodeEmpty*)(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, NULL, sizeof(bauma_CmdslNodeEmpty), NULL);
			bauma_cmdsl_NodeEmpty_construct(pEmpty);
			arg.pNode = (bauma_ICmdslNode*)pEmpty;
			bauma_Vector_append(&pFuncCall->args, bauma_CmdslNodeAndAlloc, &arg);
			continue;
		}
		arg.pNode = bauma_cmdsl_parseElement(pSelf, pStr, pErrFormatter, BAUMA_TRUE);
		if (arg.pNode == NULL) goto err;
		bauma_Vector_append(&pFuncCall->args, bauma_CmdslNodeAndAlloc, &arg);
		if (pStr->len == 0) {
			if (pErrFormatter != NULL) {
				bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "reached end while before ')'");
			}
			goto err;
		}
		c = *pStr->pStr;
		if (c == ',') {
			++pStr->pStr;
			--pStr->len;
		}
	}
err:
	bauma_cmdsl_NodeFunctionCall_destruct(pFuncCall);
	(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pFuncCall, 0, NULL);
	return NULL;
out:
	return (bauma_ICmdslNode*)pFuncCall;
}

BAUMA_CMDSL_DEF bauma_ICmdslNode *bauma_cmdsl_parseElement(bauma_Cmdsl *pSelf, bauma_cmdsl_ParseSource* pStr, bauma_StringBuilder *pErrFormatter, bauma_bool_t insideFunction) {
	if (pStr->len == 0) {
		if (pErrFormatter != NULL) {
			bauma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "reached end while parsing element");
		}
		return NULL;
	}
	if (pStr->pStr[0] == pSelf->escapeChar) {
		if (bauam_cmdsl_isNonFunctionCallEscape(pSelf, pStr)) {
			return bauma_cmdsl_parseText(pSelf, pStr, pErrFormatter, insideFunction);
		}
		return bauma_cmdsl_parseFunctionCall(pSelf, pStr, pErrFormatter);
	}
	return bauma_cmdsl_parseText(pSelf, pStr, pErrFormatter, insideFunction);
}

BAUMA_CMDSL_DEF bauma_ICmdslNode *bauma_cmdsl_parseSequence(bauma_Cmdsl *pSelf, bauma_cmdsl_ParseSource* pStr, bauma_StringBuilder *pErrFormatter, bauma_bool_t insideFunction) {
	bauma_CmdslNodeSequence* pSequence = NULL;
	bauma_ICmdslNode* pElement = NULL;
	bauma_cmdsl_assert(pSelf != NULL);
	bauma_cmdsl_assert(pStr != NULL);
	while(pStr->len > 0) {
		if (pElement != NULL) { /* More than one exists: Have to create a sequence */
			if (pSequence == NULL) {
				pSequence = (bauma_CmdslNodeSequence*)(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, NULL, sizeof(bauma_CmdslNodeSequence), NULL);
				bauma_cmdsl_NodeSequence_construct(pSequence, pSelf->pAlloc);
			}
			bauma_cmdsl_NodeSequence_append(pSequence, pElement);
		}
		pElement = bauma_cmdsl_parseElement(pSelf, pStr, pErrFormatter, insideFunction);
		if (pElement == NULL) goto err;
	}
	/* everything parsed: */
	if (pSequence != NULL) {
		bauma_cmdsl_NodeSequence_append(pSequence, pElement);
		return (bauma_ICmdslNode*)pSequence;
	}
	/* Was only one: No sequence needed */
	return pElement;
err:
	if (pSequence != NULL) {
		(*pSequence->base.pDestruct)(pSequence);
		(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSequence, 0, NULL);
	}
	return NULL;
}

BAUMA_CMDSL_DEF bauma_bool_t bauma_cmdsl_parseMem_ext(bauma_Cmdsl *pSelf, const void *pMem, size_t memSize, bauma_StringBuilder *pErrFormatter) {
	bauma_ICmdslNode *pNode;
	bauma_cmdsl_ParseSource str;
	(void)pSelf;
	(void)pMem;
	(void)memSize;
	(void)pErrFormatter;
	str.pStr = (const char*)pMem;
	str.pBegin = str.pStr;
	str.len = memSize;
	pNode = bauma_cmdsl_parseSequence(pSelf, &str, pErrFormatter, BAUMA_FALSE);
	if (pNode != NULL) {
		if (pSelf->pRootNode != NULL) {
			(*pSelf->pRootNode->pDestruct)(pSelf->pRootNode);
			(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pRootNode, 0, NULL);
		}
		pSelf->pRootNode = pNode;
		return BAUMA_TRUE;
	}
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
	BAUMA_EXPECT(bauma_HashMap_getSize(&cmdsl.functions) > 0);
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

void test_parseSimpleText(void) {
	bauma_Cmdsl cmdsl;
	bauma_StringBuilder b;
	bauma_StringBuilder_construct(&b);
	bauma_cmdsl_construct(&cmdsl);
	BAUMA_EXPECT(bauma_cmdsl_parseStr(&cmdsl, "ABC"));
	BAUMA_EXPECT(cmdsl.pRootNode != NULL);
	(*cmdsl.pRootNode->pEval)(cmdsl.pRootNode, &b, &cmdsl);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "ABC") == 0);
	bauma_StringBuilder_clear(&b);
	BAUMA_EXPECT(bauma_cmdsl_parseStr(&cmdsl, ","));
	BAUMA_EXPECT(cmdsl.pRootNode != NULL);
	(*cmdsl.pRootNode->pEval)(cmdsl.pRootNode, &b, &cmdsl);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), ",") == 0);
	bauma_StringBuilder_destruct(&b);
	bauma_cmdsl_destruct(&cmdsl);
}

void test_parseEscapes(void) {
	bauma_Cmdsl cmdsl;
	bauma_StringBuilder b;
	bauma_StringBuilder_construct(&b);
	bauma_cmdsl_construct(&cmdsl);
	BAUMA_EXPECT(bauma_cmdsl_parseStr(&cmdsl,
		"%% %( %) %, % \t \n %* comment %* nested comment *% still in comment *% %/ line comment\n"
		"%/ other line comment\r\nx"
	));
	BAUMA_EXPECT(cmdsl.pRootNode != NULL);
	(*cmdsl.pRootNode->pEval)(cmdsl.pRootNode, &b, &cmdsl);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "% ( ) ,  x") == 0);
	bauma_StringBuilder_destruct(&b);
	bauma_cmdsl_destruct(&cmdsl);
}

void test_setGet(void) {
	bauma_Cmdsl cmdsl;
	bauma_StringBuilder b;
	bauma_StringBuilder_construct(&b);
	bauma_cmdsl_construct(&cmdsl);
	bauma_cmdsl_setVariable(&cmdsl, "Y", "42");
	BAUMA_EXPECT(bauma_cmdsl_parseStr(&cmdsl, "%set(X,4711)%get(X) %get(Y)"));
	(*cmdsl.pRootNode->pEval)(cmdsl.pRootNode, &b, &cmdsl);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "4711 42") == 0);
	bauma_StringBuilder_destruct(&b);
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
	BAUMA_TEST(test_parseSimpleText);
	BAUMA_TEST(test_parseEscapes);
	BAUMA_TEST(test_setGet);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BAUMA_CMDSL_TEST */

#endif /* BAUMA_CMDSL_H_INCLUDED */
