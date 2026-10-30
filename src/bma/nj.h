#ifndef BMA_NJ_H_INCLUDED
#define BMA_NJ_H_INCLUDED

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

Bau 'ma's minimalistic NJ parser (NJ = Nice JSON).
Allows normal JSON but also some specialities to make the language
more pleasant:

- Commas and colons in lists and maps are optional, so [1 2 3] is a valid list as long
  as whitespace is used as delimiter
- Allows C-, C++ and python style comments
- Any token which isn't "true", "false" or "null" is considered a string, so "Hello" can
  be also written Hello. Such tokens aren't allowed to contain whitespace of course.

So, for instance, this is recognized as valid NJ:

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

typedef struct bma_NjParseSrc {
	const char *pBegin; /* for error reporting to calculate error offset */
	const char *pStr; /* incremented while parsing */
	size_t len; /* decremented while parsing */
} bma_NjParseSrc;

typedef unsigned int bma_NjNodeType;

#define BMA_NJ_NODE_TYPE_INVLD  0
#define BMA_NJ_NODE_TYPE_NULL   1
#define BMA_NJ_NODE_TYPE_BOOL   2
#define BMA_NJ_NODE_TYPE_SGND   3
#define BMA_NJ_NODE_TYPE_UNSGND 4
#define BMA_NJ_NODE_TYPE_DBL    5
#define BMA_NJ_NODE_TYPE_STR    6
#define BMA_NJ_NODE_TYPE_ARR    7
#define BMA_NJ_NODE_TYPE_OBJ    8
#define BMA_NJ_NODE_NUM_TYPES   9

typedef unsigned int bma_NjDialect;

#define BMA_NJ_DIALECT_JSON         0
#define BMA_NJ_DIALECT_NJ           1
#define BMA_NJ_DIALECT_NUM_DIALECTS 2

typedef struct bma_NjNode bma_NjNode;

BMA_DEF void bma_njNodePtrDtor(void *ppNode, bma_IMemAlloc *pAlloc);

BMA_DEF_VEC(bma_NjNodePtrVec, bma_NjNode*, &bma_njNodePtrDtor)

typedef union bma_NjNodeUnion {
	bma_bool_t       b;
	bma_intmax_t     si;
	bma_uintmax_t    ui;
	double           d;
	char             *p;
	bma_NjNodePtrVec v; /* for maps and arrays, this points to all values */
} bma_NjNodeUnion;

struct bma_NjNode {
	bma_NjNodeType  type;
	bma_NjNodeUnion value;
	bma_NjNode      *pObjectValue; /* ptr from key to value in objs */
	bma_IMemAlloc   *pAlloc;
};

BMA_DEF bma_NjNode *bma_njNew_ext(bma_IMemAlloc *pAlloc);
#define bma_njNew() bma_njNew_ext(bma_getDfltMemAlloc())

BMA_DEF bma_NjNode *bma_njNewNull_ext(bma_IMemAlloc *pAlloc);
#define bma_njNewNull() bma_njNewNull_ext(bma_getDfltMemAlloc())

BMA_DEF bma_NjNode *bma_njNewBool_ext(bma_bool_t value, bma_IMemAlloc *pAlloc);
#define bma_njNewBool(value) bma_njNewBool_ext((value), bma_getDfltMemAlloc())

BMA_DEF bma_NjNode *bma_njNewSigned_ext(bma_intmax_t value, bma_IMemAlloc *pAlloc);
#define bma_njNewSigned(value) bma_njNewSigned_ext((value), bma_getDfltMemAlloc())

BMA_DEF bma_NjNode *bma_njNewUnsigned_ext(bma_uintmax_t value, bma_IMemAlloc *pAlloc);
#define bma_njNewUnsigned(value) bma_njNewUnsigned_ext((value), bma_getDfltMemAlloc())

BMA_DEF bma_NjNode *bma_njNewDouble_ext(double value, bma_IMemAlloc *pAlloc);
#define bma_njNewDouble(value) bma_njNewDouble_ext((value), bma_getDfltMemAlloc())

BMA_DEF bma_NjNode *bma_njNewStr_ext(const char *pValue, bma_bool_t xferOwnership, bma_IMemAlloc *pAlloc);
#define bma_njNewStr(value) bma_njNewStr_ext((value), BMA_FALSE, bma_getDfltMemAlloc())

BMA_DEF bma_NjNode *bma_njNewStrWithLen_ext(const char *pValue, size_t valueLen, bma_IMemAlloc *pAlloc);
#define bma_njNewStrWithLen(value, len) bma_njNewStrWithLen_ext((value), (len), bma_getDfltMemAlloc())

BMA_DEF bma_NjNode *bma_njNewArr_ext(bma_IMemAlloc *pAlloc);
#define bma_njNewArr() bma_njNewArr_ext(bma_getDfltMemAlloc())
BMA_DEF void bma_njArrAppnd(bma_NjNode* pNode, bma_NjNode* pArrayElement);

BMA_DEF bma_NjNode *bma_njNewObj_ext(bma_IMemAlloc *pAlloc);
#define bma_njNewObj() bma_njNewObj_ext(bma_getDfltMemAlloc())
BMA_DEF void bma_njObjAppnd(bma_NjNode* pNode, bma_NjNode *pKey, bma_NjNode *pValue);

BMA_DEF void bma_njDelete(bma_NjNode *pNode);

BMA_DEF bma_NjNode *bma_njParseExt(bma_NjParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc);

BMA_DEF bma_NjNode *bma_njParseMem(const void *pMem, size_t len);
BMA_DEF bma_NjNode *bma_njParse(const char *pStr);

BMA_DEF bma_NjNode *bma_njGet(bma_NjNode *pNode, const char *pPath);

BMA_DEF bma_bool_t bma_njGetBool_ext(bma_NjNode *pNode, const char *pPath, bma_bool_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess);
#define bma_njGetBool(pNode, pPath) bma_njGetBool_ext((pNode), (pPath), BMA_FALSE, NULL)

BMA_DEF bma_intmax_t bma_njGetInt_ext(bma_NjNode *pNode, const char *pPath, bma_intmax_t minValue, bma_intmax_t maxValue, bma_intmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess);
#define bma_njGetShort(pNode, pPath) ((short)bma_njGetInt_ext((pNode), (pPath), SHRT_MIN, SHRT_MAX, 0, NULL))
#define bma_njGetInt(pNode, pPath) ((int)bma_njGetInt_ext((pNode), (pPath), INT_MIN, INT_MAX, 0, NULL))
#define bma_njGetLong(pNode, pPath) ((long)bma_njGetInt_ext((pNode), (pPath), LONG_MIN, LONG_MAX, 0, NULL))
#ifdef LLONG_MAX
	#define bma_njGetLongLong(pNode, pPath) ((long long)bma_njGetInt_ext((pNode), (pPath), LLONG_MIN, LLONG_MAX, 0, NULL))
#endif

BMA_DEF bma_uintmax_t bma_njGetUint_ext(bma_NjNode *pNode, const char *pPath, bma_uintmax_t minValue, bma_uintmax_t maxValue, bma_uintmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess);
#define bma_njGetUshort(pNode, pPath) ((unsigned short)bma_njGetUint_ext((pNode), (pPath), 0, USHRT_MAX, 0, NULL))
#define bma_njGetUint(pNode, pPath) ((unsigned int)bma_njGetUint_ext((pNode), (pPath), 0, UINT_MAX, 0, NULL))
#define bma_njGetUlong(pNode, pPath) ((unsigned long)bma_njGetUint_ext((pNode), (pPath), 0, ULONG_MAX, 0, NULL))
#ifdef ULLONG_MAX
	#define bma_njGetUlongLong(pNode, pPath) ((unsigned long long)bma_njGetUint_ext((pNode), (pPath), 0, ULLONG_MAX, 0, NULL))
#endif

BMA_DEF double bma_njGetDouble_ext(bma_NjNode *pNode, const char *pPath, double minValue, double maxValue, double defaultWhenNotFound, bma_bool_t *pOptOutSuccess);
#define bma_njGetFloat(pNode, pPath) ((float)bma_njGetDouble_ext((pNode), (pPath), -FLT_MAX, FLT_MAX, 0.0, NULL))
#define bma_njGetDouble(pNode, pPath) bma_njGetDouble_ext((pNode), (pPath), -DBL_MAX, DBL_MAX, 0.0, NULL)

BMA_DEF bma_bool_t bma_njGetString_ext(bma_StrBldr *pDst, bma_NjNode *pNode, const char *pPath, const char *pDefaultWhenNotFound);
#define bma_njGetString(pDst, pNode, pPath) bma_njGetString_ext((pDst), (pNode), (pPath), "")

BMA_DEF void bma_njToStr_ext(bma_StrBldr *pDst, bma_NjNode *pNode, bma_NjDialect dialect, bma_bool_t pretty, const char *pIndent);
#define bma_njToStr(pDst, pNode) bma_njToStr_ext((pDst), (pNode), BMA_NJ_DIALECT_JSON, BMA_TRUE, "\t")

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_NJ_IMPL

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#ifdef __cplusplus
	extern "C" {
#endif

#define BMA_NJ_NUM_PARSE_BUF_SZ 64u

#define bma_njSetTrue(pBool) \
	do { \
		if ((pBool) != NULL) *pBool = BMA_TRUE; \
	} while(0)

#define bma_njSetFalse(pBool) \
	do { \
		if ((pBool) != NULL) *pBool = BMA_FALSE; \
	} while(0)

BMA_DEF void bma_njNodePtrDtor(void *ppNode, bma_IMemAlloc *pAlloc) {
	(void)pAlloc; /* a node has its own one */
	if (ppNode != NULL) {
		bma_NjNode* p = *(bma_NjNode**)ppNode;
		bma_njDelete(p);
	}
}

BMA_DEF bma_NjNode *bma_njNew_ext(bma_IMemAlloc *pAlloc) {
	bma_NjNode *p;
	bma_assert(pAlloc != NULL);
	p = bma_malloc_ext(pAlloc, bma_NjNode);
	memset(p, 0, sizeof(*p));
	p->pAlloc = pAlloc;
	return p;
}

BMA_DEF bma_NjNode *bma_njNewNull_ext(bma_IMemAlloc *pAlloc) {
	bma_NjNode *p = bma_njNew_ext(pAlloc);
	p->type = BMA_NJ_NODE_TYPE_NULL;
	return p;
}

BMA_DEF bma_NjNode *bma_njNewBool_ext(bma_bool_t value, bma_IMemAlloc *pAlloc) {
	bma_NjNode *p = bma_njNew_ext(pAlloc);
	p->type = BMA_NJ_NODE_TYPE_BOOL;
	p->value.b = value;
	return p;
}

BMA_DEF bma_NjNode *bma_njNewSigned_ext(bma_intmax_t value, bma_IMemAlloc *pAlloc) {
	bma_NjNode *p = bma_njNew_ext(pAlloc);
	p->type = BMA_NJ_NODE_TYPE_SGND;
	p->value.si = value;
	return p;
}

BMA_DEF bma_NjNode *bma_njNewUnsigned_ext(bma_uintmax_t value, bma_IMemAlloc *pAlloc) {
	bma_NjNode *p = bma_njNew_ext(pAlloc);
	p->type = BMA_NJ_NODE_TYPE_UNSGND;
	p->value.ui = value;
	return p;
}

BMA_DEF bma_NjNode *bma_njNewDouble_ext(double value, bma_IMemAlloc *pAlloc) {
	bma_NjNode *p = bma_njNew_ext(pAlloc);
	p->type = BMA_NJ_NODE_TYPE_DBL;
	p->value.d = value;
	return p;
}

BMA_DEF bma_NjNode *bma_njNewStr_ext(const char *pValue, bma_bool_t xferOwnership, bma_IMemAlloc *pAlloc) {
	bma_NjNode *p = bma_njNew_ext(pAlloc);
	p->type = BMA_NJ_NODE_TYPE_STR;
	if (xferOwnership) {
		p->value.p = (char*)pValue;
	}
	else {
		p->value.p = bma_strdup_ext(pValue, pAlloc);
	}
	return p;
}

BMA_DEF bma_NjNode *bma_njNewStrWithLen_ext(const char *pValue, size_t valueLen, bma_IMemAlloc *pAlloc) {
	bma_NjNode *p = bma_njNew_ext(pAlloc);
	p->type = BMA_NJ_NODE_TYPE_STR;
	p->value.p = bma_strndup_ext(pValue, valueLen, pAlloc);
	return p;
}

BMA_DEF bma_NjNode *bma_njNewArr_ext(bma_IMemAlloc *pAlloc) {
	bma_NjNode *p = bma_njNew_ext(pAlloc);
	p->type = BMA_NJ_NODE_TYPE_ARR;
	bma_NjNodePtrVec_ctor_ext(&p->value.v, pAlloc);
	return p;
}

BMA_DEF void bma_njArrAppnd(bma_NjNode *pNode, bma_NjNode *pArrayElement) {
	bma_assert(pNode != NULL);
	bma_assert(pArrayElement != NULL);
	bma_assert(pNode->type == BMA_NJ_NODE_TYPE_ARR);
	bma_NjNodePtrVec_appnd(&pNode->value.v, &pArrayElement);
}

BMA_DEF void bma_njObjAppnd(bma_NjNode* pNode, bma_NjNode *pKey, bma_NjNode *pValue) {
	bma_assert(pNode != NULL);
	bma_assert(pKey != NULL);
	bma_assert(pValue != NULL);
	bma_assert(pNode->type == BMA_NJ_NODE_TYPE_OBJ);
	if (pKey->pObjectValue != NULL) {
		bma_njDelete(pKey->pObjectValue);
	}
	pKey->pObjectValue = pValue;
	bma_NjNodePtrVec_appnd(&pNode->value.v, &pKey);
}

BMA_DEF bma_NjNode *bma_njNewObj_ext(bma_IMemAlloc *pAlloc) {
	bma_NjNode *p = bma_njNew_ext(pAlloc);
	p->type = BMA_NJ_NODE_TYPE_OBJ;
	bma_NjNodePtrVec_ctor_ext(&p->value.v, pAlloc);
	return p;
}

BMA_DEF void bma_njDelete(bma_NjNode *pNode) {
	if (pNode == NULL) {
		return;
	}
	bma_assert(pNode->pAlloc != NULL);
	if (pNode->pObjectValue != NULL) {
		bma_njDelete(pNode->pObjectValue);
	}
	switch(pNode->type) {
		case BMA_NJ_NODE_TYPE_STR: {
			bma_free_ext(pNode->pAlloc, pNode->value.p);
			break;
		}
		case BMA_NJ_NODE_TYPE_ARR: /* fall through */
		case BMA_NJ_NODE_TYPE_OBJ: {
			bma_NjNodePtrVec_dtor(&pNode->value.v, NULL);
			break;
		}
		default: break;
	}
	bma_free_ext(pNode->pAlloc, pNode);
}

BMA_DEF void bma_njFmtErr(bma_StrBldr *pErrFormatter, const char* pMsg, bma_NjParseSrc *pStrWithLen) {
	if (pErrFormatter != NULL) {
		bma_StrBldr_appndStr(pErrFormatter, pMsg);
		bma_StrBldr_appndStr(pErrFormatter, " (offset ");
		bma_StrBldr_appndUnsgnd(pErrFormatter, (bma_uintmax_t)(pStrWithLen->pStr - pStrWithLen->pBegin));
		bma_StrBldr_appndStr(pErrFormatter, ")");
	}
}

BMA_DEF bma_bool_t bma_njSkip(bma_NjParseSrc *pStrWithLen, bma_StrBldr *pErrFormatter) {
	while(pStrWithLen->len > 0) {
		char c = pStrWithLen->pStr[0];
		char c2 = '\0';
		if (pStrWithLen->len > 1u) {
			c2 = pStrWithLen->pStr[1];
		}
		if ((c == ':') || (c == ',') || isspace((unsigned char)c)) {
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
			if (pEnd == NULL) {
				bma_njFmtErr(pErrFormatter, "Error parsing comment: missing closing '*/'", pStrWithLen);
				return BMA_FALSE;
			}
			pEnd += 2;
			numSkip = pEnd - pStrWithLen->pStr;
			pStrWithLen->pStr += numSkip;
			pStrWithLen->len -= numSkip;
		}
		else if (((c == '/' ) && (c2 == '/')) || (c == '#')) {
			size_t numSkip = (c == '#') ? 1u : 2u;
			while ((numSkip < pStrWithLen->len) &&
			       (pStrWithLen->pStr[numSkip] != '\n') &&
			       (pStrWithLen->pStr[numSkip] != '\r')) {
				++numSkip;
			}
			if (numSkip < pStrWithLen->len) ++numSkip; /* consume the line break */
			pStrWithLen->pStr += numSkip;
			pStrWithLen->len -= numSkip;
		}
		else {
			break;
		}
	}
	return BMA_TRUE;
}

BMA_DEF bma_NjNode *bma_njParseArr(bma_NjParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc) {
	bma_NjNode *pArray = bma_njNewArr_ext(pAlloc);
	char c;
	/* skip the '[' */
	++pStrWithLen->pStr;
	--pStrWithLen->len;
	for(;;) {
		bma_NjNode *pArrayElement;
		if (!bma_njSkip(pStrWithLen, pErrFormatter)) {
			bma_njDelete(pArray);
			return NULL;
		}
		if (pStrWithLen->len == 0) {
			bma_njDelete(pArray);
			bma_njFmtErr(pErrFormatter, "Error parsing array: end of data reached", pStrWithLen);
			return NULL;
		}
		c = *pStrWithLen->pStr;
		if (c == ']') { /* end of array */
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			return pArray;
		}
		pArrayElement = bma_njParseExt(pStrWithLen, pErrFormatter, pAlloc);
		if (pArrayElement == NULL) {
			bma_njDelete(pArray);
			return NULL;
		}
		bma_njArrAppnd(pArray, pArrayElement);
	}
	return NULL; /* unreachable */
}

BMA_DEF bma_NjNode *bma_njParseObj(bma_NjParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc) {
	bma_NjNode *pObject = bma_njNewObj_ext(pAlloc);
	char c;
	/* skip the '{' */
	++pStrWithLen->pStr;
	--pStrWithLen->len;
	for(;;) {
		bma_NjNode *pKeyElement;
		bma_NjNode *pValueElement;
		if (!bma_njSkip(pStrWithLen, pErrFormatter)) {
			bma_njDelete(pObject);
			return NULL;
		}
		if (pStrWithLen->len == 0) {
			bma_njDelete(pObject);
			bma_njFmtErr(pErrFormatter, "Error parsing object: end of data reached before key", pStrWithLen);
			return NULL;
		}
		c = *pStrWithLen->pStr;
		if (c == '}') { /* end of object */
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			return pObject;
		}
		pKeyElement = bma_njParseExt(pStrWithLen, pErrFormatter, pAlloc);
		if (pKeyElement == NULL) {
			bma_njDelete(pObject);
			return NULL;
		}
		pValueElement = bma_njParseExt(pStrWithLen, pErrFormatter, pAlloc);
		if (pValueElement == NULL) {
			bma_njDelete(pKeyElement);
			bma_njDelete(pObject);
			return NULL;
		}
		bma_njObjAppnd(pObject, pKeyElement, pValueElement);
	}
	return NULL; /* unreachable */
}

BMA_DEF bma_NjNode *bma_njParseStr(bma_NjParseSrc *pStrWithLen,
                                   bma_StrBldr *pErrFormatter,
                                   bma_IMemAlloc *pAlloc) {
	bma_NjNode *pResult = NULL;
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
			pResult = bma_njNewStr_ext(pStr, BMA_TRUE, pAlloc);
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			goto out;
		}
		case '\\': {
			char c2; 
			if (pStrWithLen->len < 2u) {
				bma_njFmtErr(pErrFormatter, "Error parsing string: escape sequence too short", pStrWithLen);
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
				size_t i;
				if (pStrWithLen->len < 6u) {
					bma_njFmtErr(pErrFormatter, "Error parsing string: unicode escape sequence too short", pStrWithLen);
					goto out;
				}
				buf[0] = pStrWithLen->pStr[2];
				buf[1] = pStrWithLen->pStr[3];
				buf[2] = pStrWithLen->pStr[4];
				buf[3] = pStrWithLen->pStr[5];
				buf[4] = '\0';
				for (i=0; i<4; ++i) {
					if (!isxdigit((unsigned char)buf[i])) {
						bma_njFmtErr(pErrFormatter, "Error parsing string: unicode escape sequence invalid", pStrWithLen);
						goto out;
					}
				}
				hex = strtoul(buf, NULL, 16);
				if (hex == 0) {
					bma_njFmtErr(pErrFormatter, "Error parsing string: unicode escape sequence \\u0000 is rejected because it terminates C strings", pStrWithLen);
					goto out;
				}
				if ((hex >= 0xD800u) && (hex <= 0xDFFFu)) {
					bma_njFmtErr(pErrFormatter, "Error parsing string: unicode escape sequence can't be a surrogate", pStrWithLen);
					goto out;
				}
				bma_StrBldr_appndCdPntUtf8(&b, hex);
				pStrWithLen->pStr += 6;
				pStrWithLen->len -= 6u;
				break;
			}
			default: {
				bma_njFmtErr(pErrFormatter, "Error parsing string: wrong escape sequence", pStrWithLen);
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
	bma_njFmtErr(pErrFormatter, "Error parsing string: missing closing quote", pStrWithLen);
out:
	bma_StrBldr_dtor(&b, NULL);
	return pResult;
}

BMA_DEF bma_NjNode *bma_njParseTok(bma_NjParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc) {
	const char *p = pStrWithLen->pStr;
	size_t len = 0;
	size_t i;
	bma_bool_t containsDotOrE = BMA_FALSE;
	bma_bool_t containsX = BMA_FALSE;
	bma_uintmax_t uValue;
	char *pEnd;
	while(pStrWithLen->len > 0) {
		char c = *pStrWithLen->pStr;
		if ((c == ':') || (c == ',') || (c == '[') || (c == ']') ||
		    (c == '{') || (c == '}') || isspace((unsigned char)c)) break;
		if ((c == '.') || (c == 'e') || (c == 'E')) containsDotOrE = BMA_TRUE;
		if ((c == 'x') || (c == 'X')) containsX = BMA_TRUE;
		++len;
		++pStrWithLen->pStr;
		--pStrWithLen->len;
	}
	if (len == 0) {
		bma_njFmtErr(pErrFormatter, "Error parsing token: length is zero", pStrWithLen);
		return NULL;
	}
	if (((p[0] >= '0') && (p[0] <= '9')) || (p[0] == '.') || (p[0] == '+') || (p[0] == '-')) {
		/* expect token to be an integer of floating point number */
		char parseBuf[BMA_NJ_NUM_PARSE_BUF_SZ];
		char *pParseBuf; /* A null terminated array to have reliable stdlib number parsing */
		if (containsX) {
			bma_njFmtErr(pErrFormatter, "Hex. numbers aren't supported", pStrWithLen);
			return NULL;
		}
		if (len < sizeof(parseBuf)) { /* fits, inclusive '\0' terminator */
			pParseBuf = parseBuf; /* avoid dyn. mem alloc in most cases */
		}
		else {
			pParseBuf = (char*)(*pAlloc->pRllc)(pAlloc, NULL, len+1, NULL);
		}
		#define NJ_FREE_PARSE_BUF() \
			do { \
				if (pParseBuf != parseBuf) { \
					bma_free_ext(pAlloc, pParseBuf); \
				} \
			} while(0)
		memcpy(pParseBuf, p, len);	
		pParseBuf[len] = '\0';
		errno = 0;
		if (containsDotOrE) { /* parse as double */
			double value;
			value = strtod(pParseBuf, &pEnd);
			if ((value == 0.0) && (errno == ERANGE)) {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing double: value out of range", pStrWithLen);
				return NULL;
			}
			if ((value == HUGE_VAL) || (value == -HUGE_VAL)) {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing double: value out of range", pStrWithLen);
				return NULL;
			}
			if (*pEnd != '\0') {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing double: invalid format", pStrWithLen);
				return NULL;
			}
			NJ_FREE_PARSE_BUF();
			return bma_njNewDouble_ext(value, pAlloc);
		}
		if (p[0] == '-') {
			bma_intmax_t value;
			#ifdef LLONG_MAX
				value = (bma_intmax_t)strtoll(pParseBuf, &pEnd, 10);
				if (((value == LLONG_MAX) || (value == LLONG_MIN) || (value == 0)) && (errno == ERANGE)) {
					NJ_FREE_PARSE_BUF();
					bma_njFmtErr(pErrFormatter, "Error parsing signed integer: value out of range", pStrWithLen);
					return NULL;
				}
			#else
				value = (bma_intmax_t)strtol(pParseBuf, &pEnd, 10);
				if (((value == LONG_MAX) || (value == LONG_MIN) || (value == 0)) && (errno == ERANGE)) {
					NJ_FREE_PARSE_BUF();
					bma_njFmtErr(pErrFormatter, "Error parsing signed integer: value out of range", pStrWithLen);
					return NULL;
				}
			#endif
			if (*pEnd != '\0') {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing signed integer: invalid format", pStrWithLen);
				return NULL;
			}
			NJ_FREE_PARSE_BUF();
			return bma_njNewSigned_ext(value, pAlloc);
		}
		#ifdef ULLONG_MAX
			uValue = (bma_uintmax_t)strtoull(pParseBuf, &pEnd, 10);
			if (((uValue == ULLONG_MAX) || (uValue == 0)) && (errno == ERANGE)) {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing unsigned integer: value out of range", pStrWithLen);
				return NULL;
			}
			if (*pEnd != '\0') {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing unsigned integer: invalid format", pStrWithLen);
				return NULL;
			}
		#else
			uValue = (bma_uintmax_t)strtoul(pParseBuf, &pEnd, 10);
			if (((uValue == ULONG_MAX) || (uValue == 0)) && (errno == ERANGE)) {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing unsigned integer: value out of range", pStrWithLen);
				return NULL;
			}
			if (*pEnd != '\0') {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing unsigned integer: invalid format", pStrWithLen);
				return NULL;
			}
		#endif
		NJ_FREE_PARSE_BUF();
		return bma_njNewUnsigned_ext(uValue, pAlloc);
		#undef NJ_FREE_PARSE_BUF
	}
	if ((len == 4u) && (memcmp(p, "null", 4u) == 0)) {
		return bma_njNewNull_ext(pAlloc);
	}
	if ((len == 4u) && (memcmp(p, "true", 4u) == 0)) {
		return bma_njNewBool_ext(BMA_TRUE, pAlloc);
	}
	if ((len == 5u) && (memcmp(p, "false", 5u) == 0)) {
		return bma_njNewBool_ext(BMA_FALSE, pAlloc);
	}
	if (((p[0] >= 'a') && (p[0] <= 'z')) || ((p[0] >= 'A') && (p[0] <= 'Z')) || (p[0] == '_'))
	{
		for (i=1u; i<len; ++i) {
			if (((p[i] >= 'a') && (p[i] <= 'z')) || ((p[i] >= 'A') && (p[i] <= 'Z')) ||
                            ((p[i] >= '0') && (p[i] <= '9')) || (p[i] == '_')) {
				/* good */
			}
			else {
				bma_njFmtErr(pErrFormatter, "Error parsing token: invalid", pStrWithLen);
				return NULL;
			}
		}
		return bma_njNewStrWithLen_ext(p, len, pAlloc);
	}
	bma_njFmtErr(pErrFormatter, "Error parsing token: invalid", pStrWithLen);
	return NULL;
}

BMA_DEF bma_NjNode *bma_njParseExt(bma_NjParseSrc *pStrWithLen,
                                   bma_StrBldr *pErrFormatter,
                                   bma_IMemAlloc *pAlloc) {
	char c;
	bma_assert(pStrWithLen != NULL);
	bma_assert(pAlloc != NULL);
	if (!bma_njSkip(pStrWithLen, pErrFormatter)) return NULL;
	if (pStrWithLen->len == 0) {
		bma_njFmtErr(pErrFormatter, "Error parsing: unexpected end of data", pStrWithLen);
		return NULL;
	}
	c = *pStrWithLen->pStr;
	switch(c) {
		case '[': return bma_njParseArr(pStrWithLen, pErrFormatter, pAlloc);
		case '{': return bma_njParseObj(pStrWithLen, pErrFormatter, pAlloc);
		case '"': return bma_njParseStr(pStrWithLen, pErrFormatter, pAlloc);
		default: return bma_njParseTok(pStrWithLen, pErrFormatter, pAlloc);
	}
}

BMA_DEF bma_NjNode *bma_njParseRoot(bma_NjParseSrc *pStrWithLen,
                                    bma_StrBldr *pErrFormatter,
                                    bma_IMemAlloc *pAlloc) {
	bma_NjNode *pNode = bma_njParseExt(pStrWithLen, pErrFormatter, pAlloc);
	if (pNode == NULL) return NULL;
	if (!bma_njSkip(pStrWithLen, pErrFormatter)) {
		bma_njDelete(pNode);
		return NULL;
	}
	if (pStrWithLen->len != 0) {
		bma_njDelete(pNode);
		bma_njFmtErr(pErrFormatter, "Error parsing: unexpected data after root value", pStrWithLen);
		return NULL;
	}
	return pNode;
}

BMA_DEF bma_NjNode *bma_njParseMem(const void *pMem, size_t len) {
	bma_NjParseSrc strWithLen;
	strWithLen.pBegin = (const char*)pMem;
	strWithLen.pStr = (const char*)pMem;
	strWithLen.len = len;
	return bma_njParseRoot(&strWithLen, NULL, bma_getDfltMemAlloc());
}

BMA_DEF bma_NjNode *bma_njParse(const char *pStr) {
	bma_NjParseSrc strWithLen;
	strWithLen.pBegin = pStr;
	strWithLen.pStr = pStr;
	strWithLen.len = strlen(pStr);
	return bma_njParseRoot(&strWithLen, NULL, bma_getDfltMemAlloc());
}

BMA_DEF bma_NjNode *bma_njGet(bma_NjNode *pNode, const char *pPath) {
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
			if (idx > BMA_SIZE_MAX) return NULL;
			if (pNode->type != BMA_NJ_NODE_TYPE_ARR) return NULL;
			if (idx >= bma_NjNodePtrVec_getSz(&pNode->value.v)) return NULL;
			pNode = *bma_NjNodePtrVec_at(&pNode->value.v, (size_t)idx);
			num = (size_t)((pEnd+1) - pPath);
			pPath += num;
			pathLen -= num;
		}
		else { /* find object element by key string */
			size_t num;
			size_t i;
			bma_NjNode *pSubNode;
			bma_bool_t found = BMA_FALSE;
			const char* pEnd = strchr(pPath, '/');
			if (pEnd == NULL) pEnd = pPath + pathLen;
			num = (size_t)(pEnd - pPath);
			if (num == 0) return NULL;
			if (pNode->type != BMA_NJ_NODE_TYPE_OBJ) return NULL;
			for (i=0; i<bma_NjNodePtrVec_getSz(&pNode->value.v); ++i) {
				pSubNode = *bma_NjNodePtrVec_at(&pNode->value.v, i);
				if (pSubNode->type != BMA_NJ_NODE_TYPE_STR) return NULL;
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

BMA_DEF bma_bool_t bma_njGetBool_ext(bma_NjNode *pNode, const char *pPath, bma_bool_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess) {
	bma_NjNode *pSubNode;
	bma_assert(pNode != NULL);
	bma_njSetFalse(pOptOutSuccess);
	pSubNode = bma_njGet(pNode, pPath);
	if (pSubNode == NULL) {
		return defaultWhenNotFound;
	}
	if (pSubNode->type != BMA_NJ_NODE_TYPE_BOOL) {
		return defaultWhenNotFound;
	}
	bma_njSetTrue(pOptOutSuccess);
	return pSubNode->value.b;
}

BMA_DEF bma_intmax_t bma_njGetInt_ext(bma_NjNode *pNode, const char *pPath, bma_intmax_t minValue, bma_intmax_t maxValue, bma_intmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess) {
	bma_NjNode *pSubNode;
	bma_intmax_t result;
	bma_assert(pNode != NULL);
	bma_njSetFalse(pOptOutSuccess);
	pSubNode = bma_njGet(pNode, pPath);
	if (pSubNode == NULL) return defaultWhenNotFound;
	switch(pSubNode->type) {
		case BMA_NJ_NODE_TYPE_BOOL: result = (pSubNode->value.b) ? 1 : 0; break;
		case BMA_NJ_NODE_TYPE_SGND: result = pSubNode->value.si; break;
		case BMA_NJ_NODE_TYPE_UNSGND: {
			if (pSubNode->value.ui > (bma_uintmax_t)BMA_INTMAX_MAX) return defaultWhenNotFound;
			result = (bma_intmax_t)pSubNode->value.ui;
			break;
		}
		case BMA_NJ_NODE_TYPE_DBL: {
			double d = pSubNode->value.d;
			/* (double)INTMAX_MIN is exactly -2^(N-1); negated it's the exclusive upper bound */
			if (!((d >= (double)BMA_INTMAX_MIN) && (d < -(double)BMA_INTMAX_MIN))) return defaultWhenNotFound;
			result = (bma_intmax_t)d;
			if (((double)result) != d) return defaultWhenNotFound;
			break;
		}
		default: return defaultWhenNotFound;
	}
	if ((result < minValue) || (result > maxValue)) return defaultWhenNotFound;
	bma_njSetTrue(pOptOutSuccess);
	return result;
}

BMA_DEF bma_uintmax_t bma_njGetUint_ext(bma_NjNode *pNode, const char *pPath, bma_uintmax_t minValue, bma_uintmax_t maxValue, bma_uintmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess) {
	bma_NjNode *pSubNode;
	bma_uintmax_t result;
	bma_assert(pNode != NULL);
	bma_njSetFalse(pOptOutSuccess);
	pSubNode = bma_njGet(pNode, pPath);
	if (pSubNode == NULL) return defaultWhenNotFound;
	switch(pSubNode->type) {
		case BMA_NJ_NODE_TYPE_BOOL: result = (pSubNode->value.b) ? 1u : 0u; break;
		case BMA_NJ_NODE_TYPE_SGND: {
			if (pSubNode->value.si < 0) return defaultWhenNotFound;
			result = (bma_uintmax_t)pSubNode->value.si;
			break;
		}
		case BMA_NJ_NODE_TYPE_UNSGND: {
			result = pSubNode->value.ui;
			break;
		}
		case BMA_NJ_NODE_TYPE_DBL: {
			double d = pSubNode->value.d;
			/* exactly 2^N; (double)UINTMAX_MAX alone would round */
			double lim = 2.0 * (double)((BMA_UINTMAX_MAX >> 1) + 1u);
			if (!((d >= 0.0) && (d < lim))) return defaultWhenNotFound;
			result = (bma_uintmax_t)d;
			if (((double)result) != d) return defaultWhenNotFound;
			break;
		}
		default: return defaultWhenNotFound;
	}
	if ((result < minValue) || (result > maxValue)) return defaultWhenNotFound;
	bma_njSetTrue(pOptOutSuccess);
	return result;
}

BMA_DEF double bma_njGetDouble_ext(bma_NjNode *pNode, const char *pPath, double minValue, double maxValue, double defaultWhenNotFound, bma_bool_t *pOptOutSuccess) {
	bma_NjNode *pSubNode;
	double result;
	bma_assert(pNode != NULL);
	bma_njSetFalse(pOptOutSuccess);
	pSubNode = bma_njGet(pNode, pPath);
	if (pSubNode == NULL) return defaultWhenNotFound;
	switch(pSubNode->type) {
		case BMA_NJ_NODE_TYPE_BOOL: result = (pSubNode->value.b) ? 1.0 : 0.0; break;
		case BMA_NJ_NODE_TYPE_SGND: {
			result = (double)pSubNode->value.si;
			if (result >= -(double)BMA_INTMAX_MIN) return defaultWhenNotFound;
			if ((bma_intmax_t)result != pSubNode->value.si) return defaultWhenNotFound;
			break;
		}
		case BMA_NJ_NODE_TYPE_UNSGND: {
			result = (double)pSubNode->value.ui;
			if (result >= 2.0 * (double)((BMA_UINTMAX_MAX >> 1) + 1u)) return defaultWhenNotFound;
			if ((bma_uintmax_t)result != pSubNode->value.ui) return defaultWhenNotFound;
			break;
		}
		case BMA_NJ_NODE_TYPE_DBL: {
			result = pSubNode->value.d;
			break;
		}
		default: return defaultWhenNotFound;
	}
	if ((result < minValue) || (result > maxValue)) return defaultWhenNotFound;
	bma_njSetTrue(pOptOutSuccess);
	return result;
}

BMA_DEF bma_bool_t bma_njGetString_ext(bma_StrBldr *pDst, bma_NjNode *pNode, const char *pPath, const char *pDefaultWhenNotFound) {
	bma_NjNode *pSubNode;
	bma_assert(pDst != NULL);
	bma_assert(pNode != NULL);
	pSubNode = bma_njGet(pNode, pPath);
	if (pSubNode == NULL) {
		bma_StrBldr_appndStr(pDst, pDefaultWhenNotFound);
		return BMA_FALSE;
	}
	switch(pSubNode->type) {
		case BMA_NJ_NODE_TYPE_BOOL:   bma_StrBldr_appndBool(pDst, pSubNode->value.b);    break;
		case BMA_NJ_NODE_TYPE_SGND:   bma_StrBldr_appndSgnd(pDst, pSubNode->value.si);   break;
		case BMA_NJ_NODE_TYPE_UNSGND: bma_StrBldr_appndUnsgnd(pDst, pSubNode->value.ui); break;
		case BMA_NJ_NODE_TYPE_DBL:    bma_StrBldr_appndDbl(pDst, pSubNode->value.d);     break;
		case BMA_NJ_NODE_TYPE_STR:    bma_StrBldr_appndStr(pDst, pSubNode->value.p);     break;
		default: {
			bma_StrBldr_appndStr(pDst, pDefaultWhenNotFound);
			return BMA_FALSE;
		}
	}
	return BMA_TRUE;
}

BMA_DEF void bma_njPutIndent(bma_StrBldr *pDst, const char *pIndent, size_t indentSize, size_t indentLevel) {
	size_t i;
	bma_assert(pDst != NULL);
	bma_assert(pIndent != NULL);
	for (i=0; i<indentLevel; ++i) {
		bma_StrBldr_appndStrN(pDst, pIndent, indentSize);
	}
}

/*! Checks if the string could be printed without escapes or double quotes in NJ */
BMA_DEF bma_bool_t bma_njIsSmplTok(const char *pStr, size_t len) {
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

BMA_DEF void bma_njToStr_impl(bma_StrBldr *pDst, bma_NjNode *pNode, bma_NjDialect dialect, bma_bool_t pretty, bma_bool_t initialIndent, const char *pIndent, size_t indentSize, size_t indentLevel) {
	bma_assert(pDst != NULL);
	bma_assert(pNode != NULL);
	bma_assert(pIndent != NULL);
	if (pretty && initialIndent) {
		bma_njPutIndent(pDst, pIndent, indentSize, indentLevel);
	}
	switch(pNode->type) {
		case BMA_NJ_NODE_TYPE_NULL:   bma_StrBldr_appndStrN(pDst, "null", 4u); break;
		case BMA_NJ_NODE_TYPE_BOOL:   bma_StrBldr_appndBool(pDst, pNode->value.b);    break;
		case BMA_NJ_NODE_TYPE_SGND:   bma_StrBldr_appndSgnd(pDst, pNode->value.si);   break;
		case BMA_NJ_NODE_TYPE_UNSGND: bma_StrBldr_appndUnsgnd(pDst, pNode->value.ui); break;
		case BMA_NJ_NODE_TYPE_DBL:    bma_StrBldr_appndDbl(pDst, pNode->value.d);     break;
		case BMA_NJ_NODE_TYPE_STR: {
			const char *p = pNode->value.p;
			size_t len = strlen(pNode->value.p);
			if ((dialect == BMA_NJ_DIALECT_NJ) && bma_njIsSmplTok(p, len)) {
				bma_StrBldr_appndStrN(pDst, p, len);
			}
			else {
				bma_jsonEscape(pDst, p, len);
			}
			break;
		}
		case BMA_NJ_NODE_TYPE_ARR: {
			size_t i, size;
			bma_NjNode *pSubNode;
			bma_StrBldr_appndChr(pDst, '[', 1u);
			if (pretty) {
				bma_StrBldr_appndChr(pDst, '\n', 1u);
			}
			++indentLevel;
			size = bma_NjNodePtrVec_getSz(&pNode->value.v);
			for (i=0; i<size; ++i) {
				pSubNode = *bma_NjNodePtrVec_at(&pNode->value.v, i);
				bma_njToStr_impl(pDst, pSubNode, dialect, pretty, BMA_TRUE, pIndent, indentSize, indentLevel);
				if (((dialect == BMA_NJ_DIALECT_NJ) && (size > 1)) || ((i+1) < size)) {
					bma_StrBldr_appndChr(pDst, ',', 1u);
				}
				if (pretty) {
					bma_StrBldr_appndChr(pDst, '\n', 1u);
				}
			}
			--indentLevel;
			if (pretty) {
				bma_njPutIndent(pDst, pIndent, indentSize, indentLevel);
			}
			bma_StrBldr_appndChr(pDst, ']', 1u);
			break;
		}
		case BMA_NJ_NODE_TYPE_OBJ: {
			size_t i, size;
			bma_NjNode *pKeyNode, *pValueNode;
			bma_StrBldr_appndChr(pDst, '{', 1u);
			if (pretty) {
				bma_StrBldr_appndChr(pDst, '\n', 1u);
			}
			++indentLevel;
			size = bma_NjNodePtrVec_getSz(&pNode->value.v);
			for (i=0; i<size; ++i) {
				pKeyNode = *bma_NjNodePtrVec_at(&pNode->value.v, i);
				pValueNode = pKeyNode->pObjectValue;
				bma_njToStr_impl(pDst, pKeyNode, dialect, pretty, BMA_TRUE, pIndent, indentSize, indentLevel);
				bma_StrBldr_appndChr(pDst, ':', 1u);
				if (pretty) {
					bma_StrBldr_appndChr(pDst, ' ', 1u);
				}
				bma_njToStr_impl(pDst, pValueNode, dialect, pretty, BMA_FALSE, pIndent, indentSize, indentLevel);
				if (((dialect == BMA_NJ_DIALECT_NJ) && (size > 1)) || ((i+1) < size)) {
					bma_StrBldr_appndChr(pDst, ',', 1u);
				}
				if (pretty) {
					bma_StrBldr_appndChr(pDst, '\n', 1u);
				}
			}
			--indentLevel;
			if (pretty) {
				bma_njPutIndent(pDst, pIndent, indentSize, indentLevel);
			}
			bma_StrBldr_appndChr(pDst, '}', 1u);
			break;
		}
		default: break;
	}
}

BMA_DEF void bma_njToStr_ext(bma_StrBldr *pDst, bma_NjNode *pNode, bma_NjDialect dialect, bma_bool_t pretty, const char *pIndent) {
	bma_assert(pDst != NULL);
	bma_assert(pNode != NULL);
	bma_assert(pIndent != NULL);
	bma_njToStr_impl(pDst, pNode, dialect, pretty, BMA_TRUE, pIndent, strlen(pIndent), 0);
	if (pretty) {
		bma_StrBldr_appndChr(pDst, '\n', 1u);
	}
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_NJ_IMPL */

#ifdef BMA_NJ_TEST

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
	bma_NjNode *pNode;
	bma_NjNode *pSubNode;
	pNode = bma_njNew();
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_INVLD);
	BMA_EXPECT(pNode->pObjectValue == NULL);
	BMA_EXPECT(pNode->pAlloc == bma_getDfltMemAlloc());
	bma_njDelete(pNode);
	pNode = bma_njNewNull();
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_NULL);
	bma_njDelete(pNode);
	pNode = bma_njNewBool(BMA_FALSE);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_BOOL);
	BMA_EXPECT(!pNode->value.b);
	bma_njDelete(pNode);
	pNode = bma_njNewBool(BMA_TRUE);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_BOOL);
	BMA_EXPECT(pNode->value.b);
	bma_njDelete(pNode);
	pNode = bma_njNewSigned(BMA_INTMAX_MIN);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_SGND);
	BMA_EXPECT(pNode->value.si == BMA_INTMAX_MIN);
	bma_njDelete(pNode);
	pNode = bma_njNewSigned(BMA_INTMAX_MAX);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_SGND);
	BMA_EXPECT(pNode->value.si == BMA_INTMAX_MAX);
	bma_njDelete(pNode);
	pNode = bma_njNewUnsigned(0u);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_UNSGND);
	BMA_EXPECT(pNode->value.ui == 0u);
	bma_njDelete(pNode);
	pNode = bma_njNewUnsigned(BMA_UINTMAX_MAX);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_UNSGND);
	BMA_EXPECT(pNode->value.ui == BMA_UINTMAX_MAX);
	bma_njDelete(pNode);
	pNode = bma_njNewDouble(3.14);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_DBL);
	BMA_EXPECT(fabs(pNode->value.d - 3.14) < 0.001);
	bma_njDelete(pNode);
	pNode = bma_njNewStr("Hello");
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "Hello") == 0);
	bma_njDelete(pNode);
	pNode = bma_njNewStrWithLen("Hello", 3u);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "Hel") == 0);
	bma_njDelete(pNode);
	pNode = bma_njNewArr();
	bma_njArrAppnd(pNode, bma_njNewStr("Hello"));
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_ARR);
	bma_njDelete(pNode);
	pNode = bma_njNewObj();
	bma_njObjAppnd(pNode, bma_njNewStr("Key"), bma_njNewStr("Hello"));
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_OBJ);
	pSubNode = *bma_NjNodePtrVec_at(&pNode->value.v, 0);
	BMA_EXPECT(strcmp(pSubNode->value.p, "Key") == 0);
	BMA_EXPECT(pSubNode->pObjectValue != NULL);
	BMA_EXPECT(strcmp(pSubNode->pObjectValue->value.p, "Hello") == 0);
	bma_njDelete(pNode);
}

void test_parse(void) {
	bma_NjNode *pNode;
	pNode = bma_njParse("null");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_NULL);
	bma_njDelete(pNode);
	pNode = bma_njParse("true");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_BOOL);
	BMA_EXPECT(pNode->value.b == BMA_TRUE);
	bma_njDelete(pNode);
	pNode = bma_njParse("false");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_BOOL);
	BMA_EXPECT(pNode->value.b == BMA_FALSE);
	bma_njDelete(pNode);
	pNode = bma_njParse("4711");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_UNSGND);
	BMA_EXPECT(pNode->value.ui == 4711);
	bma_njDelete(pNode);
	pNode = bma_njParse("-4711");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_SGND);
	BMA_EXPECT(pNode->value.si == -4711);
	bma_njDelete(pNode);
	pNode = bma_njParse("3.14");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_DBL);
	BMA_EXPECT(fabs(pNode->value.d - 3.14) <= 0.001);
	bma_njDelete(pNode);
	pNode = bma_njParse("Hello");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "Hello") == 0);
	bma_njDelete(pNode);
	pNode = bma_njParse("[]");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 0);
	bma_njDelete(pNode);
	pNode = bma_njParse("[1]");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 1);
	bma_njDelete(pNode);
	pNode = bma_njParse("[1 Hello]");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_njDelete(pNode);
	pNode = bma_njParse("[1,Hello,]");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_njDelete(pNode);
	pNode = bma_njParse("{A:1,B:2}");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_OBJ);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_njDelete(pNode);
	pNode = bma_njParse("{A 1 B 2}");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_OBJ);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_njDelete(pNode);
	pNode = bma_njParse(" // comment \n[1 /* comment */ 2] // comment");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_ARR);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_njDelete(pNode);
	pNode = bma_njParse("\"Hello\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "Hello") == 0);
	bma_njDelete(pNode);
	pNode = bma_njParse("\"\\r\\n\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "\r\n") == 0);
	bma_njDelete(pNode);
	pNode = bma_njParse("\"\\\"\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "\"") == 0);
	bma_njDelete(pNode);
	pNode = bma_njParse("\"\\u0001\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "\x01") == 0);
	bma_njDelete(pNode);
	pNode = bma_njParse("\"H\\u00E4llo\"");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pNode->value.p, "H\xC3\xA4llo") == 0);
	bma_njDelete(pNode);
	pNode = bma_njParse(
		"{\n"
		"	name: Anybody,\n"
		"	age: 42, // comment\n"
		"	hobbies: [reading, writing, swimming,]\n"
		"}\n"
	);
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_OBJ);
	bma_njDelete(pNode);
}

void test_trailing(void) {
	bma_NjNode *pNode;
	bma_NjParseSrc src;
	bma_StrBldr err;
	BMA_EXPECT(bma_njParse("1 2") == NULL);
	BMA_EXPECT(bma_njParse("[1]]") == NULL);
	BMA_EXPECT(bma_njParse("{a 1} garbage") == NULL);
	BMA_EXPECT(bma_njParseMem("[1] x", 5u) == NULL);
	pNode = bma_njParseMem("[1] x", 3u);
	BMA_EXPECT(pNode != NULL);
	bma_njDelete(pNode);
	pNode = bma_njParse("[1] // trailing comment\n  ");
	BMA_EXPECT(pNode != NULL);
	bma_njDelete(pNode);
	pNode = bma_njParse("  42 , ");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_UNSGND);
	bma_njDelete(pNode);
	bma_StrBldr_ctor(&err);
	src.pBegin = "1 2";
	src.pStr = src.pBegin;
	src.len = 3u;
	BMA_EXPECT(bma_njParseRoot(&src, &err, bma_getDfltMemAlloc()) == NULL);
	BMA_EXPECT(strstr(bma_StrBldr_getStr(&err), "unexpected data after root value (offset 2)") != NULL);
	bma_StrBldr_dtor(&err, NULL);
}

void test_comments(void) {
	bma_NjNode *pNode;
	bma_NjParseSrc src;
	bma_StrBldr err;
	BMA_EXPECT(bma_njParse("/* x") == NULL);
	BMA_EXPECT(bma_njParse("[1 /* x") == NULL);
	BMA_EXPECT(bma_njParse("{a /* x") == NULL);
	BMA_EXPECT(bma_njParse("[1] /* x") == NULL);
	BMA_EXPECT(bma_njParse("[1] /*") == NULL);
	BMA_EXPECT(bma_njParse("[1] /*/") == NULL);
	pNode = bma_njParse("/**/[1 /* a */ 2] /* b */");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 2);
	bma_njDelete(pNode);
	pNode = bma_njParse("# c\n[1 # c\r2 // c\n3]#");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 3);
	bma_njDelete(pNode);
	bma_StrBldr_ctor(&err);
	src.pBegin = "[1] /* x";
	src.pStr = src.pBegin;
	src.len = strlen(src.pBegin);
	BMA_EXPECT(bma_njParseRoot(&src, &err, bma_getDfltMemAlloc()) == NULL);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&err), "Error parsing comment: missing closing '*/' (offset 4)") == 0);
	bma_StrBldr_dtor(&err, NULL);
}

void test_get(void) {
	bma_NjNode *pNode, *pSubNode;
	pNode = bma_njParse(
		"{\n"
		"	name: Anybody,\n"
		"	age: 42, // comment\n"
		"	hobbies: [reading, writing, swimming,]\n"
		"}\n"
	);
	BMA_EXPECT(pNode != NULL);
	pSubNode = bma_njGet(pNode, "name");
	BMA_EXPECT(pSubNode != NULL);
	BMA_EXPECT(pSubNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pSubNode->value.p, "Anybody") == 0);
	pSubNode = bma_njGet(pNode, "age");
	BMA_EXPECT(pSubNode != NULL);
	BMA_EXPECT(pSubNode->type == BMA_NJ_NODE_TYPE_UNSGND);
	BMA_EXPECT(pSubNode->value.ui == 42u);
	pSubNode = bma_njGet(pNode, "hobbies/[2]");
	BMA_EXPECT(pSubNode != NULL);
	BMA_EXPECT(pSubNode->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pSubNode->value.p, "swimming") == 0);
	bma_njDelete(pNode);
}

void test_toString(void) {
	/*
	BMA_DEF void bma_njToStr_ext(bma_StrBldr *pDst, bma_NjNode *pNode, bma_NjDialect dialect, bma_bool_t pretty, const char *pIndent) {
	*/
	bma_StrBldr b;
	bma_NjNode *pNode;
	bma_StrBldr_ctor(&b);
	pNode = bma_njParse("null");
	BMA_EXPECT(pNode != NULL);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_FALSE, "");
	bma_njDelete(pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "null") == 0);
	bma_StrBldr_clear(&b);
	pNode = bma_njParse("true");
	BMA_EXPECT(pNode != NULL);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_FALSE, "");
	bma_njDelete(pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "true") == 0);
	bma_StrBldr_clear(&b);
	pNode = bma_njParse("false");
	BMA_EXPECT(pNode != NULL);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_FALSE, "");
	bma_njDelete(pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "false") == 0);
	bma_StrBldr_clear(&b);
	pNode = bma_njParse("0");
	BMA_EXPECT(pNode != NULL);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_FALSE, "");
	bma_njDelete(pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "0") == 0);
	bma_StrBldr_clear(&b);
	pNode = bma_njParse("\"Hello\"");
	BMA_EXPECT(pNode != NULL);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_FALSE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "\"Hello\"") == 0);
	bma_StrBldr_clear(&b);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_NJ, BMA_TRUE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "Hello\n") == 0);
	bma_njDelete(pNode);
	bma_StrBldr_clear(&b);
	pNode = bma_njParse("[]");
	BMA_EXPECT(pNode != NULL);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_FALSE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[]") == 0);
	bma_StrBldr_clear(&b);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_TRUE, "\t");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[\n]\n") == 0);
	bma_StrBldr_clear(&b);
	bma_njDelete(pNode);
	pNode = bma_njParse("[1]");
	BMA_EXPECT(pNode != NULL);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_FALSE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[1]") == 0);
	bma_StrBldr_clear(&b);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_TRUE, "\t");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[\n\t1\n]\n") == 0);
	bma_StrBldr_clear(&b);
	bma_njDelete(pNode);
	pNode = bma_njParse("[1,2]");
	BMA_EXPECT(pNode != NULL);
	bma_njToStr(&b, pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[\n\t1,\n\t2\n]\n") == 0);
	bma_StrBldr_clear(&b);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_NJ, BMA_TRUE, "\t");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[\n\t1,\n\t2,\n]\n") == 0);
	bma_StrBldr_clear(&b);
	bma_njDelete(pNode);
	pNode = bma_njParse("{key:1,value:2}");
	BMA_EXPECT(pNode != NULL);
	bma_njToStr(&b, pNode);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b),
		"{\n"
		"	\"key\": 1,\n"
		"	\"value\": 2\n"
		"}\n"
	) == 0);
	bma_StrBldr_clear(&b);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_NJ, BMA_TRUE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b),
		"{\n"
		"key: 1,\n"
		"value: 2,\n"
		"}\n"
	) == 0);
	bma_StrBldr_clear(&b);
	bma_njDelete(pNode);
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
	BMA_TEST(test_trailing);
	BMA_TEST(test_comments);
	BMA_TEST(test_get);
	BMA_TEST(test_toString);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_NJ_TEST */

#endif /* BMA_NJ_H_INCLUDED */
