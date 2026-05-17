#ifndef BMA_NSON_H_INCLUDED
#define BMA_NSON_H_INCLUDED

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

Bau 'ma's minimalistic NSON parser (NSON = Nice JSON).
Allows normal JSON but also some specialities to make the language
more pleasant:

- Commas and colons in lists and maps are optional, so [1 2 3] is a valid list as long
  as whitespace is used as delimiter
- Allows C-, C++ and python style comments
- Any token which isn't "true", "false" or "null" is considered a string, so "Hello" can
  be also written Hello. Such tokens aren't allowed to contain whitespace of course.

So, for instance, this is recognized as valid NSON:

{
    name: Arnold, // The name
    age: 34,      // The age
}

This makes it much more comfortable than this in pure JSON:

{
    "name_comment": "The name",
    "name": "Arnold",
    "age_comment": "The age",
    "age": 34
}

The above example could even be written shorter:

{
    name Arnold
    age 34
}

although readability would suffer from that, but the parser would still be happy: The ":" and "," are optional anyway.

Having those additional features doesn't make the parser much more complicated
but makes writing this json dialect a joy.
*/

#include <limits.h>
#include <float.h>
#include <bma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct bma_NsonParseSrc {
	const char *pBegin; /* for error reporting to calculate error offset */
	const char *pStr; /* incremented while parsing */
	size_t len; /* decremented while parsing */
} bma_NsonParseSrc;

typedef unsigned int bma_NsonNodeType;

#define BMA_NSON_NODE_TYPE_INVLD  0
#define BMA_NSON_NODE_TYPE_NULL   1
#define BMA_NSON_NODE_TYPE_BOOL   2
#define BMA_NSON_NODE_TYPE_SGND   3
#define BMA_NSON_NODE_TYPE_UNSGND 4
#define BMA_NSON_NODE_TYPE_DBL    5
#define BMA_NSON_NODE_TYPE_STR    6
#define BMA_NSON_NODE_TYPE_ARR    7
#define BMA_NSON_NODE_TYPE_OBJ    8
#define BMA_NSON_NODE_NUM_TYPES   9

typedef unsigned int bma_NsonDialect;

#define BMA_NSON_DIALECT_JSON         0
#define BMA_NSON_DIALECT_NSON         1
#define BMA_NSON_DIALECT_NUM_DIALECTS 2

typedef struct bma_NsonNode bma_NsonNode;

BMA_DEF void bma_nsonNodePtrDtor(void *ppNode, bma_IMemAlloc *pAlloc);

BMA_DEF_VEC(bma_NsonNodePtrVec, bma_NsonNode*, &bma_nsonNodePtrDtor)

typedef union bma_NsonNodeUnion {
	bma_bool_t         b;
	bma_intmax_t       si;
	bma_uintmax_t      ui;
	double             d;
	char               *p;
	bma_NsonNodePtrVec v; /* for maps and arrays, this points to all values */
} bma_NsonNodeUnion;

struct bma_NsonNode {
	bma_NsonNodeType  type;
	bma_NsonNodeUnion value;
	bma_NsonNode      *pObjectValue; /* ptr from key to value in objs */
	bma_IMemAlloc     *pAlloc;
};

BMA_DEF bma_NsonNode *bma_nsonNew_ext(bma_IMemAlloc *pAlloc);
#define bma_nsonNew() bma_nsonNew_ext(bma_getDfltMemAlloc())

BMA_DEF bma_NsonNode *bma_nsonNewNull_ext(bma_IMemAlloc *pAlloc);
#define bma_nsonNewNull() bma_nsonNewNull_ext(bma_getDfltMemAlloc())

BMA_DEF bma_NsonNode *bma_nsonNewBool_ext(bma_bool_t value, bma_IMemAlloc *pAlloc);
#define bma_nsonNewBool(value) bma_nsonNewBool_ext((value), bma_getDfltMemAlloc())

BMA_DEF bma_NsonNode *bma_nsonNewSigned_ext(bma_intmax_t value, bma_IMemAlloc *pAlloc);
#define bma_nsonNewSigned(value) bma_nsonNewSigned_ext((value), bma_getDfltMemAlloc())

BMA_DEF bma_NsonNode *bma_nsonNewUnsigned_ext(bma_uintmax_t value, bma_IMemAlloc *pAlloc);
#define bma_nsonNewUnsigned(value) bma_nsonNewUnsigned_ext((value), bma_getDfltMemAlloc())

BMA_DEF bma_NsonNode *bma_nsonNewDouble_ext(double value, bma_IMemAlloc *pAlloc);
#define bma_nsonNewDouble(value) bma_nsonNewDouble_ext((value), bma_getDfltMemAlloc())

BMA_DEF bma_NsonNode *bma_nsonNewStr_ext(const char *pValue, bma_bool_t xferOwnership, bma_IMemAlloc *pAlloc);
#define bma_nsonNewStr(value) bma_nsonNewStr_ext((value), BMA_FALSE, bma_getDfltMemAlloc())

BMA_DEF bma_NsonNode *bma_nsonNewStrWithLen_ext(const char *pValue, size_t valueLen, bma_IMemAlloc *pAlloc);
#define bma_nsonNewStrWithLen(value, len) bma_nsonNewStrWithLen_ext((value), (len), bma_getDfltMemAlloc())

BMA_DEF bma_NsonNode *bma_nsonNewArr_ext(bma_IMemAlloc *pAlloc);
#define bma_nsonNewArr() bma_nsonNewArr_ext(bma_getDfltMemAlloc())
BMA_DEF void bma_nsonArrAppnd(bma_NsonNode* pNode, bma_NsonNode* pArrayElement);

BMA_DEF bma_NsonNode *bma_nsonNewObj_ext(bma_IMemAlloc *pAlloc);
#define bma_nsonNewObj() bma_nsonNewObj_ext(bma_getDfltMemAlloc())
BMA_DEF void bma_nsonObjAppnd(bma_NsonNode* pNode, bma_NsonNode *pKey, bma_NsonNode *pValue);

BMA_DEF void bma_nsonDelete(bma_NsonNode *pNode);

BMA_DEF bma_NsonNode *bma_nsonParseExt(bma_NsonParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc);

BMA_DEF bma_NsonNode *bma_nsonParseMem(const void *pMem, size_t len);
BMA_DEF bma_NsonNode *bma_nsonParse(const char *pStr);

BMA_DEF bma_NsonNode *bma_nsonGet(bma_NsonNode *pNode, const char *pPath);

BMA_DEF bma_bool_t bma_nsonGetBool_ext(bma_NsonNode *pNode, const char *pPath, bma_bool_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess);
#define bma_nsonGetBool(pNode, pPath) bma_nsonGetBool_ext((pNode), (pPath), BMA_FALSE, NULL)

BMA_DEF bma_intmax_t bma_nsonGetInt_ext(bma_NsonNode *pNode, const char *pPath, bma_intmax_t minValue, bma_intmax_t maxValue, bma_intmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess);
#define bma_nsonGetShort(pNode, pPath) ((short)bma_nsonGetInt_ext((pNode), (pPath), SHRT_MIN, SHRT_MAX, 0, NULL))
#define bma_nsonGetInt(pNode, pPath) ((int)bma_nsonGetInt_ext((pNode), (pPath), INT_MIN, INT_MAX, 0, NULL))
#define bma_nsonGetLong(pNode, pPath) ((long)bma_nsonGetInt_ext((pNode), (pPath), LONG_MIN, LONG_MAX, 0, NULL))
#ifdef LLONG_MAX
	#define bma_nsonGetLongLong(pNode, pPath) ((long long)bma_nsonGetInt_ext((pNode), (pPath), LLONG_MIN, LLONG_MAX, 0, NULL))
#endif

BMA_DEF bma_uintmax_t bma_nsonGetUint_ext(bma_NsonNode *pNode, const char *pPath, bma_uintmax_t minValue, bma_uintmax_t maxValue, bma_uintmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess);
#define bma_nsonGetUshort(pNode, pPath) ((unsigned short)bma_nsonGetUint_ext((pNode), (pPath), 0, USHRT_MAX, 0, NULL))
#define bma_nsonGetUint(pNode, pPath) ((unsigned int)bma_nsonGetUint_ext((pNode), (pPath), 0, UINT_MAX, 0, NULL))
#define bma_nsonGetUlong(pNode, pPath) ((long)bma_nsonGetUint_ext((pNode), (pPath), 0, ULONG_MAX, 0, NULL))
#ifdef ULLONG_MAX
	#define bma_nsonGetUlongLong(pNode, pPath) ((unsigned long long)bma_nsonGetUint_ext((pNode), (pPath), 0, ULLONG_MAX, 0, NULL))
#endif

BMA_DEF double bma_nsonGetDouble_ext(bma_NsonNode *pNode, const char *pPath, double minValue, double maxValue, double defaultWhenNotFound, bma_bool_t *pOptOutSuccess);
#define bma_nsonGetFloat(pNode, pPath) ((float)bma_nsonGetDouble_ext((pNode), (pPath), -FLT_MAX, FLT_MAX, 0.0, NULL))
#define bma_nsonGetDouble(pNode, pPath) bma_nsonGetDouble_ext((pNode), (pPath), -DBL_MAX, DBL_MAX, 0.0, NULL)

BMA_DEF bma_bool_t bma_nsonGetString_ext(bma_StrBldr *pDst, bma_NsonNode *pNode, const char *pPath, const char *pDefaultWhenNotFound);
#define bma_nsonGetString(pNode, pPath) bma_nsonGetString_ext((*pDst), (pNode), (pPath), "")

BMA_DEF void bma_nsonToStr_ext(bma_StrBldr *pDst, bma_NsonNode *pNode, bma_NsonDialect dialect, bma_bool_t pretty, const char *pIndent);
#define bma_nsonToStr(pDst, pNode) bma_nsonToStr_ext((pDst), (pNode), BMA_NSON_DIALECT_JSON, BMA_TRUE, "\t")

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_NSON_IMPL

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#ifdef __cplusplus
	extern "C" {
#endif

#define BMA_NSON_NUM_PARSE_BUF_SZ 64u

#define bma_nsonSetTrue(pBool) \
	do { \
		if ((pBool) != NULL) *pBool = BMA_TRUE; \
	} while(0)

#define bma_nsonSetFalse(pBool) \
	do { \
		if ((pBool) != NULL) *pBool = BMA_FALSE; \
	} while(0)

BMA_DEF void bma_nsonNodePtrDtor(void *ppNode, bma_IMemAlloc *pAlloc) {
	(void)pAlloc; /* a node has its own one */
	if (ppNode != NULL) {
		bma_NsonNode* p = *(bma_NsonNode**)ppNode;
		bma_nsonDelete(p);
	}
}

BMA_DEF bma_NsonNode *bma_nsonNew_ext(bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p;
	bma_assert(pAlloc != NULL);
	p = bma_malloc_ext(pAlloc, bma_NsonNode);
	memset(p, 0, sizeof(*p));
	p->pAlloc = pAlloc;
	return p;
}

BMA_DEF bma_NsonNode *bma_nsonNewNull_ext(bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p = bma_nsonNew_ext(pAlloc);
	p->type = BMA_NSON_NODE_TYPE_NULL;
	return p;
}

BMA_DEF bma_NsonNode *bma_nsonNewBool_ext(bma_bool_t value, bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p = bma_nsonNew_ext(pAlloc);
	p->type = BMA_NSON_NODE_TYPE_BOOL;
	p->value.b = value;
	return p;
}

BMA_DEF bma_NsonNode *bma_nsonNewSigned_ext(bma_intmax_t value, bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p = bma_nsonNew_ext(pAlloc);
	p->type = BMA_NSON_NODE_TYPE_SGND;
	p->value.si = value;
	return p;
}

BMA_DEF bma_NsonNode *bma_nsonNewUnsigned_ext(bma_uintmax_t value, bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p = bma_nsonNew_ext(pAlloc);
	p->type = BMA_NSON_NODE_TYPE_UNSGND;
	p->value.ui = value;
	return p;
}

BMA_DEF bma_NsonNode *bma_nsonNewDouble_ext(double value, bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p = bma_nsonNew_ext(pAlloc);
	p->type = BMA_NSON_NODE_TYPE_DBL;
	p->value.d = value;
	return p;
}

BMA_DEF bma_NsonNode *bma_nsonNewStr_ext(const char *pValue, bma_bool_t xferOwnership, bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p = bma_nsonNew_ext(pAlloc);
	p->type = BMA_NSON_NODE_TYPE_STR;
	if (xferOwnership) {
		p->value.p = (char*)pValue;
	}
	else {
		p->value.p = bma_strdup_ext(pValue, pAlloc);
	}
	return p;
}

BMA_DEF bma_NsonNode *bma_nsonNewStrWithLen_ext(const char *pValue, size_t valueLen, bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p = bma_nsonNew_ext(pAlloc);
	p->type = BMA_NSON_NODE_TYPE_STR;
	p->value.p = bma_strndup_ext(pValue, valueLen, pAlloc);
	return p;
}

BMA_DEF bma_NsonNode *bma_nsonNewArr_ext(bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p = bma_nsonNew_ext(pAlloc);
	p->type = BMA_NSON_NODE_TYPE_ARR;
	bma_NsonNodePtrVec_ctor_ext(&p->value.v, pAlloc);
	return p;
}

BMA_DEF void bma_nsonArrAppnd(bma_NsonNode *pNode, bma_NsonNode *pArrayElement) {
	bma_assert(pNode != NULL);
	bma_assert(pArrayElement != NULL);
	bma_assert(pNode->type == BMA_NSON_NODE_TYPE_ARR);
	bma_NsonNodePtrVec_appnd(&pNode->value.v, &pArrayElement);
}

BMA_DEF void bma_nsonObjAppnd(bma_NsonNode* pNode, bma_NsonNode *pKey, bma_NsonNode *pValue) {
	bma_assert(pNode != NULL);
	bma_assert(pKey != NULL);
	bma_assert(pValue != NULL);
	bma_assert(pNode->type == BMA_NSON_NODE_TYPE_OBJ);
	if (pKey->pObjectValue != NULL) {
		bma_nsonDelete(pKey->pObjectValue);
	}
	pKey->pObjectValue = pValue;
	bma_NsonNodePtrVec_appnd(&pNode->value.v, &pKey);
}

BMA_DEF bma_NsonNode *bma_nsonNewObj_ext(bma_IMemAlloc *pAlloc) {
	bma_NsonNode *p = bma_nsonNew_ext(pAlloc);
	p->type = BMA_NSON_NODE_TYPE_OBJ;
	bma_NsonNodePtrVec_ctor_ext(&p->value.v, pAlloc);
	return p;
}

BMA_DEF void bma_nsonDelete(bma_NsonNode *pNode) {
	if (pNode == NULL) {
		return;
	}
	bma_assert(pNode->pAlloc != NULL);
	if (pNode->pObjectValue != NULL) {
		bma_nsonDelete(pNode->pObjectValue);
	}
	switch(pNode->type) {
		case BMA_NSON_NODE_TYPE_STR: {
			bma_free_ext(pNode->pAlloc, pNode->value.p);
			break;
		}
		case BMA_NSON_NODE_TYPE_ARR: /* fall through */
		case BMA_NSON_NODE_TYPE_OBJ: {
			bma_NsonNodePtrVec_dtor(&pNode->value.v, NULL);
			break;
		}
		default: break;
	}
	bma_free_ext(pNode->pAlloc, pNode);
}

BMA_DEF void bma_nsonSkip(bma_NsonParseSrc *pStrWithLen) {
	while(pStrWithLen->len > 0) {
		char c = pStrWithLen->pStr[0];
		char c2 = '\0';
		if (pStrWithLen->len > 1u) {
			c2 = pStrWithLen->pStr[1];
		}
		if ((c == ':') || (c == ',') || isspace(c)) {
			++pStrWithLen->pStr;
			--pStrWithLen->len;
		}
		else if ((c == '/' ) && (c2 == '*')) {
			const char *pEnd;
			size_t numSkip;
			pEnd = (const char*)bma_memmem(pStrWithLen->pStr + 2,
			                               pStrWithLen->len - 2u,
			                               "*/",
			                               2u);
			if (pEnd == NULL) return;
			pEnd += 2;
			numSkip = pEnd - pStrWithLen->pStr;
			pStrWithLen->pStr += numSkip;
			pStrWithLen->len -= numSkip;
		}
		else if (((c == '/' ) && (c2 == '/')) || (c == '#')) {
			const char* pNl;
			size_t numSkip;
			pNl = (const char*)memchr(pStrWithLen->pStr+2, '\n', pStrWithLen->len - 2u);
			if (pNl == NULL) {
				pNl = (const char*)memchr(pStrWithLen->pStr+2,
				                          '\r',
				                          pStrWithLen->len - 2u);
			}
			if (pNl == NULL) {
				return;
			}
			numSkip = pNl - pStrWithLen->pStr + 1;
			pStrWithLen->pStr += numSkip;
			pStrWithLen->len -= numSkip;
		}
		else {
			break;
		}
	}
}

BMA_DEF void bma_nsonFmtErr(bma_StrBldr *pErrFormatter, const char* pMsg, bma_NsonParseSrc *pStrWithLen) {
	if (pErrFormatter != NULL) {
		bma_StrBldr_appndStr(pErrFormatter, pMsg);
		bma_StrBldr_appndStr(pErrFormatter, " (offset ");
		bma_StrBldr_appndUnsgnd(pErrFormatter, (bma_uintmax_t)(pStrWithLen->pStr - pStrWithLen->pBegin));
		bma_StrBldr_appndStr(pErrFormatter, ")");
	}
}

BMA_DEF bma_NsonNode *bma_nsonParseArr(bma_NsonParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc) {
	bma_NsonNode *pArray = bma_nsonNewArr_ext(pAlloc);
	/* skip the '[' */
	++pStrWithLen->pStr;
	--pStrWithLen->len;
	for(;;) {
		bma_NsonNode *pArrayElement;
		bma_nsonSkip(pStrWithLen);
		if (pStrWithLen->len == 0) {
			bma_nsonDelete(pArray);
			bma_nsonFmtErr(pErrFormatter, "Error parsing array: end of data reached", pStrWithLen);
			return NULL;
		}
		char c = *pStrWithLen->pStr;
		if (c == ']') { /* end of array */
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			return pArray;
		}
		pArrayElement = bma_nsonParseExt(pStrWithLen, pErrFormatter, pAlloc);
		if (pArrayElement == NULL) {
			bma_nsonDelete(pArray);
			return NULL;
		}
		bma_nsonArrAppnd(pArray, pArrayElement);
	}
	return NULL; /* unreachable */
}

BMA_DEF bma_NsonNode *bma_nsonParseObj(bma_NsonParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc) {
	bma_NsonNode *pObject = bma_nsonNewObj_ext(pAlloc);
	/* skip the '[' */
	++pStrWithLen->pStr;
	--pStrWithLen->len;
	for(;;) {
		bma_NsonNode *pKeyElement;
		bma_NsonNode *pValueElement;
		bma_nsonSkip(pStrWithLen);
		if (pStrWithLen->len == 0) {
			bma_nsonDelete(pObject);
			bma_nsonFmtErr(pErrFormatter, "Error parsing object: end of data reached before key", pStrWithLen);
			return NULL;
		}
		char c = *pStrWithLen->pStr;
		if (c == '}') { /* end of object */
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			return pObject;
		}
		pKeyElement = bma_nsonParseExt(pStrWithLen, pErrFormatter, pAlloc);
		if (pKeyElement == NULL) {
			bma_nsonDelete(pObject);
			return NULL;
		}
		pValueElement = bma_nsonParseExt(pStrWithLen, pErrFormatter, pAlloc);
		if (pValueElement == NULL) {
			bma_nsonDelete(pKeyElement);
			bma_nsonDelete(pObject);
			return NULL;
		}
		bma_nsonObjAppnd(pObject, pKeyElement, pValueElement);
	}
	return NULL; /* unreachable */
}

BMA_DEF bma_NsonNode *bma_nsonParseStr(bma_NsonParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc) {
	/* skip the '"' */
	bma_NsonNode *pResult = NULL;
	bma_StrBldr b;
	/* skip the '"' */
	++pStrWithLen->pStr;
	--pStrWithLen->len;
	bma_StrBldr_ctor_ext(&b, pAlloc);
	while (pStrWithLen->len > 0) {
		char c = pStrWithLen->pStr[0];
		switch(c) {
		case '"': {
			char *pStr = bma_StrBldr_rlse(&b);
			pResult = bma_nsonNewStr_ext(pStr, BMA_TRUE, pAlloc);
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			goto out;
		}
		case '\\': {
			char c2; 
			if (pStrWithLen->len < 2u) {
				bma_nsonFmtErr(pErrFormatter, "Error parsing string: escape sequence too short", pStrWithLen);
				goto out;
			}
			c2 = pStrWithLen->pStr[1];
			switch(c2) {
			case '"':
			case '\\':
			case '/': {
				bma_StrBldr_appndChr(&b, c2, 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'b': {
				bma_StrBldr_appndChr(&b, '\b', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'f': {
				bma_StrBldr_appndChr(&b, '\f', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'n': {
				bma_StrBldr_appndChr(&b, '\n', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'r': {
				bma_StrBldr_appndChr(&b, '\r', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 't': {
				bma_StrBldr_appndChr(&b, '\t', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'u': {
				char buf[5];
				unsigned long hex;
				if (pStrWithLen->len < 6u) {
					bma_nsonFmtErr(pErrFormatter, "Error parsing string: unicode escape sequence too short", pStrWithLen);
					goto out;
				}
				buf[0] = pStrWithLen->pStr[2];
				buf[1] = pStrWithLen->pStr[3];
				buf[2] = pStrWithLen->pStr[4];
				buf[3] = pStrWithLen->pStr[5];
				buf[4] = '\0';
				errno = 0;
				hex = strtoul(buf, NULL, 16);
				if ((hex == 0) && (errno == ERANGE)) {
					bma_nsonFmtErr(pErrFormatter, "Error parsing string: unicode escape sequence invalid", pStrWithLen);
					goto out;
				}
				bma_StrBldr_appndCdPntUtf8(&b, hex);
				pStrWithLen->pStr += 6;
				pStrWithLen->len -= 6u;
				break;
			}
			default: {
				bma_nsonFmtErr(pErrFormatter, "Error parsing string: wrong escape sequence", pStrWithLen);
				goto out;
			}
			}
			break;
		}
		default: {
			bma_StrBldr_appndChr(&b, c, 1u);
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			break;
		}
		}
	}
out:
	bma_StrBldr_dtor(&b, NULL);
	return pResult;
}

BMA_DEF bma_NsonNode *bma_nsonParseTok(bma_NsonParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc) {
	const char *p = pStrWithLen->pStr;
	size_t len = 0;
	size_t i;
	bma_bool_t containsDot = BMA_FALSE;
	while(pStrWithLen->len > 0) {
		char c = *pStrWithLen->pStr;
		if ((c == ':') || (c == ',') || (c == '[') || (c == ']') ||
		    (c == '{') || (c == '}') || isspace(c)) break;
		if (c == '.') containsDot = BMA_TRUE;
		++len;
		++pStrWithLen->pStr;
		--pStrWithLen->len;
	}
	if (len == 0) {
		bma_nsonFmtErr(pErrFormatter, "Error parsing token: length is zero", pStrWithLen);
		return NULL;
	}
	if (((p[0] >= '0') && (p[0] <= '9')) || (p[0] == '.') || (p[0] == '+') || (p[0] == '-')) {
		/* expect token to be an integer of floating point number */
		char parseBuf[BMA_NSON_NUM_PARSE_BUF_SZ];
		char *pParseBuf; /* A null terminated array to have reliable stdlib number parsing */
		if (len < sizeof(parseBuf)) { /* fits, inclusive '\0' terminator */
			pParseBuf = parseBuf; /* avoid dyn. mem alloc in most cases */
		}
		else {
			pParseBuf = (char*)(*pAlloc->pRllc)(pAlloc, NULL, len+1, NULL);
		}
		#define NSON_FREE_PARSE_BUF() \
			do { \
				if (pParseBuf != parseBuf) { \
					bma_free_ext(pAlloc, pParseBuf); \
				} \
			} while(0)
		memcpy(pParseBuf, p, len);	
		pParseBuf[len] = '\0';
		errno = 0;
		if (containsDot) { /* parse as double */
			double value;
			value = strtod(pParseBuf, NULL);
			if ((value == 0.0) && (errno == ERANGE)) {
				NSON_FREE_PARSE_BUF();
				bma_nsonFmtErr(pErrFormatter, "Error parsing double: value out of range", pStrWithLen);
				return NULL;
			}
			if (value == HUGE_VAL) {
				NSON_FREE_PARSE_BUF();
				bma_nsonFmtErr(pErrFormatter, "Error parsing double: value out of range", pStrWithLen);
				return NULL;
			}
			NSON_FREE_PARSE_BUF();
			return bma_nsonNewDouble_ext(value, pAlloc);
		}
		if (p[0] == '-') {
			bma_intmax_t value;
			#ifdef LLONG_MAX
				value = (bma_intmax_t)strtoll(pParseBuf, NULL, 10);
				if (((value == LLONG_MAX) || (value == LLONG_MIN) || (value == 0)) && (errno == ERANGE)) {
					NSON_FREE_PARSE_BUF();
					bma_nsonFmtErr(pErrFormatter, "Error parsing signed integer: value out of range", pStrWithLen);
					return NULL;
				}
			#else
				value = (bma_intmax_t)strtol(pParseBuf, NULL, 10);
				if (((value == LONG_MAX) || (value == LONG_MIN) || (value == 0)) && (errno == ERANGE)) {
					NSON_FREE_PARSE_BUF();
					bma_nsonFmtErr(pErrFormatter, "Error parsing signed integer: value out of range", pStrWithLen);
					return NULL;
				}
			#endif
			NSON_FREE_PARSE_BUF();
			return bma_nsonNewSigned_ext(value, pAlloc);
		}
		bma_uintmax_t value;
		#ifdef ULLONG_MAX
			value = (bma_uintmax_t)strtoull(pParseBuf, NULL, 10);
			if (((value == ULLONG_MAX) || (value == 0)) && (errno == ERANGE)) {
				NSON_FREE_PARSE_BUF();
				bma_nsonFmtErr(pErrFormatter, "Error parsing unsigned integer: value out of range", pStrWithLen);
				return NULL;
			}
		#else
			value = (bma_uintmax_t)strtoul(pParseBuf, NULL, 10);
			if (((value == ULONG_MAX) || (value == 0)) && (errno == ERANGE)) {
				NSON_FREE_PARSE_BUF();
				bma_nsonFmtErr(pErrFormatter, "Error parsing unsigned integer: value out of range", pStrWithLen);
				return NULL;
			}
		#endif
		NSON_FREE_PARSE_BUF();
		return bma_nsonNewUnsigned_ext(value, pAlloc);
		#undef NSON_FREE_PARSE_BUF
	}
	if ((len == 4u) && (memcmp(p, "null", 4u) == 0)) {
		return bma_nsonNewNull_ext(pAlloc);
	}
	if ((len == 4u) && (memcmp(p, "true", 4u) == 0)) {
		return bma_nsonNewBool_ext(BMA_TRUE, pAlloc);
	}
	if ((len == 5u) && (memcmp(p, "false", 5u) == 0)) {
		return bma_nsonNewBool_ext(BMA_FALSE, pAlloc);
	}
	if (((p[0] >= 'a') && (p[0] <= 'z')) || ((p[0] >= 'A') && (p[0] <= 'Z')) || (p[0] == '_'))
	{
		for (i=1u; i<len; ++i) {
			if (((p[i] >= 'a') && (p[i] <= 'z')) || ((p[i] >= 'A') && (p[i] <= 'Z')) ||
                            ((p[i] >= '0') && (p[i] <= '9')) || (p[i] == '_')) {
				/* good */
			}
			else {
				bma_nsonFmtErr(pErrFormatter, "Error parsing token: invalid", pStrWithLen);
				return NULL;
			}
		}
		return bma_nsonNewStrWithLen_ext(p, len, pAlloc);
	}
	bma_nsonFmtErr(pErrFormatter, "Error parsing token: invalid", pStrWithLen);
	return NULL;
}

BMA_DEF bma_NsonNode *bma_nsonParseExt(bma_NsonParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc) {
	char c;
	bma_assert(pStrWithLen != NULL);
	bma_assert(pAlloc != NULL);
	bma_nsonSkip(pStrWithLen);
	if (pStrWithLen->len == 0) return NULL;
	c = *pStrWithLen->pStr;
	switch(c) {
		case '[': return bma_nsonParseArr(pStrWithLen, pErrFormatter, pAlloc);
		case '{': return bma_nsonParseObj(pStrWithLen, pErrFormatter, pAlloc);
		case '"': return bma_nsonParseStr(pStrWithLen, pErrFormatter, pAlloc);
		default: return bma_nsonParseTok(pStrWithLen, pErrFormatter, pAlloc);
	}
}

BMA_DEF bma_NsonNode *bma_nsonParseMem(const void *pMem, size_t len) {
	bma_NsonParseSrc strWithLen;
	strWithLen.pBegin = (const char*)pMem;
	strWithLen.pStr = (const char*)pMem;
	strWithLen.len = len;
	return bma_nsonParseExt(&strWithLen, NULL, bma_getDfltMemAlloc());
}

BMA_DEF bma_NsonNode *bma_nsonParse(const char *pStr) {
	bma_NsonParseSrc strWithLen;
	strWithLen.pBegin = pStr;
	strWithLen.pStr = pStr;
	strWithLen.len = strlen(pStr);
	return bma_nsonParseExt(&strWithLen, NULL, bma_getDfltMemAlloc());
}

BMA_DEF bma_NsonNode *bma_nsonGet(bma_NsonNode *pNode, const char *pPath) {
	size_t pathLen;
	bma_assert(pNode != NULL);
	bma_assert(pPath != NULL);
	pathLen = strlen(pPath);
	if (pathLen == 0) return pNode;
	while (pathLen > 0) {
		if (*pPath == '[') { /* array indexing */
			char tmp[BMA_UINTMAX_MAX_DEC_LEN+1]; /* for strtoul(l) to have null term. buf */
			size_t num;
			bma_uintmax_t idx;
			const char* pEnd = strchr(pPath+1, ']');
			if (pEnd == 0) return NULL;
			num = (size_t)(pEnd - (pPath+1));
			if (num > BMA_UINTMAX_MAX_DEC_LEN) return NULL;
			memcpy(tmp, pPath+1, num);
			tmp[num] = '\0';
			errno = 0;
#ifdef ULLONG_MAX
			idx = strtoull(tmp, NULL, 10);
#else
			idx = strtoul(tmp, NULL, 10);
#endif
			if (errno != 0) return NULL;
			if (idx > ((size_t)-1)) return NULL;
			if (pNode->type != BMA_NSON_NODE_TYPE_ARR) return NULL;
			if (idx > bma_NsonNodePtrVec_getSz(&pNode->value.v)) return NULL;
			pNode = *bma_NsonNodePtrVec_at(&pNode->value.v, (size_t)idx);
			num = (size_t)((pEnd+1) - pPath);
			pPath += num;
			pathLen -= num;
		}
		else { /* find object element by key string */
			size_t num;
			size_t i;
			bma_NsonNode *pSubNode;
			bma_bool_t found = BMA_FALSE;
			const char* pEnd = strchr(pPath, '/');
			if (pEnd == NULL) pEnd = pPath + pathLen;
			num = (size_t)(pEnd - pPath);
			if (num == 0) return NULL;
			if (pNode->type != BMA_NSON_NODE_TYPE_OBJ) return NULL;
			for (i=0; i<bma_NsonNodePtrVec_getSz(&pNode->value.v); ++i) {
				pSubNode = *bma_NsonNodePtrVec_at(&pNode->value.v, i);
				if (pSubNode->type != BMA_NSON_NODE_TYPE_STR) return NULL;
				bma_assert(pSubNode->value.p != NULL);
				if (strlen(pSubNode->value.p) != num) continue;
				if (memcmp(pSubNode->value.p, pPath, num) == 0) {
					bma_assert(pSubNode->pObjectValue != NULL);
					pNode = pSubNode->pObjectValue;
					found = BMA_TRUE;
					num = (pEnd - pPath);
					pPath += num;
					pathLen -= num;
					break;
				}
			}
			if (!found) return NULL;
		}
		if ((pathLen > 0) && (*pPath == '/')) {
				++pPath;
				--pathLen;
		}
	}
	return pNode;
}

BMA_DEF bma_bool_t bma_nsonGetBool_ext(bma_NsonNode *pNode, const char *pPath, bma_bool_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess) {
	bma_NsonNode *pSubNode;
	bma_assert(pNode != NULL);
	bma_nsonSetFalse(pOptOutSuccess);
	pSubNode = bma_nsonGet(pNode, pPath);
	if (pSubNode == NULL) {
		return defaultWhenNotFound;
	}
	if (pSubNode->type != BMA_NSON_NODE_TYPE_BOOL) {
		return defaultWhenNotFound;
	}
	bma_nsonSetTrue(pOptOutSuccess);
	return pSubNode->value.b;
}

BMA_DEF bma_intmax_t bma_nsonGetInt_ext(bma_NsonNode *pNode, const char *pPath, bma_intmax_t minValue, bma_intmax_t maxValue, bma_intmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess) {
	bma_NsonNode *pSubNode;
	bma_intmax_t result;
	bma_assert(pNode != NULL);
	bma_nsonSetFalse(pOptOutSuccess);
	pSubNode = bma_nsonGet(pNode, pPath);
	if (pSubNode == NULL) return defaultWhenNotFound;
	switch(pSubNode->type) {
		case BMA_NSON_NODE_TYPE_BOOL: result = (pNode->value.b) ? 1 : 0; break;
		case BMA_NSON_NODE_TYPE_SGND: result = pNode->value.si; break;
		case BMA_NSON_NODE_TYPE_UNSGND: {
			if (pNode->value.ui > (bma_uintmax_t)BMA_INTMAX_MAX) return defaultWhenNotFound;
			result = (bma_intmax_t)pNode->value.ui;
			break;
		}
		case BMA_NSON_NODE_TYPE_DBL: {
			result = (bma_intmax_t)pNode->value.d;
			if (((double)result) != pNode->value.d) return defaultWhenNotFound;
			break;
		}
		default: return defaultWhenNotFound;
	}
	if ((result < minValue) || (result > maxValue)) return defaultWhenNotFound;
	bma_nsonSetTrue(pOptOutSuccess);
	return result;
}

BMA_DEF bma_uintmax_t bma_nsonGetUint_ext(bma_NsonNode *pNode, const char *pPath, bma_uintmax_t minValue, bma_uintmax_t maxValue, bma_uintmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess) {
	bma_NsonNode *pSubNode;
	bma_uintmax_t result;
	bma_assert(pNode != NULL);
	bma_nsonSetFalse(pOptOutSuccess);
	pSubNode = bma_nsonGet(pNode, pPath);
	if (pSubNode == NULL) return defaultWhenNotFound;
	switch(pSubNode->type) {
		case BMA_NSON_NODE_TYPE_BOOL: result = (pNode->value.b) ? 1u : 0u; break;
		case BMA_NSON_NODE_TYPE_SGND: {
			if (pNode->value.si < 0) return defaultWhenNotFound;
			result = (bma_uintmax_t)pNode->value.si;
			break;
		}
		case BMA_NSON_NODE_TYPE_UNSGND: {
			result = pNode->value.ui;
			break;
		}
		case BMA_NSON_NODE_TYPE_DBL: {
			result = (bma_uintmax_t)pNode->value.d;
			if (((double)result) != pNode->value.d) return defaultWhenNotFound;
			break;
		}
		default: return defaultWhenNotFound;
	}
	if ((result < minValue) || (result > maxValue)) return defaultWhenNotFound;
	bma_nsonSetTrue(pOptOutSuccess);
	return result;
}

BMA_DEF double bma_nsonGetDouble_ext(bma_NsonNode *pNode, const char *pPath, double minValue, double maxValue, double defaultWhenNotFound, bma_bool_t *pOptOutSuccess) {
	bma_NsonNode *pSubNode;
	double result;
	bma_assert(pNode != NULL);
	bma_nsonSetFalse(pOptOutSuccess);
	pSubNode = bma_nsonGet(pNode, pPath);
	if (pSubNode == NULL) return defaultWhenNotFound;
	switch(pSubNode->type) {
		case BMA_NSON_NODE_TYPE_BOOL: result = (pNode->value.b) ? 1.0 : 0.0; break;
		case BMA_NSON_NODE_TYPE_SGND: {
			result = (double)pNode->value.si;
			if ((bma_intmax_t)result != pNode->value.si) return defaultWhenNotFound;
			break;
		}
		case BMA_NSON_NODE_TYPE_UNSGND: {
			result = (double)pNode->value.ui;
			if ((bma_uintmax_t)result != pNode->value.ui) return defaultWhenNotFound;
			break;
		}
		case BMA_NSON_NODE_TYPE_DBL: {
			result = pNode->value.d;
			break;
		}
		default: return defaultWhenNotFound;
	}
	if ((result < minValue) || (result > maxValue)) return defaultWhenNotFound;
	bma_nsonSetTrue(pOptOutSuccess);
	return result;
}

BMA_DEF bma_bool_t bma_nsonGetString_ext(bma_StrBldr *pDst, bma_NsonNode *pNode, const char *pPath, const char *pDefaultWhenNotFound) {
	bma_NsonNode *pSubNode;
	bma_assert(pDst != NULL);
	bma_assert(pNode != NULL);
	pSubNode = bma_nsonGet(pNode, pPath);
	if (pSubNode == NULL) {
		bma_StrBldr_appndStr(pDst, pDefaultWhenNotFound);
		return BMA_FALSE;
	}
	switch(pSubNode->type) {
		case BMA_NSON_NODE_TYPE_BOOL:   bma_StrBldr_appndBool(pDst, pNode->value.b);    break;
		case BMA_NSON_NODE_TYPE_SGND:   bma_StrBldr_appndSgnd(pDst, pNode->value.si);   break;
		case BMA_NSON_NODE_TYPE_UNSGND: bma_StrBldr_appndUnsgnd(pDst, pNode->value.ui); break;
		case BMA_NSON_NODE_TYPE_DBL:    bma_StrBldr_appndDbl(pDst, pNode->value.d);     break;
		case BMA_NSON_NODE_TYPE_STR:    bma_StrBldr_appndStr(pDst, pNode->value.p);     break;
		default: {
			bma_StrBldr_appndStr(pDst, pDefaultWhenNotFound);
			return BMA_FALSE;
		}
	}
	return BMA_TRUE;
}

BMA_DEF void bma_nsonPutIndent(bma_StrBldr *pDst, const char *pIndent, size_t indentSize, size_t indentLevel) {
	size_t i;
	bma_assert(pDst != NULL);
	bma_assert(pIndent != NULL);
	for (i=0; i<indentLevel; ++i) {
		bma_StrBldr_appndStrN(pDst, pIndent, indentSize);
	}
}

/*! Checks if the string could be printed without escapes or double quotes in NSON */
BMA_DEF bma_bool_t bma_nsonIsSmplTok(const char *pStr, size_t len) {
	if (len == 0) return BMA_FALSE;
	if ((len == 4u) && (memcmp(pStr, "null",  4u) == 0)) return BMA_FALSE;
	if ((len == 4u) && (memcmp(pStr, "true",  4u) == 0)) return BMA_FALSE;
	if ((len == 5u) && (memcmp(pStr, "false", 5u) == 0)) return BMA_FALSE;
	if (((*pStr >= 'a') && (*pStr <= 'z')) ||
	    ((*pStr >= 'A') && (*pStr <= 'Z')) ||
            (*pStr == '_')) {
		/* good: initial char must not be a number */
	}
	else {
		return BMA_FALSE;
	}
	++pStr;
	--len;
	while(len > 0) {
		if (((*pStr >= 'a') && (*pStr <= 'z')) ||
		    ((*pStr >= 'A') && (*pStr <= 'Z')) ||
		    ((*pStr >= '0') && (*pStr <= '9')) ||
		    (*pStr == '_')) {
			/* good: follow-chars can also be number */
		}
		else {
			return BMA_FALSE;
		}
		++pStr;
		--len;
	}
	return BMA_TRUE;
}

BMA_DEF void bma_jsonEscape(bma_StrBldr *pDst, const char *pStr, size_t len) {
	bma_StrBldr_appndChr(pDst, '"', 1u);
	while (len > 0) {
		switch (*pStr) {
		case '"':  bma_StrBldr_appndStrN(pDst, "\\\"", 2u); break;
		case '\\': bma_StrBldr_appndStrN(pDst, "\\\\", 2u); break;
		case '\b': bma_StrBldr_appndStrN(pDst, "\\b",  2u); break;
		case '\f': bma_StrBldr_appndStrN(pDst, "\\f",  2u); break;
		case '\n': bma_StrBldr_appndStrN(pDst, "\\n",  2u); break;
		case '\r': bma_StrBldr_appndStrN(pDst, "\\r",  2u); break;
		case '\t': bma_StrBldr_appndStrN(pDst, "\\t",  2u); break;
		default: {
			if (((unsigned char)*pStr <= 0x1F) || ((unsigned char)*pStr == 0x7F)) {
				/* control chars */
				char buf[7];
				size_t n;
#if BMA_MODERN_C
				n = (size_t)snprintf(buf, sizeof(buf), "\\u%04X", (int)((unsigned char)*pStr));
#else
				n = (size_t)sprintf(buf, "\\u%04X", (int)((unsigned char)*pStr));
#endif
				bma_assert(n == 6u);
				bma_StrBldr_appndStrN(pDst, buf, n);
			}
			else {
				bma_StrBldr_appndChr(pDst, *pStr, 1u);
			}
			break;
		}
		}
		++pStr;
		--len;
	}
	bma_StrBldr_appndChr(pDst, '"', 1u);
}

BMA_DEF void bma_nsonToStr_impl(bma_StrBldr *pDst, bma_NsonNode *pNode, bma_NsonDialect dialect, bma_bool_t pretty, bma_bool_t initialIndent, const char *pIndent, size_t indentSize, size_t indentLevel) {
	bma_assert(pDst != NULL);
	bma_assert(pNode != NULL);
	bma_assert(pIndent != NULL);
	if (pretty && initialIndent) {
		bma_nsonPutIndent(pDst, pIndent, indentSize, indentLevel);
	}
	switch(pNode->type) {
		case BMA_NSON_NODE_TYPE_NULL:   bma_StrBldr_appndStrN(pDst, "null", 4u); break;
		case BMA_NSON_NODE_TYPE_BOOL:   bma_StrBldr_appndBool(pDst, pNode->value.b);    break;
		case BMA_NSON_NODE_TYPE_SGND:   bma_StrBldr_appndSgnd(pDst, pNode->value.si);   break;
		case BMA_NSON_NODE_TYPE_UNSGND: bma_StrBldr_appndUnsgnd(pDst, pNode->value.ui); break;
		case BMA_NSON_NODE_TYPE_DBL:    bma_StrBldr_appndDbl(pDst, pNode->value.d);     break;
		case BMA_NSON_NODE_TYPE_STR: {
			const char *p = pNode->value.p;
			size_t len = strlen(pNode->value.p);
			if ((dialect == BMA_NSON_DIALECT_NSON) && bma_nsonIsSmplTok(p, len)) {
				bma_StrBldr_appndStrN(pDst, p, len);
			}
			else {
				bma_jsonEscape(pDst, p, len);
			}
			break;
		}
		case BMA_NSON_NODE_TYPE_ARR: {
			size_t i, size;
			bma_NsonNode *pSubNode;
			bma_StrBldr_appndChr(pDst, '[', 1u);
			if (pretty) {
				bma_StrBldr_appndChr(pDst, '\n', 1u);
			}
			++indentLevel;
			size = bma_NsonNodePtrVec_getSz(&pNode->value.v);
			for (i=0; i<size; ++i) {
				pSubNode = *bma_NsonNodePtrVec_at(&pNode->value.v, i);
				bma_nsonToStr_impl(pDst, pSubNode, dialect, pretty, BMA_TRUE, pIndent, indentSize, indentLevel);
				if (((dialect == BMA_NSON_DIALECT_NSON) && (size > 1)) || ((i+1) < size)) {
					bma_StrBldr_appndChr(pDst, ',', 1u);
				}
				if (pretty) {
					bma_StrBldr_appndChr(pDst, '\n', 1u);
				}
			}
			--indentLevel;
			if (pretty) {
				bma_nsonPutIndent(pDst, pIndent, indentSize, indentLevel);
			}
			bma_StrBldr_appndChr(pDst, ']', 1u);
			break;
		}
		case BMA_NSON_NODE_TYPE_OBJ: {
			size_t i, size;
			bma_NsonNode *pKeyNode, *pValueNode;
			bma_StrBldr_appndChr(pDst, '{', 1u);
			if (pretty) {
				bma_StrBldr_appndChr(pDst, '\n', 1u);
			}
			++indentLevel;
			size = bma_NsonNodePtrVec_getSz(&pNode->value.v);
			for (i=0; i<size; ++i) {
				pKeyNode = *bma_NsonNodePtrVec_at(&pNode->value.v, i);
				pValueNode = pKeyNode->pObjectValue;
				bma_nsonToStr_impl(pDst, pKeyNode, dialect, pretty, BMA_TRUE, pIndent, indentSize, indentLevel);
				bma_StrBldr_appndChr(pDst, ':', 1u);
				if (pretty) {
					bma_StrBldr_appndChr(pDst, ' ', 1u);
				}
				bma_nsonToStr_impl(pDst, pValueNode, dialect, pretty, BMA_FALSE, pIndent, indentSize, indentLevel);
				if (((dialect == BMA_NSON_DIALECT_NSON) && (size > 1)) || ((i+1) < size)) {
					bma_StrBldr_appndChr(pDst, ',', 1u);
				}
				if (pretty) {
					bma_StrBldr_appndChr(pDst, '\n', 1u);
				}
			}
			--indentLevel;
			if (pretty) {
				bma_nsonPutIndent(pDst, pIndent, indentSize, indentLevel);
			}
			bma_StrBldr_appndChr(pDst, '}', 1u);
			break;
		}
		default: break;
	}
}

BMA_DEF void bma_nsonToStr_ext(bma_StrBldr *pDst, bma_NsonNode *pNode, bma_NsonDialect dialect, bma_bool_t pretty, const char *pIndent) {
	bma_assert(pDst != NULL);
	bma_assert(pNode != NULL);
	bma_assert(pIndent != NULL);
	bma_nsonToStr_impl(pDst, pNode, dialect, pretty, BMA_TRUE, pIndent, strlen(pIndent), 0);
	if (pretty) {
		bma_StrBldr_appndChr(pDst, '\n', 1u);
	}
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_NSON_IMPLEMENTATION */

#ifdef BMA_NSON_TEST

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

void test_new(void) {
	bma_NsonNode *pNode;
	bma_NsonNode *pSubNode;
	pNode = bma_nsonNew();
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_INVLD);
	BMA_EXPECT(pNode->pObjectValue == NULL);
	BMA_EXPECT(pNode->pAlloc == bma_getDfltMemAlloc());
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewNull();
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_NULL);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewBool(BMA_FALSE);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_BOOL);
	BMA_EXPECT(!pNode->value.b);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewBool(BMA_TRUE);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_BOOL);
	BMA_EXPECT(pNode->value.b);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewSigned(BMA_INTMAX_MIN);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_SGND);
	BMA_EXPECT(pNode->value.si == BMA_INTMAX_MIN);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewSigned(BMA_INTMAX_MAX);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_SGND);
	BMA_EXPECT(pNode->value.si == BMA_INTMAX_MAX);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewUnsigned(0u);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_UNSGND);
	BMA_EXPECT(pNode->value.ui == 0u);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewUnsigned(BMA_UINTMAX_MAX);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_UNSGND);
	BMA_EXPECT(pNode->value.ui == BMA_UINTMAX_MAX);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewDouble(3.14);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_DBL);
	BMA_EXPECT(fabs(pNode->value.d - 3.14) < 0.001);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewStr("Hello");
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "Hello") == 0);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewStrWithLen("Hello", 3u);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "Hel") == 0);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewArr();
	bma_nsonArrAppnd(pNode, bma_nsonNewStr("Hello"));
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_ARR);
	bma_nsonDelete(pNode);
	pNode = bma_nsonNewObj();
	bma_nsonObjAppnd(pNode, bma_nsonNewStr("Key"), bma_nsonNewStr("Hello"));
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_OBJ);
	pSubNode = *bma_NsonNodePtrVec_at(&pNode->value.v, 0);
	BMA_EXPECT(strcmp(pSubNode->value.p, "Key") == 0);
	BMA_EXPECT(pSubNode->pObjectValue != NULL);
	BMA_EXPECT(strcmp(pSubNode->pObjectValue->value.p, "Hello") == 0);
	bma_nsonDelete(pNode);
}

void test_parse(void) {
	bma_NsonNode *pNode;
	pNode = bma_nsonParse("null");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_NULL);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("true");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_BOOL);
	BMA_EXPECT(pNode->value.b == BMA_TRUE);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("false");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_BOOL);
	BMA_EXPECT(pNode->value.b == BMA_FALSE);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("4711");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_UNSGND);
	BMA_EXPECT(pNode->value.ui == 4711);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("-4711");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_SGND);
	BMA_EXPECT(pNode->value.si == -4711);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("3.14");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_DBL);
	BMA_EXPECT(fabs(pNode->value.d - 3.14) <= 0.001);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("Hello");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "Hello") == 0);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("[]");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NsonNodePtrVec_getSz(&pNode->value.v) == 0);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("[1]");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NsonNodePtrVec_getSz(&pNode->value.v) == 1);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("[1 Hello]");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NsonNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("[1,Hello,]");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NsonNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("{A:1,B:2}");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_OBJ);
	BMA_EXPECT(bma_NsonNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("{A 1 B 2}");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_OBJ);
	BMA_EXPECT(bma_NsonNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse(" // comment \n[1 /* comment */ 2] // comment");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NsonNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("\"Hello\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "Hello") == 0);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("\"\\r\\n\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "\r\n") == 0);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("\"\\\"\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "\"") == 0);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("\"\\u0001\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "\x01") == 0);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("\"H\\u00E4llo\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "H\xC3\xA4llo") == 0);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse(
		"{\n"
		"	name: Anybody,\n"
		"	age: 42, // comment\n"
		"	hobbies: [reading, writing, swimming,]\n"
		"}\n"
	);
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NSON_NODE_TYPE_OBJ);
	bma_nsonDelete(pNode);
}

void test_get(void) {
	bma_NsonNode *pNode, *pSubNode;
	pNode = bma_nsonParse(
		"{\n"
		"	name: Anybody,\n"
		"	age: 42, // comment\n"
		"	hobbies: [reading, writing, swimming,]\n"
		"}\n"
	);
	BMA_EXPECT(pNode != NULL);
	pSubNode = bma_nsonGet(pNode, "name");
	BMA_EXPECT(pSubNode != NULL);
	BMA_EXPECT(pSubNode->type = BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pSubNode->value.p, "Anybody") == 0);
	pSubNode = bma_nsonGet(pNode, "age");
	BMA_EXPECT(pSubNode != NULL);
	BMA_EXPECT(pSubNode->type = BMA_NSON_NODE_TYPE_UNSGND);
	BMA_EXPECT(pSubNode->value.ui == 42u);
	pSubNode = bma_nsonGet(pNode, "hobbies/[2]");
	BMA_EXPECT(pSubNode != NULL);
	BMA_EXPECT(pSubNode->type = BMA_NSON_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pSubNode->value.p, "swimming") == 0);
	bma_nsonDelete(pNode);
}

void test_toString(void) {
	/*
	BMA_DEF void bma_nsonToStr_ext(bma_StrBldr *pDst, bma_NsonNode *pNode, bma_NsonDialect dialect, bma_bool_t pretty, const char *pIndent) {
	*/
	bma_StrBldr b;
	bma_NsonNode *pNode;
	bma_StrBldr_ctor(&b);
	pNode = bma_nsonParse("null");
	BMA_EXPECT(pNode != NULL);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_JSON, BMA_FALSE, "");
	bma_nsonDelete(pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "null") == 0);
	bma_StrBldr_clear(&b);
	pNode = bma_nsonParse("true");
	BMA_EXPECT(pNode != NULL);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_JSON, BMA_FALSE, "");
	bma_nsonDelete(pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "true") == 0);
	bma_StrBldr_clear(&b);
	pNode = bma_nsonParse("false");
	BMA_EXPECT(pNode != NULL);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_JSON, BMA_FALSE, "");
	bma_nsonDelete(pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "false") == 0);
	bma_StrBldr_clear(&b);
	pNode = bma_nsonParse("0");
	BMA_EXPECT(pNode != NULL);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_JSON, BMA_FALSE, "");
	bma_nsonDelete(pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "0") == 0);
	bma_StrBldr_clear(&b);
	pNode = bma_nsonParse("\"Hello\"");
	BMA_EXPECT(pNode != NULL);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_JSON, BMA_FALSE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "\"Hello\"") == 0);
	bma_StrBldr_clear(&b);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_NSON, BMA_TRUE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "Hello\n") == 0);
	bma_nsonDelete(pNode);
	bma_StrBldr_clear(&b);
	pNode = bma_nsonParse("[]");
	BMA_EXPECT(pNode != NULL);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_JSON, BMA_FALSE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[]") == 0);
	bma_StrBldr_clear(&b);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_JSON, BMA_TRUE, "\t");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[\n]\n") == 0);
	bma_StrBldr_clear(&b);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("[1]");
	BMA_EXPECT(pNode != NULL);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_JSON, BMA_FALSE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[1]") == 0);
	bma_StrBldr_clear(&b);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_JSON, BMA_TRUE, "\t");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[\n\t1\n]\n") == 0);
	bma_StrBldr_clear(&b);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("[1,2]");
	BMA_EXPECT(pNode != NULL);
	bma_nsonToStr(&b, pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[\n\t1,\n\t2\n]\n") == 0);
	bma_StrBldr_clear(&b);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_NSON, BMA_TRUE, "\t");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[\n\t1,\n\t2,\n]\n") == 0);
	bma_StrBldr_clear(&b);
	bma_nsonDelete(pNode);
	pNode = bma_nsonParse("{key:1,value:2}");
	BMA_EXPECT(pNode != NULL);
	bma_nsonToStr(&b, pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b),
		"{\n"
		"	\"key\": 1,\n"
		"	\"value\": 2\n"
		"}\n"
	) == 0);
	bma_StrBldr_clear(&b);
	bma_nsonToStr_ext(&b, pNode, BMA_NSON_DIALECT_NSON, BMA_TRUE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b),
		"{\n"
		"key: 1,\n"
		"value: 2,\n"
		"}\n"
	) == 0);
	bma_StrBldr_clear(&b);
	bma_nsonDelete(pNode);
	bma_StrBldr_dtor(&b, NULL);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_new);
	BMA_TEST(test_parse);
	BMA_TEST(test_get);
	BMA_TEST(test_toString);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_NSON_TEST */

#endif /* BMA_NSON_H_INCLUDED */
