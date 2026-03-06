#ifndef BMA_CMDSL_H_INCLUDED
#define BMA_CMDSL_H_INCLUDED

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
  of the template with bma_Cmdsl_setVar().
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
that are pre-set using bma_Cmdsl_setVar().

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

typedef struct bma_CmdslVar {
	bma_StrN    name;
	bma_StrBldr value;
} bma_CmdslVar;

BMA_DEF void bma_CmdslVar_dtor(bma_CmdslVar *p, bma_IMemAlloc *pAlloc);

typedef struct bma_Cmdsl bma_Cmdsl;

typedef bma_bool_t (*bma_cmdsl_func_t)(bma_StrBldr *pDst, bma_Cmdsl* pCmdsl, void *pUserData, const bma_StrBldr* pParams, size_t numOfParams);

typedef struct bma_CmdslFuncWthUsrData {
	bma_cmdsl_func_t pFunc;
	void *pUserData; /* optional, heap-allocated */
	bma_dtor_t pUserDataDtor;  /* optional */
} bma_CmdslFuncWthUsrData;

BMA_DEF void bma_CmdslFuncWthUsrData_dtor(bma_CmdslFuncWthUsrData* pSelf, bma_IMemAlloc *pAlloc);

typedef struct bma_ICmdslNode bma_ICmdslNode;

struct bma_Cmdsl {
	bma_HshMp       functions;
	bma_HshMp       variables;
	bma_Vec         pushedVariables;
	bma_Vec         tmpStrBldrs;
	bma_Vec         tmpStrBldrsVecs;
	bma_ICmdslNode  *pRootNode;
	bma_IMemAlloc   *pAlloc;
	char escapeChar;
};

BMA_DEF void bma_Cmdsl_ctor_ext(bma_Cmdsl *pSelf, char escapeChar, bma_IMemAlloc *pAlloc);
#define bma_Cmdsl_ctor(pSelf) bma_Cmdsl_ctor_ext((pSelf), '%', bma_getDfltMemAlloc())

BMA_DEF void bma_Cmdsl_dtor(bma_Cmdsl *pSelf, bma_IMemAlloc *pAlloc);

BMA_DEF bma_bool_t bma_Cmdsl_addFunc(bma_Cmdsl *pSelf, const char *pFuncName, bma_CmdslFuncWthUsrData* p);

BMA_DEF bma_bool_t bma_Cmdsl_setVar(bma_Cmdsl *pSelf, const char *pVarName, const char *pValue);
BMA_DEF bma_bool_t bma_Cmdsl_setVar_ext(bma_Cmdsl *pSelf, const char *pVarName, size_t nameLen, const char *pValue, size_t valueLen);


BMA_DEF const char *bma_Cmdsl_getVar(bma_Cmdsl *pSelf, const char *pVarName);
BMA_DEF const bma_StrBldr *bma_Cmdsl_getVar_ext(bma_Cmdsl *pSelf, const char *pVarName, size_t nameLen);

BMA_DEF bma_bool_t bma_Cmdsl_parseMem_ext(bma_Cmdsl *pSelf, const void *pMem, size_t memSize, bma_StrBldr *pErrFormatter);
#define bma_Cmdsl_parseMem(pSelf, pMem, memSize) bma_Cmdsl_parseMem_ext((pSelf), (pMem), (memSize), NULL)

BMA_DEF bma_bool_t bma_Cmdsl_parseStr_ext(bma_Cmdsl *pSelf, const char *pStr, bma_StrBldr *pErrFormatter);
#define bma_Cmdsl_parseStr(pSelf, pStr) bma_Cmdsl_parseStr_ext((pSelf), (pStr), NULL)

BMA_DEF void bma_Cmdsl_allcTmpStrBldrs(bma_Cmdsl *pSelf, size_t num, bma_Vec *pOut);
BMA_DEF void bma_Cmdsl_freeTmpStrBldrs(bma_Cmdsl *pSelf, bma_Vec *pTmpStrBldrs);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_CMDSL_IMPL

#include <string.h>
#include <ctype.h>


#ifdef __cplusplus
	extern "C" {
#endif

#define BMA_CMDSL_MAX_NUM_PARAMS 8u

struct bma_ICmdslNode {
	void (*pDestruct)(void* pSelf, bma_IMemAlloc *pAlloc);
	bma_bool_t (*pEval)(void* pSelf, bma_StrBldr* pDst, bma_Cmdsl* pCmdsl);
};


typedef struct bma_cmdsl_ParseSrc {
	const char *pBegin; /* for error reporting to calculate error offset */
	const char *pStr; /* incremented while parsing */
	size_t len; /* decremented while parsing */
} bma_cmdsl_ParseSrc;

/*
typedef bma_bool_t (*bma_cmdsl_pFunction)(bma_StrBldr *pDst, bma_Cmdsl* pCmdsl, void *pUserData, const bma_StrN* pParams, size_t numOfParams);
*/

BMA_DEF bma_bool_t bma_cmdsl_funcSet(bma_StrBldr *pDst, bma_Cmdsl* pCmdsl, void *pUserData, const bma_StrBldr* pParams, size_t numOfParams) {
	(void)pUserData;
	if (numOfParams != 2) {
		bma_StrBldr_clear(pDst);
		bma_StrBldr_appndStr(pDst, "Wrong number of parameters for function %set(), expected 2 but got ");
		bma_StrBldr_appndUnsgnd(pDst, numOfParams);
		return BMA_FALSE;
	}
	bma_Cmdsl_setVar_ext(pCmdsl, bma_StrBldr_getStr(&pParams[0]),
                                     bma_StrBldr_getSz(&pParams[0]),
                                     bma_StrBldr_getStr(&pParams[1]),
                                     bma_StrBldr_getSz(&pParams[1]));
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_cmdsl_funcGet(bma_StrBldr *pDst, bma_Cmdsl* pCmdsl, void *pUserData, const bma_StrBldr* pParams, size_t numOfParams) {
	(void)pUserData;
	const bma_StrBldr *pValue;
	if (numOfParams != 1) {
		bma_StrBldr_clear(pDst);
		bma_StrBldr_appndStr(pDst, "Wrong number of parameters for function %get(), expected 1 but got ");
		bma_StrBldr_appndUnsgnd(pDst, numOfParams);
		return BMA_FALSE;
	}
	pValue = bma_Cmdsl_getVar_ext(pCmdsl, bma_StrBldr_getStr(&pParams[0]), bma_StrBldr_getSz(&pParams[0]));
	if (pValue != NULL) {
		bma_StrBldr_appndStrN(pDst, bma_StrBldr_getStr(pValue), bma_StrBldr_getSz(pValue));
	}
	return BMA_TRUE;
}

BMA_DEF void bma_CmdslVar_dtor(bma_CmdslVar *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_assert(pAlloc != NULL);
	bma_StrN_dtor(&pSelf->name, pAlloc);
	bma_StrBldr_dtor(&pSelf->value, NULL);
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF void bma_CmdslNodePtr_dtor(bma_ICmdslNode **ppSelf, bma_IMemAlloc *pAlloc) {
	bma_ICmdslNode* pSelf;
	bma_assert(ppSelf != NULL);
	bma_assert(pAlloc != NULL);
	pSelf = *ppSelf;
	bma_assert(pSelf != NULL);
	(*pSelf->pDestruct)(pSelf, pAlloc);
	bma_free_ext(pAlloc, pSelf);
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

typedef struct bma_CmdslNodeText {
	bma_ICmdslNode base;
	bma_StrN text;
} bma_CmdslNodeText;

BMA_DEF void bma_CmdslNodeText_dtor(bma_CmdslNodeText *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_assert(pAlloc != NULL);
	bma_StrN_dtor(&pSelf->text, pAlloc);
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF bma_bool_t bma_CmdslNodeText_eval(void *pSelf_, bma_StrBldr *pDst, bma_Cmdsl* pCmdsl) {
	bma_CmdslNodeText *pSelf = (bma_CmdslNodeText*)pSelf_;
	bma_assert(pSelf != NULL);
	bma_assert(pDst != NULL);
	bma_assert(pCmdsl != NULL);
	bma_StrBldr_appndStrN(pDst, pSelf->text.p, pSelf->text.len);
	return BMA_TRUE;
}

BMA_DEF void bma_CmdslNodeText_ctor(bma_CmdslNodeText *pSelf, const char *pText, size_t textLen, bma_bool_t xferOwnership, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_assert(pText != NULL);
	bma_assert(pAlloc != NULL);
	pSelf->base.pDestruct = (bma_dtor_t)&bma_CmdslNodeText_dtor;
	pSelf->base.pEval = &bma_CmdslNodeText_eval;
	if (xferOwnership) {
		pSelf->text.p = (char*)pText;
	}
	else {
		pSelf->text.p = bma_strndup_ext(pText, textLen, pAlloc);
	}
	pSelf->text.len = textLen;
}

typedef struct bma_CmdslNodeEmpty {
	bma_ICmdslNode base;
} bma_CmdslNodeEmpty;

BMA_DEF void bma_CmdslNodeEmpty_dtor(bma_CmdslNodeEmpty *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	(void)pAlloc;
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF bma_bool_t bma_CmdslNodeEmpty_eval(void *pSelf, bma_StrBldr *pDst, bma_Cmdsl* pCmdsl) {
	bma_assert(pSelf != NULL);
	bma_assert(pDst != NULL);
	bma_assert(pCmdsl != NULL);
	(void)pSelf;
	(void)pDst;
	(void)pCmdsl;
	return BMA_TRUE;
}

BMA_DEF void bma_CmdslNodeEmpty_ctor(bma_CmdslNodeEmpty *pSelf) {
	bma_assert(pSelf != NULL);
	pSelf->base.pDestruct = (bma_dtor_t)&bma_CmdslNodeEmpty_dtor;
	pSelf->base.pEval = &bma_CmdslNodeEmpty_eval;
}
		
typedef struct bma_CmdslNodeFuncCall {
	bma_ICmdslNode base;
	bma_CmdslFuncWthUsrData *pFunc;
	bma_Vec args;
} bma_CmdslNodeFuncCall;

BMA_DEF void bma_cmdsl_NodeFunctionCall_destruct(bma_CmdslNodeFuncCall *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_Vec_dtor(&pSelf->args, NULL);
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF bma_bool_t bma_cmdsl_NodeFunctionCall_eval(void *pSelf_, bma_StrBldr *pDst, bma_Cmdsl* pCmdsl) {
	bma_CmdslNodeFuncCall *pSelf = (bma_CmdslNodeFuncCall*)pSelf_;
	bma_CmdslFuncWthUsrData *pFunc;
	bma_Vec tmpStrBldrs;
	size_t i, nargs;
	bma_bool_t result = BMA_TRUE;
	bma_assert(pSelf != NULL);
	bma_assert(pDst != NULL);
	bma_assert(pCmdsl != NULL);
	bma_assert(pSelf->pFunc != NULL);
	pFunc = pSelf->pFunc;
	nargs = bma_Vec_getSz(&pSelf->args);
	bma_Cmdsl_allcTmpStrBldrs(pCmdsl, nargs, &tmpStrBldrs);
	for (i=0; i<nargs; ++i) {
		/* evaluate arguments */
		bma_ICmdslNode *pArgNode = *bma_Vec_at(&pSelf->args, i, bma_ICmdslNode*);
		bma_StrBldr *pArgStrBldr = bma_Vec_at(&tmpStrBldrs, i, bma_StrBldr);
		if (!(*pArgNode->pEval)(pArgNode, pArgStrBldr, pCmdsl)) {
			result = BMA_FALSE;
			goto cleanup;
		}
	}
	/* call function with args, results stored in pDst */
	result = (*pFunc->pFunc)(pDst, pCmdsl, pFunc->pUserData, bma_Vec_getData(&tmpStrBldrs, bma_StrBldr), nargs);
cleanup:
	bma_Cmdsl_freeTmpStrBldrs(pCmdsl, &tmpStrBldrs);
	return result;
}

BMA_DEF void bma_cmdsl_NodeFunctionCall_construct(bma_CmdslNodeFuncCall *pSelf, bma_CmdslFuncWthUsrData *pFunc, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_assert(pFunc != NULL);
	bma_assert(pAlloc != NULL);
	pSelf->base.pDestruct = (bma_dtor_t)&bma_cmdsl_NodeFunctionCall_destruct;
	pSelf->base.pEval = &bma_cmdsl_NodeFunctionCall_eval;
	pSelf->pFunc = pFunc;
	bma_Vec_ctor_ext(&pSelf->args, bma_ICmdslNode*, (bma_dtor_t)&bma_CmdslNodePtr_dtor, pAlloc);
}

typedef struct bma_CmdslNodeSeq {
	bma_ICmdslNode base;
	bma_Vec nodes; /* of bma_CmdslNodeAndAlloc* */
} bma_CmdslNodeSeq;

BMA_DEF void bma_CmdslNodeSeq_dtor(bma_CmdslNodeSeq *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	(void)pAlloc;
	bma_Vec_dtor(&pSelf->nodes, NULL);
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF bma_bool_t bma_CmdslNodeSeq_eval(void *pSelf_, bma_StrBldr *pDst, bma_Cmdsl* pCmdsl) {
	size_t i;
	bma_CmdslNodeSeq *pSelf = (bma_CmdslNodeSeq*)pSelf_;
	bma_assert(pSelf != NULL);
	bma_assert(pDst != NULL);
	bma_assert(pCmdsl != NULL);
	for (i=0; i<bma_Vec_getSz(&pSelf->nodes); ++i) {
		bma_ICmdslNode *pNode = *bma_Vec_at(&pSelf->nodes, i, bma_ICmdslNode*);
		if (!(*pNode->pEval)(pNode, pDst, pCmdsl)) {
			return BMA_FALSE;
		}
	}
	return BMA_TRUE;
}

BMA_DEF void bma_CmdslNodeSeq_ctor(bma_CmdslNodeSeq *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_assert(pAlloc != NULL);
	pSelf->base.pDestruct = (bma_dtor_t)&bma_CmdslNodeSeq_dtor;
	pSelf->base.pEval = &bma_CmdslNodeSeq_eval;
	bma_Vec_ctor_ext(&pSelf->nodes, bma_ICmdslNode*, (bma_dtor_t)&bma_CmdslNodePtr_dtor, pAlloc);
}

BMA_DEF void bma_CmdslNodeSeq_appnd(bma_CmdslNodeSeq *pSelf, bma_ICmdslNode *pNode) {
	bma_assert(pSelf != NULL);
	bma_assert(pNode != NULL);
	bma_Vec_appnd(&pSelf->nodes, bma_ICmdslNode*, &pNode);
}

BMA_DEF void bma_CmdslFuncWthUsrData_dtor(bma_CmdslFuncWthUsrData* pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_assert(pAlloc != NULL);
	if (pSelf->pUserData != NULL) {
		if (pSelf->pUserDataDtor != NULL) {
			(*pSelf->pUserDataDtor)(pSelf->pUserData, pAlloc);
		}
		bma_free_ext(pAlloc, pSelf->pUserData);
	}
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF void bma_Cmdsl_ctor_ext(bma_Cmdsl *pSelf, char escapeChar, bma_IMemAlloc *pAlloc) {
	bma_CmdslFuncWthUsrData fu;
	bma_assert(pSelf != NULL);
	bma_assert(pAlloc != NULL);
	pSelf->escapeChar = escapeChar;
	bma_HshMp_ctor_ext(&pSelf->functions, bma_StrN, bma_CmdslFuncWthUsrData, (bma_dtor_t)&bma_StrN_dtor, (bma_dtor_t)&bma_CmdslFuncWthUsrData_dtor, (bma_hash_t)&bma_StrN_hash, (bma_eq_t)&bma_StrN_eq, pAlloc);
	bma_HshMp_ctor_ext(&pSelf->variables, bma_StrN, bma_StrBldr,
	                            (bma_dtor_t)&bma_StrN_dtor, (bma_dtor_t)&bma_StrBldr_dtor,
	                            (bma_hash_t)&bma_StrN_hash, (bma_eq_t)&bma_StrN_eq, pAlloc);
	bma_Vec_ctor_ext(&pSelf->pushedVariables, bma_CmdslVar, (bma_dtor_t)&bma_CmdslVar_dtor, pAlloc);
	bma_Vec_ctor_ext(&pSelf->tmpStrBldrs, bma_StrBldr, (bma_dtor_t)&bma_StrBldr_dtor, pAlloc);
	bma_Vec_ctor_ext(&pSelf->tmpStrBldrsVecs, bma_Vec, (bma_dtor_t)&bma_Vec_dtor, pAlloc);
	pSelf->pRootNode = NULL;
	pSelf->pAlloc = pAlloc;
	/* add built-in functions */
	fu.pFunc = &bma_cmdsl_funcSet;
	fu.pUserData = NULL;
	fu.pUserDataDtor = NULL;
	bma_Cmdsl_addFunc(pSelf, "set", &fu);
	fu.pFunc = &bma_cmdsl_funcGet;
	fu.pUserData = NULL;
	fu.pUserDataDtor = NULL;
	bma_Cmdsl_addFunc(pSelf, "get", &fu);
}

BMA_DEF void bma_Cmdsl_dtor(bma_Cmdsl *pSelf, bma_IMemAlloc *pAlloc) {
	(void)pAlloc; // bma_Cmdsl has its own
	if (pSelf->pRootNode != NULL) {
		(*pSelf->pRootNode->pDestruct)(pSelf->pRootNode, pSelf->pAlloc);
		bma_free_ext(pSelf->pAlloc, pSelf->pRootNode);
	}
	bma_Vec_dtor(&pSelf->tmpStrBldrsVecs, NULL);
	bma_Vec_dtor(&pSelf->tmpStrBldrs, NULL);
	bma_Vec_dtor(&pSelf->pushedVariables, NULL);
	bma_HshMp_dtor(&pSelf->variables, NULL);
	bma_HshMp_dtor(&pSelf->functions, NULL);
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF bma_bool_t bma_cmdsl_isValidIdentifier(const char* pStr, size_t len) {
	size_t i;
	char c;
	if (len == 0) return BMA_FALSE;
	c = *pStr++;
	if (!(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || (c == '_'))) return BMA_FALSE;
	for (i=1; i<len; ++i) {
		c = *pStr++;
		if (!(((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '_'))) return BMA_FALSE;
	}
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_Cmdsl_addFunc(bma_Cmdsl *pSelf, const char *pFuncName, bma_CmdslFuncWthUsrData* p) {
	size_t funcNameLen;
	bma_CmdslFuncWthUsrData *pExistingFunc;
	bma_assert(pSelf != NULL);
	bma_assert(pFuncName != NULL);
	bma_assert(p != NULL);
	bma_assert(p->pFunc != NULL);
	bma_StrN key;
	funcNameLen = strlen(pFuncName);
	if (!bma_cmdsl_isValidIdentifier(pFuncName, funcNameLen)) {
		return BMA_FALSE;
	}
	/* for look-up, no dyn. allocation is needed */
	key.p = (char*)pFuncName;
	key.len = funcNameLen;
	pExistingFunc = bma_HshMp_get(&pSelf->functions, &key, bma_StrN, bma_CmdslFuncWthUsrData);
	if (pExistingFunc != NULL) {
		/* function already exists */
		return BMA_FALSE;
	}
	/* to put into map, dyn. allocate so string isn't lost */
	key.p = bma_strndup_ext(pFuncName, funcNameLen, pSelf->pAlloc);
        /* len stays same */
	bma_HshMp_put(&pSelf->functions, &key, p, bma_StrN, bma_CmdslFuncWthUsrData);
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_Cmdsl_setVar_ext(bma_Cmdsl *pSelf, const char *pVarName, size_t nameLen, const char *pValue, size_t valueLen) {
	bma_StrBldr *pFoundValue;
	bma_StrBldr newContent;
	bma_assert(pSelf != NULL);
	bma_assert(pVarName != NULL);
	bma_assert(pValue != NULL);
	if (!bma_cmdsl_isValidIdentifier(pVarName, nameLen)) {
		return BMA_FALSE;
	}
	bma_StrN varName;
	/* For look-up, varName doesn't need heap allocation (will be faster)*/
	varName.p = (char*)pVarName;
	varName.len = nameLen;
	pFoundValue = bma_HshMp_get(&pSelf->variables, &varName, bma_StrN, bma_StrBldr);
	if (pFoundValue != NULL) {
		bma_StrBldr_clear(pFoundValue);
		bma_StrBldr_appndStrN(pFoundValue, pValue, valueLen);
		return BMA_TRUE;
	}
	/* now, varName must be heap allocated to not loose the content when stored */
	varName.p = bma_strndup_ext(pVarName, nameLen, pSelf->pAlloc);
	/* len stays same */
	bma_StrBldr_ctor_ext(&newContent, pSelf->pAlloc);
	bma_StrBldr_appndStrN(&newContent, pValue, valueLen);
	bma_HshMp_put(&pSelf->variables, &varName, &newContent, bma_StrN, bma_StrBldr);
	return BMA_TRUE;
}

BMA_DEF bma_bool_t bma_Cmdsl_setVar(bma_Cmdsl *pSelf, const char *pVarName, const char *pValue) {
	bma_assert(pSelf != NULL);
	bma_assert(pVarName != NULL);
	bma_assert(pValue != NULL);
	return bma_Cmdsl_setVar_ext(pSelf, pVarName, strlen(pVarName), pValue, strlen(pValue));
}

BMA_DEF const char *bma_Cmdsl_getVar(bma_Cmdsl *pSelf, const char *pVarName) {
	const bma_StrBldr *pValue;
	bma_assert(pSelf != NULL);
	bma_assert(pVarName != NULL);
	pValue = bma_Cmdsl_getVar_ext(pSelf, pVarName, strlen(pVarName));
	if (pValue != NULL) {
		return bma_StrBldr_getStr(pValue);
	}
	return "";
}

BMA_DEF const bma_StrBldr *bma_Cmdsl_getVar_ext(bma_Cmdsl *pSelf, const char *pVarName, size_t nameLen) {
	bma_StrN varName;
	varName.p = (char*)pVarName;
	varName.len = nameLen;
	return bma_HshMp_get(&pSelf->variables, &varName, bma_StrN, bma_StrBldr);
}


BMA_DEF bma_bool_t bauam_cmdsl_isNonFunctionCallEscape(bma_Cmdsl *pSelf, bma_cmdsl_ParseSrc* pStr) {
	char c;
	bma_assert(pSelf != NULL);
	bma_assert(pStr != NULL);
	if (pStr->len < 2) return BMA_FALSE;
	if (pStr->pStr[0] != pSelf->escapeChar) return BMA_FALSE;
	c = pStr->pStr[1];
	if ((c == pSelf->escapeChar) || (strchr("(),*/", c) != NULL) || isspace(c)) {
		/* string escape or comment but no function call */
		return BMA_TRUE;
	}
	return BMA_FALSE;
}

BMA_DEF bma_ICmdslNode *bma_cmdsl_parseElement(bma_Cmdsl *pSelf, bma_cmdsl_ParseSrc* pStr, bma_StrBldr *pErrFormatter, bma_bool_t insideFunction);

static char CMDSL_ERR_AT_OFFSET[] = "Err at offset ";

BMA_DEF void bma_cmdsl_printErrAtOffs(bma_StrBldr *pErrFormatter, bma_cmdsl_ParseSrc* pStr, const char *pMsg) {
	bma_StrBldr_appndStr(pErrFormatter, CMDSL_ERR_AT_OFFSET);
	bma_StrBldr_appndUnsgnd(pErrFormatter, (size_t)(pStr->pStr - pStr->pBegin));
	bma_StrBldr_appndStr(pErrFormatter, ": ");
	bma_StrBldr_appndStr(pErrFormatter, pMsg);
}

BMA_DEF bma_ICmdslNode *bma_cmdsl_parseText(bma_Cmdsl *pSelf, bma_cmdsl_ParseSrc* pStr, bma_StrBldr *pErrFormatter, bma_bool_t insideFunction) {
	bma_StrBldr b;
	bma_CmdslNodeText *pText;
	char *p;
	size_t l;
	bma_StrBldr_ctor_ext(&b, pSelf->pAlloc);
	while(pStr->len > 0) {
		char c = *pStr->pStr;
		if (insideFunction && ((c == ',') || (c == ')'))) {
			break;
		}
		if (c != pSelf->escapeChar) {
			bma_StrBldr_appndChr(&b, (unsigned char)*pStr->pStr, 1u);
		}
		else
		{
			if (pStr->len < 2u) {
				if (pErrFormatter != NULL) {
					bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Found escape sequence but string is at end");
				}
				goto err;
			}
			c = pStr->pStr[1];
			pStr->pStr += 2u;
			pStr->len -= 2u;
			if ((c == pSelf->escapeChar) || (strchr("(),", c) != NULL)) {
				/* %% or %( or %) or %, */
				bma_StrBldr_appndChr(&b, c, 1u);
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
								bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Found superfluous comment end sequence");
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
	l = bma_StrBldr_getSz(&b);
	p = bma_StrBldr_rlse(&b);
	pText = bma_malloc_ext(pSelf->pAlloc, bma_CmdslNodeText);
	bma_CmdslNodeText_ctor(pText, p, l, BMA_TRUE, pSelf->pAlloc);
	return (bma_ICmdslNode*)pText;
err:
	bma_StrBldr_dtor(&b, NULL);
	return NULL;
}

BMA_DEF bma_ICmdslNode *bma_cmdsl_parseFunctionCall(bma_Cmdsl *pSelf, bma_cmdsl_ParseSrc* pStr, bma_StrBldr *pErrFormatter) {
	const char *p;
	size_t n;
	bma_StrN funcName;
	bma_CmdslFuncWthUsrData* pFunc;
	bma_CmdslNodeFuncCall* pFuncCall;
	if ((pStr->len == 0) || (*pStr->pStr != pSelf->escapeChar)) {
		if (pErrFormatter != NULL) {
			bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "reached end while parsing function call");
		}
		return NULL;
	}
	++pStr->pStr;
	--pStr->len;
	p = memchr(pStr->pStr, '(', pStr->len);
	if (p == NULL) {
		if (pErrFormatter != NULL) {
			bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "'(' not found");
		}
		return NULL;
	}
	n = (size_t)(p - pStr->pStr); /* length of function name */
	if (n == 0) {
		if (pErrFormatter != NULL) {
			bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Function with zero name not allowed");
		}
		return NULL;
	}
	funcName.p = (char*)pStr->pStr;
	funcName.len = n;
	if (!bma_cmdsl_isValidIdentifier(funcName.p, funcName.len)) {
		if (pErrFormatter != NULL) {
			bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Invalid function name: ");
			bma_StrBldr_appndStrN(pErrFormatter, funcName.p, funcName.len);
		}
		return NULL;
	}
	pFunc = bma_HshMp_get(&pSelf->functions, &funcName, bma_StrN, bma_CmdslFuncWthUsrData);
	if (pFunc == NULL) {
		if (pErrFormatter != NULL) {
			bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "Function not found: ");
			bma_StrBldr_appndStrN(pErrFormatter, funcName.p, funcName.len);
		}
		return NULL;
	}
	++n; /* jump after '(' */
	pStr->pStr += n;
	pStr->len -= n;
	if (pStr->len == 0) {
		/* Not even a closing ')' found */
		if (pErrFormatter != NULL) {
			bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "reached end while parsing parameters");
		}
		return NULL;
	}
	pFuncCall = bma_malloc_ext(pSelf->pAlloc, bma_CmdslNodeFuncCall);
	bma_cmdsl_NodeFunctionCall_construct(pFuncCall, pFunc, pSelf->pAlloc);
	while(pStr->len > 0) {
		bma_ICmdslNode* pArg;
		char c = *pStr->pStr;
		if (c == ')') {
			++pStr->pStr;
			--pStr->len;
			goto out;
		}
		if (c == ',') {
			/* empty parameter */
			bma_CmdslNodeEmpty *pEmpty;
			pEmpty = bma_malloc_ext(pSelf->pAlloc, bma_CmdslNodeEmpty);
			bma_CmdslNodeEmpty_ctor(pEmpty);
			pArg = (bma_ICmdslNode*)pEmpty;
			bma_Vec_appnd(&pFuncCall->args, bma_ICmdslNode*, &pArg);
			continue;
		}
		pArg = bma_cmdsl_parseElement(pSelf, pStr, pErrFormatter, BMA_TRUE);
		if (pArg == NULL) goto err;
		bma_Vec_appnd(&pFuncCall->args, bma_ICmdslNode*, &pArg);
		if (pStr->len == 0) {
			if (pErrFormatter != NULL) {
				bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "reached end while before ')'");
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
	bma_cmdsl_NodeFunctionCall_destruct(pFuncCall, pSelf->pAlloc);
	bma_free_ext(pSelf->pAlloc, pFuncCall);
	return NULL;
out:
	return (bma_ICmdslNode*)pFuncCall;
}

BMA_DEF bma_ICmdslNode *bma_cmdsl_parseElement(bma_Cmdsl *pSelf, bma_cmdsl_ParseSrc* pStr, bma_StrBldr *pErrFormatter, bma_bool_t insideFunction) {
	if (pStr->len == 0) {
		if (pErrFormatter != NULL) {
			bma_cmdsl_printErrAtOffs(pErrFormatter, pStr, "reached end while parsing element");
		}
		return NULL;
	}
	if (pStr->pStr[0] == pSelf->escapeChar) {
		if (bauam_cmdsl_isNonFunctionCallEscape(pSelf, pStr)) {
			return bma_cmdsl_parseText(pSelf, pStr, pErrFormatter, insideFunction);
		}
		return bma_cmdsl_parseFunctionCall(pSelf, pStr, pErrFormatter);
	}
	return bma_cmdsl_parseText(pSelf, pStr, pErrFormatter, insideFunction);
}

BMA_DEF bma_ICmdslNode *bma_cmdsl_parseSequence(bma_Cmdsl *pSelf, bma_cmdsl_ParseSrc* pStr, bma_StrBldr *pErrFormatter, bma_bool_t insideFunction) {
	bma_CmdslNodeSeq* pSequence = NULL;
	bma_ICmdslNode* pElement = NULL;
	bma_assert(pSelf != NULL);
	bma_assert(pStr != NULL);
	while(pStr->len > 0) {
		if (pElement != NULL) { /* More than one exists: Have to create a sequence */
			if (pSequence == NULL) {
				pSequence = bma_malloc_ext(pSelf->pAlloc, bma_CmdslNodeSeq);
				bma_CmdslNodeSeq_ctor(pSequence, pSelf->pAlloc);
			}
			bma_CmdslNodeSeq_appnd(pSequence, pElement);
		}
		pElement = bma_cmdsl_parseElement(pSelf, pStr, pErrFormatter, insideFunction);
		if (pElement == NULL) goto err;
	}
	/* everything parsed: */
	if (pSequence != NULL) {
		bma_CmdslNodeSeq_appnd(pSequence, pElement);
		return (bma_ICmdslNode*)pSequence;
	}
	/* Was only one: No sequence needed */
	return pElement;
err:
	if (pSequence != NULL) {
		(*pSequence->base.pDestruct)(pSequence, pSelf->pAlloc);
		bma_free_ext(pSelf->pAlloc, pSequence);
	}
	return NULL;
}

BMA_DEF bma_bool_t bma_Cmdsl_parseMem_ext(bma_Cmdsl *pSelf, const void *pMem, size_t memSize, bma_StrBldr *pErrFormatter) {
	bma_ICmdslNode *pNode;
	bma_cmdsl_ParseSrc str;
	(void)pSelf;
	(void)pMem;
	(void)memSize;
	(void)pErrFormatter;
	str.pStr = (const char*)pMem;
	str.pBegin = str.pStr;
	str.len = memSize;
	pNode = bma_cmdsl_parseSequence(pSelf, &str, pErrFormatter, BMA_FALSE);
	if (pNode != NULL) {
		if (pSelf->pRootNode != NULL) {
			(*pSelf->pRootNode->pDestruct)(pSelf->pRootNode, pSelf->pAlloc);
			bma_free_ext(pSelf->pAlloc, pSelf->pRootNode);
		}
		pSelf->pRootNode = pNode;
		return BMA_TRUE;
	}
	return BMA_FALSE;
}

BMA_DEF bma_bool_t bma_Cmdsl_parseStr_ext(bma_Cmdsl *pSelf, const char *pStr, bma_StrBldr *pErrFormatter) {
	size_t len;
	bma_assert(pSelf != NULL);
	bma_assert(pStr != NULL);
	len = strlen(pStr);
	return bma_Cmdsl_parseMem_ext(pSelf, pStr, len, pErrFormatter);
}

BMA_DEF void bma_Cmdsl_allcTmpStrBldrs(bma_Cmdsl *pSelf, size_t num, bma_Vec *pOut) {
	size_t numExstng, rest, i, n;
	bma_assert(pSelf != NULL);
	bma_assert(pOut != NULL);
	if (bma_Vec_getSz(&pSelf->tmpStrBldrsVecs) > 0) {
		bma_Vec_rmv(&pSelf->tmpStrBldrsVecs, bma_Vec_getSz(&pSelf->tmpStrBldrsVecs) - 1u, pOut, bma_Vec);
		bma_Vec_clear(pOut);
	}
	else {
		bma_Vec_ctor_ext(pOut, bma_StrBldr, (bma_dtor_t)&bma_StrBldr_dtor, pSelf->pAlloc);
	}
	numExstng = bma_Vec_getSz(&pSelf->tmpStrBldrs);
	n = bma_min(num, numExstng);
	for (i=0; i<n; ++i) {
		bma_StrBldr b;
		bma_Vec_rmv(&pSelf->tmpStrBldrs, bma_Vec_getSz(&pSelf->tmpStrBldrs) - 1u, &b, bma_StrBldr);
		bma_StrBldr_clear(&b);
		bma_Vec_appnd(pOut, bma_StrBldr, &b);
	}
	if (num <= numExstng) return;
	rest = num - numExstng;
	for (i=0; i<rest; ++i) {
		bma_StrBldr b;
		bma_StrBldr_ctor_ext(&b, pSelf->pAlloc);
		bma_Vec_appnd(pOut, bma_StrBldr, &b);
	}
}

BMA_DEF void bma_Cmdsl_freeTmpStrBldrs(bma_Cmdsl *pSelf, bma_Vec *pTmpStrBldrs) {
	while(bma_Vec_getSz(pTmpStrBldrs) > 0) {
		bma_StrBldr b;
		bma_Vec_rmv(pTmpStrBldrs, bma_Vec_getSz(pTmpStrBldrs) - 1u, &b, bma_StrBldr);
		bma_Vec_appnd(&pSelf->tmpStrBldrs, bma_StrBldr, &b);
	}
	bma_Vec_appnd(&pSelf->tmpStrBldrsVecs, bma_Vec, pTmpStrBldrs);
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_CMDSL_IMPLEMENTATION */

#ifdef BMA_CMDSL_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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

void test_construct(void) {
	bma_Cmdsl cmdsl;
	bma_Cmdsl_ctor(&cmdsl);
	BMA_EXPECT(bma_HshMp_getSz(&cmdsl.functions) > 0);
	BMA_EXPECT(bma_HshMp_getSz(&cmdsl.variables) == 0);
	BMA_EXPECT(bma_Vec_getSz(&cmdsl.pushedVariables) == 0);
	BMA_EXPECT(cmdsl.pRootNode == NULL);
	bma_Cmdsl_dtor(&cmdsl, NULL);
}

void test_setGetVariable(void) {
	bma_Cmdsl cmdsl;
	bma_Cmdsl_ctor(&cmdsl);
	bma_Cmdsl_setVar(&cmdsl, "var1", "value1");
	BMA_EXPECT(strcmp(bma_Cmdsl_getVar(&cmdsl, "var1"), "value1") == 0);
	bma_Cmdsl_setVar(&cmdsl, "var1", "value2");
	BMA_EXPECT(strcmp(bma_Cmdsl_getVar(&cmdsl, "var1"), "value2") == 0);
	bma_Cmdsl_setVar(&cmdsl, "var2", "value3");
	BMA_EXPECT(strcmp(bma_Cmdsl_getVar(&cmdsl, "var2"), "value3") == 0);
	BMA_EXPECT(strcmp(bma_Cmdsl_getVar(&cmdsl, "Unknown"), "") == 0);
	BMA_EXPECT(bma_HshMp_getSz(&cmdsl.variables) == 2u);
	bma_Cmdsl_dtor(&cmdsl, NULL);
}

void test_parseSimpleText(void) {
	bma_Cmdsl cmdsl;
	bma_StrBldr b;
	bma_StrBldr_ctor(&b);
	bma_Cmdsl_ctor(&cmdsl);
	BMA_EXPECT(bma_Cmdsl_parseStr(&cmdsl, "ABC"));
	BMA_EXPECT(cmdsl.pRootNode != NULL);
	(*cmdsl.pRootNode->pEval)(cmdsl.pRootNode, &b, &cmdsl);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "ABC") == 0);
	bma_StrBldr_clear(&b);
	BMA_EXPECT(bma_Cmdsl_parseStr(&cmdsl, ","));
	BMA_EXPECT(cmdsl.pRootNode != NULL);
	(*cmdsl.pRootNode->pEval)(cmdsl.pRootNode, &b, &cmdsl);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), ",") == 0);
	bma_StrBldr_dtor(&b, NULL);
	bma_Cmdsl_dtor(&cmdsl, NULL);
}

void test_parseEscapes(void) {
	bma_Cmdsl cmdsl;
	bma_StrBldr b;
	bma_StrBldr_ctor(&b);
	bma_Cmdsl_ctor(&cmdsl);
	BMA_EXPECT(bma_Cmdsl_parseStr(&cmdsl,
		"%% %( %) %, % \t \n %* comment %* nested comment *% still in comment *% %/ line comment\n"
		"%/ other line comment\r\nx"
	));
	BMA_EXPECT(cmdsl.pRootNode != NULL);
	(*cmdsl.pRootNode->pEval)(cmdsl.pRootNode, &b, &cmdsl);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "% ( ) ,  x") == 0);
	bma_StrBldr_dtor(&b, NULL);
	bma_Cmdsl_dtor(&cmdsl, NULL);
}

void test_setGet(void) {
	bma_Cmdsl cmdsl;
	bma_StrBldr b;
	bma_StrBldr_ctor(&b);
	bma_Cmdsl_ctor(&cmdsl);
	bma_Cmdsl_setVar(&cmdsl, "Y", "42");
	BMA_EXPECT(bma_Cmdsl_parseStr(&cmdsl, "%set(X,4711)%get(X) %get(Y)"));
	(*cmdsl.pRootNode->pEval)(cmdsl.pRootNode, &b, &cmdsl);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "4711 42") == 0);
	bma_StrBldr_dtor(&b, NULL);
	bma_Cmdsl_dtor(&cmdsl, NULL);
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_construct);
	BMA_TEST(test_setGetVariable);
	BMA_TEST(test_parseSimpleText);
	BMA_TEST(test_parseEscapes);
	BMA_TEST(test_setGet);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_CMDSL_TEST */

#endif /* BMA_CMDSL_H_INCLUDED */
