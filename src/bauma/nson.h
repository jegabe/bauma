#ifndef BAUMA_NSON_H_INCLUDED
#define BAUMA_NSON_H_INCLUDED

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
#include <bauma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef BAUMA_NSON_DEF
	#define BAUMA_NSON_DEF
#endif

typedef struct bauma_nson_StringWithLength {
	const char *pBegin; /* for error reporting to calculate error offset */
	const char *pStr; /* incremented while parsing */
	size_t len; /* decremented while parsing */
} bauma_nson_StringWithLength;

typedef unsigned int bauma_NsonNodeType;

#define BAUMA_NSON_NODE_TYPE_INVALID  0
#define BAUMA_NSON_NODE_TYPE_NULL     1
#define BAUMA_NSON_NODE_TYPE_BOOL     2
#define BAUMA_NSON_NODE_TYPE_SIGNED   3
#define BAUMA_NSON_NODE_TYPE_UNSIGNED 4
#define BAUMA_NSON_NODE_TYPE_DOUBLE   5
#define BAUMA_NSON_NODE_TYPE_STRING   6
#define BAUMA_NSON_NODE_TYPE_ARRAY    7
#define BAUMA_NSON_NODE_TYPE_OBJECT   8
#define BAUMA_NSON_NODE_NUM_OF_TYPES  9

typedef unsigned int bauma_NsonDialect;

#define BAUMA_NSON_DIALECT_JSON            0
#define BAUMA_NSON_DIALECT_NSON            1
#define BAUMA_NSON_DIALECT_NUM_OF_DIALECTS 2

typedef union bauma_NsonNodeUnion {
	bauma_bool_t    b;
	bauma_intmax_t  si;
	bauma_uintmax_t ui;
	double          d;
	char            *p;
	bauma_Vector    v; /* for maps and arrays, this points to all values */
} bauma_NsonNodeUnion;

typedef struct bauma_NsonNode bauma_NsonNode;

struct bauma_NsonNode {
	bauma_NsonNodeType    type;
	bauma_NsonNodeUnion   value;
	bauma_NsonNode        *pObjectValue; /* ptr from key to value in objs */
	bauma_IMemAllocator   *pAlloc;
};

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_new_ext(bauma_IMemAllocator *pAlloc);
#define bauma_nson_new() bauma_nson_new_ext(bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newNull_ext(bauma_IMemAllocator *pAlloc);
#define bauma_nson_newNull() bauma_nson_newNull_ext(bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newBool_ext(bauma_bool_t value, bauma_IMemAllocator *pAlloc);
#define bauma_nson_newBool(value) bauma_nson_newBool_ext((value), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newSigned_ext(bauma_intmax_t value, bauma_IMemAllocator *pAlloc);
#define bauma_nson_newSigned(value) bauma_nson_newSigned_ext((value), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newUnsigned_ext(bauma_uintmax_t value, bauma_IMemAllocator *pAlloc);
#define bauma_nson_newUnsigned(value) bauma_nson_newUnsigned_ext((value), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newDouble_ext(double value, bauma_IMemAllocator *pAlloc);
#define bauma_nson_newDouble(value) bauma_nson_newDouble_ext((value), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newStr_ext(const char *pValue, bauma_bool_t xferOwnership, bauma_IMemAllocator *pAlloc);
#define bauma_nson_newStr(value) bauma_nson_newStr_ext((value), BAUMA_FALSE, bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newStrWithLen_ext(const char *pValue, size_t valueLen, bauma_IMemAllocator *pAlloc);
#define bauma_nson_newStrWithLen(value, len) bauma_nson_newStrWithLen_ext((value), (len), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newArray_ext(bauma_IMemAllocator *pAlloc);
#define bauma_nson_newArray() bauma_nson_newArray_ext(bauma_getDefaultMemAllocator())
BAUMA_NSON_DEF void bauma_nson_array_append(bauma_NsonNode* pNode, bauma_NsonNode* pArrayElement);

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newObject_ext(bauma_IMemAllocator *pAlloc);
#define bauma_nson_newObject() bauma_nson_newObject_ext(bauma_getDefaultMemAllocator())
BAUMA_NSON_DEF void bauma_nson_object_append(bauma_NsonNode* pNode, bauma_NsonNode *pKey, bauma_NsonNode *pValue);

BAUMA_NSON_DEF void bauma_nson_delete(bauma_NsonNode *pNode);

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parse_ext(bauma_nson_StringWithLength *pStrWithLen,
                                                    bauma_StringBuilder *pErrFormatter,
                                                    bauma_IMemAllocator *pAlloc);

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parseMem(const void *pMem, size_t len);
BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parseStr(const char *pStr);

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_get(bauma_NsonNode *pNode, const char *pPath);

BAUMA_NSON_DEF bauma_bool_t bauma_nson_getBool_ext(bauma_NsonNode *pNode, const char *pPath, bauma_bool_t defaultWhenNotFound, bauma_bool_t *pOptOutSuccess);
#define bauma_nson_getBool(pNode, pPath) bauma_nson_getBool_ext((pNode), (pPath), BAUMA_FALSE, NULL)

BAUMA_NSON_DEF bauma_intmax_t bauma_nson_getInt_ext(bauma_NsonNode *pNode, const char *pPath, bauma_intmax_t minValue, bauma_intmax_t maxValue, bauma_intmax_t defaultWhenNotFound, bauma_bool_t *pOptOutSuccess);
#define bauma_nson_getShort(pNode, pPath) ((short)bauma_nson_getInt_ext((pNode), (pPath), SHRT_MIN, SHRT_MAX, 0, NULL))
#define bauma_nson_getInt(pNode, pPath) ((int)bauma_nson_getInt_ext((pNode), (pPath), INT_MIN, INT_MAX, 0, NULL))
#define bauma_nson_getLong(pNode, pPath) ((long)bauma_nson_getInt_ext((pNode), (pPath), LONG_MIN, LONG_MAX, 0, NULL))
#ifdef LLONG_MAX
	#define bauma_nson_getLongLong(pNode, pPath) ((long long)bauma_nson_getInt_ext((pNode), (pPath), LLONG_MIN, LLONG_MAX, 0, NULL))
#endif

BAUMA_NSON_DEF bauma_uintmax_t bauma_nson_getUint_ext(bauma_NsonNode *pNode, const char *pPath, bauma_uintmax_t minValue, bauma_uintmax_t maxValue, bauma_uintmax_t defaultWhenNotFound, bauma_bool_t *pOptOutSuccess);
#define bauma_nson_getUshort(pNode, pPath) ((unsigned short)bauma_nson_getUint_ext((pNode), (pPath), 0, USHRT_MAX, 0, NULL))
#define bauma_nson_getUint(pNode, pPath) ((unsigned int)bauma_nson_getUint_ext((pNode), (pPath), 0, UINT_MAX, 0, NULL))
#define bauma_nson_getUlong(pNode, pPath) ((long)bauma_nson_getUint_ext((pNode), (pPath), 0, ULONG_MAX, 0, NULL))
#ifdef ULLONG_MAX
	#define bauma_nson_getUlongLong(pNode, pPath) ((unsigned long long)bauma_nson_getUint_ext((pNode), (pPath), 0, ULLONG_MAX, 0, NULL))
#endif

BAUMA_NSON_DEF double bauma_nson_getDouble_ext(bauma_NsonNode *pNode, const char *pPath, double minValue, double maxValue, double defaultWhenNotFound, bauma_bool_t *pOptOutSuccess);
#define bauma_nson_getFloat(pNode, pPath) ((float)bauma_nson_getDouble_ext((pNode), (pPath), -FLT_MAX, FLT_MAX, 0.0, NULL))
#define bauma_nson_getDouble(pNode, pPath) bauma_nson_getDouble_ext((pNode), (pPath), -DBL_MAX, DBL_MAX, 0.0, NULL)

BAUMA_NSON_DEF bauma_bool_t bauma_nson_getString_ext(bauma_StringBuilder *pDst, bauma_NsonNode *pNode, const char *pPath, const char *pDefaultWhenNotFound);
#define bauma_nson_getString(pNode, pPath) bauma_nson_getString_ext((*pDst), (pNode), (pPath), "")

BAUMA_NSON_DEF void bauma_nson_toString_ext(bauma_StringBuilder *pDst, bauma_NsonNode *pNode, bauma_NsonDialect dialect, bauma_bool_t pretty, const char *pIndent);
#define bauma_nson_toString(pDst, pNode) bauma_nson_toString_ext((pDst), (pNode), BAUMA_NSON_DIALECT_JSON, BAUMA_TRUE, "\t")

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BAUMA_NSON_IMPLEMENTATION

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#ifdef bauma_nson_custom_assert
	#define bauma_nson_assert(x) bauma_nson_custom_assert(x)
#else
	#include <assert.h>
	#define bauma_nson_assert(x) assert(x)
#endif


#ifdef __cplusplus
	extern "C" {
#endif

#define BAUMA_NSON_NUMBER_PARSE_BUFFER_SIZE 64u

#define bauma_nson_set_true(pBool) \
	do { \
		if ((pBool) != NULL) *pBool = BAUMA_TRUE; \
	} while(0)

#define bauma_nson_set_false(pBool) \
	do { \
		if ((pBool) != NULL) *pBool = BAUMA_FALSE; \
	} while(0)

BAUMA_NSON_DEF void bauma_nson_NodePtr_destruct(void *ppNode) {
	if (ppNode != NULL) {
		bauma_NsonNode* p = *(bauma_NsonNode**)ppNode;
		bauma_nson_delete(p);
	}
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_new_ext(bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p;
	bauma_nson_assert(pAlloc != NULL);
	p = (bauma_NsonNode*)(*pAlloc->pRealloc)(pAlloc, NULL, sizeof(bauma_NsonNode), NULL);
	memset(p, 0, sizeof(*p));
	p->pAlloc = pAlloc;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newNull_ext(bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_NULL;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newBool_ext(bauma_bool_t value, bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_BOOL;
	p->value.b = value;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newSigned_ext(bauma_intmax_t value, bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_SIGNED;
	p->value.si = value;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newUnsigned_ext(bauma_uintmax_t value, bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_UNSIGNED;
	p->value.ui = value;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newDouble_ext(double value, bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_DOUBLE;
	p->value.d = value;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newStr_ext(const char *pValue, bauma_bool_t xferOwnership, bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_STRING;
	if (xferOwnership) {
		p->value.p = (char*)pValue;
	}
	else {
		p->value.p = bauma_strdup_ext(pValue, pAlloc);
	}
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newStrWithLen_ext(const char *pValue, size_t valueLen, bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_STRING;
	p->value.p = bauma_strdupn_ext(pValue, valueLen, pAlloc);
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newArray_ext(bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_ARRAY;
	bauma_Vector_construct_ext(&p->value.v, bauma_NsonNode*, &bauma_nson_NodePtr_destruct, pAlloc);
	return p;
}

BAUMA_NSON_DEF void bauma_nson_array_append(bauma_NsonNode *pNode, bauma_NsonNode *pArrayElement) {
	bauma_nson_assert(pNode != NULL);
	bauma_nson_assert(pArrayElement != NULL);
	bauma_nson_assert(pNode->type == BAUMA_NSON_NODE_TYPE_ARRAY);
	bauma_Vector_append(&pNode->value.v, bauma_NsonNode*, &pArrayElement);
}

BAUMA_NSON_DEF void bauma_nson_object_append(bauma_NsonNode* pNode, bauma_NsonNode *pKey, bauma_NsonNode *pValue) {
	bauma_nson_assert(pNode != NULL);
	bauma_nson_assert(pKey != NULL);
	bauma_nson_assert(pValue != NULL);
	bauma_nson_assert(pNode->type == BAUMA_NSON_NODE_TYPE_OBJECT);
	if (pKey->pObjectValue != NULL) {
		bauma_nson_delete(pKey->pObjectValue);
	}
	pKey->pObjectValue = pValue;
	bauma_Vector_append(&pNode->value.v, bauma_NsonNode*, &pKey);
}


BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newObject_ext(bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_OBJECT;
	bauma_Vector_construct_ext(&p->value.v, bauma_NsonNode*, &bauma_nson_NodePtr_destruct, pAlloc);
	return p;
}

BAUMA_NSON_DEF void bauma_nson_delete(bauma_NsonNode *pNode) {
	if (pNode == NULL) {
		return;
	}
	bauma_nson_assert(pNode->pAlloc != NULL);
	if (pNode->pObjectValue != NULL) {
		bauma_nson_delete(pNode->pObjectValue);
	}
	switch(pNode->type) {
		case BAUMA_NSON_NODE_TYPE_STRING: {
			(*pNode->pAlloc->pRealloc)(pNode->pAlloc, pNode->value.p, 0, NULL);
			break;
		}
		case BAUMA_NSON_NODE_TYPE_ARRAY: /* fall through */
		case BAUMA_NSON_NODE_TYPE_OBJECT: {
			bauma_Vector_destruct(&pNode->value.v);
			break;
		}
		default: break;
	}
	(*pNode->pAlloc->pRealloc)(pNode->pAlloc, pNode, 0, NULL);
}

BAUMA_NSON_DEF void bauma_nson_skip(bauma_nson_StringWithLength *pStrWithLen) {
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
			pEnd = (const char*)bauma_memmem(pStrWithLen->pStr + 2,
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

BAUMA_NSON_DEF void bauma_nson_fmtErr(bauma_StringBuilder *pErrFormatter, const char* pMsg, bauma_nson_StringWithLength *pStrWithLen) {
	if (pErrFormatter != NULL) {
		bauma_StringBuilder_appendStr(pErrFormatter, pMsg);
		bauma_StringBuilder_appendStr(pErrFormatter, " (offset ");
		bauma_StringBuilder_appendUnsigned(pErrFormatter, (bauma_uintmax_t)(pStrWithLen->pStr - pStrWithLen->pBegin));
		bauma_StringBuilder_appendStr(pErrFormatter, ")");
	}
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parse_array(bauma_nson_StringWithLength *pStrWithLen,
                                                      bauma_StringBuilder *pErrFormatter,
                                                      bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *pArray = bauma_nson_newArray_ext(pAlloc);
	/* skip the '[' */
	++pStrWithLen->pStr;
	--pStrWithLen->len;
	for(;;) {
		bauma_NsonNode *pArrayElement;
		bauma_nson_skip(pStrWithLen);
		if (pStrWithLen->len == 0) {
			bauma_nson_delete(pArray);
			bauma_nson_fmtErr(pErrFormatter, "Error parsing array: end of data reached", pStrWithLen);
			return NULL;
		}
		char c = *pStrWithLen->pStr;
		if (c == ']') { /* end of array */
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			return pArray;
		}
		pArrayElement = bauma_nson_parse_ext(pStrWithLen, pErrFormatter, pAlloc);
		if (pArrayElement == NULL) {
			bauma_nson_delete(pArray);
			return NULL;
		}
		bauma_nson_array_append(pArray, pArrayElement);
	}
	return NULL; /* unreachable */
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parse_object(bauma_nson_StringWithLength *pStrWithLen,
                                                       bauma_StringBuilder *pErrFormatter,
                                                       bauma_IMemAllocator *pAlloc) {
	bauma_NsonNode *pObject = bauma_nson_newObject_ext(pAlloc);
	/* skip the '[' */
	++pStrWithLen->pStr;
	--pStrWithLen->len;
	for(;;) {
		bauma_NsonNode *pKeyElement;
		bauma_NsonNode *pValueElement;
		bauma_nson_skip(pStrWithLen);
		if (pStrWithLen->len == 0) {
			bauma_nson_delete(pObject);
			bauma_nson_fmtErr(pErrFormatter, "Error parsing object: end of data reached before key", pStrWithLen);
			return NULL;
		}
		char c = *pStrWithLen->pStr;
		if (c == '}') { /* end of object */
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			return pObject;
		}
		pKeyElement = bauma_nson_parse_ext(pStrWithLen, pErrFormatter, pAlloc);
		if (pKeyElement == NULL) {
			bauma_nson_delete(pObject);
			return NULL;
		}
		pValueElement = bauma_nson_parse_ext(pStrWithLen, pErrFormatter, pAlloc);
		if (pValueElement == NULL) {
			bauma_nson_delete(pKeyElement);
			bauma_nson_delete(pObject);
			return NULL;
		}
		bauma_nson_object_append(pObject, pKeyElement, pValueElement);
	}
	return NULL; /* unreachable */
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parse_string(bauma_nson_StringWithLength *pStrWithLen,
                                                       bauma_StringBuilder *pErrFormatter,
                                                       bauma_IMemAllocator *pAlloc) {
	/* skip the '"' */
	bauma_NsonNode *pResult = NULL;
	bauma_StringBuilder b;
	/* skip the '"' */
	++pStrWithLen->pStr;
	--pStrWithLen->len;
	bauma_StringBuilder_construct_ext(&b, pAlloc);
	while (pStrWithLen->len > 0) {
		char c = pStrWithLen->pStr[0];
		switch(c) {
		case '"': {
			char *pStr = bauma_StringBuilder_release(&b);
			pResult = bauma_nson_newStr_ext(pStr, BAUMA_TRUE, pAlloc);
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			goto out;
		}
		case '\\': {
			char c2; 
			if (pStrWithLen->len < 2u) {
				bauma_nson_fmtErr(pErrFormatter, "Error parsing string: escape sequence too short", pStrWithLen);
				goto out;
			}
			c2 = pStrWithLen->pStr[1];
			switch(c2) {
			case '"':
			case '\\':
			case '/': {
				bauma_StringBuilder_appendChar(&b, c2, 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'b': {
				bauma_StringBuilder_appendChar(&b, '\b', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'f': {
				bauma_StringBuilder_appendChar(&b, '\f', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'n': {
				bauma_StringBuilder_appendChar(&b, '\n', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'r': {
				bauma_StringBuilder_appendChar(&b, '\r', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 't': {
				bauma_StringBuilder_appendChar(&b, '\t', 1u);
				pStrWithLen->pStr += 2;
				pStrWithLen->len -= 2u;
				break;
			}
			case 'u': {
				char buf[5];
				unsigned long hex;
				if (pStrWithLen->len < 6u) {
					bauma_nson_fmtErr(pErrFormatter, "Error parsing string: unicode escape sequence too short", pStrWithLen);
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
					bauma_nson_fmtErr(pErrFormatter, "Error parsing string: unicode escape sequence invalid", pStrWithLen);
					goto out;
				}
				bauma_StringBuilder_appendCodePointUtf8(&b, hex);
				pStrWithLen->pStr += 6;
				pStrWithLen->len -= 6u;
				break;
			}
			default: {
				bauma_nson_fmtErr(pErrFormatter, "Error parsing string: wrong escape sequence", pStrWithLen);
				goto out;
			}
			}
			break;
		}
		default: {
			bauma_StringBuilder_appendChar(&b, c, 1u);
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			break;
		}
		}
	}
out:
	bauma_StringBuilder_destruct(&b);
	return pResult;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parse_token(bauma_nson_StringWithLength *pStrWithLen,
                                                      bauma_StringBuilder *pErrFormatter,
                                                      bauma_IMemAllocator *pAlloc) {
	const char *p = pStrWithLen->pStr;
	size_t len = 0;
	size_t i;
	bauma_bool_t containsDot = BAUMA_FALSE;
	while(pStrWithLen->len > 0) {
		char c = *pStrWithLen->pStr;
		if ((c == ':') || (c == ',') || (c == '[') || (c == ']') ||
		    (c == '{') || (c == '}') || isspace(c)) break;
		if (c == '.') containsDot = BAUMA_TRUE;
		++len;
		++pStrWithLen->pStr;
		--pStrWithLen->len;
	}
	if (len == 0) {
		bauma_nson_fmtErr(pErrFormatter, "Error parsing token: length is zero", pStrWithLen);
		return NULL;
	}
	if (((p[0] >= '0') && (p[0] <= '9')) || (p[0] == '.') || (p[0] == '+') || (p[0] == '-')) {
		/* expect token to be an integer of floating point number */
		char parseBuf[BAUMA_NSON_NUMBER_PARSE_BUFFER_SIZE];
		char *pParseBuf; /* A null terminated array to have reliable stdlib number parsing */
		if (len < sizeof(parseBuf)) { /* fits, inclusive '\0' terminator */
			pParseBuf = parseBuf; /* avoid dyn. mem alloc in most cases */
		}
		else {
			pParseBuf = (char*)(*pAlloc->pRealloc)(pAlloc, NULL, len+1, NULL);
		}
		#define NSON_FREE_PARSE_BUF() \
			do { \
				if (pParseBuf != parseBuf) { \
					(*pAlloc->pRealloc)(pAlloc, pParseBuf, 0, NULL); \
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
				bauma_nson_fmtErr(pErrFormatter, "Error parsing double: value out of range", pStrWithLen);
				return NULL;
			}
			if (value == HUGE_VAL) {
				NSON_FREE_PARSE_BUF();
				bauma_nson_fmtErr(pErrFormatter, "Error parsing double: value out of range", pStrWithLen);
				return NULL;
			}
			NSON_FREE_PARSE_BUF();
			return bauma_nson_newDouble_ext(value, pAlloc);
		}
		if (p[0] == '-') {
			bauma_intmax_t value;
			#ifdef LLONG_MAX
				value = (bauma_intmax_t)strtoll(pParseBuf, NULL, 10);
				if (((value == LLONG_MAX) || (value == LLONG_MIN) || (value == 0)) && (errno == ERANGE)) {
					NSON_FREE_PARSE_BUF();
					bauma_nson_fmtErr(pErrFormatter, "Error parsing signed integer: value out of range", pStrWithLen);
					return NULL;
				}
			#else
				value = (bauma_intmax_t)strtol(pParseBuf, NULL, 10);
				if (((value == LONG_MAX) || (value == LONG_MIN) || (value == 0)) && (errno == ERANGE)) {
					NSON_FREE_PARSE_BUF();
					bauma_nson_fmtErr(pErrFormatter, "Error parsing signed integer: value out of range", pStrWithLen);
					return NULL;
				}
			#endif
			NSON_FREE_PARSE_BUF();
			return bauma_nson_newSigned_ext(value, pAlloc);
		}
		bauma_uintmax_t value;
		#ifdef ULLONG_MAX
			value = (bauma_uintmax_t)strtoull(pParseBuf, NULL, 10);
			if (((value == ULLONG_MAX) || (value == 0)) && (errno == ERANGE)) {
				NSON_FREE_PARSE_BUF();
				bauma_nson_fmtErr(pErrFormatter, "Error parsing unsigned integer: value out of range", pStrWithLen);
				return NULL;
			}
		#else
			value = (bauma_uintmax_t)strtoul(pParseBuf, NULL, 10);
			if (((value == ULONG_MAX) || (value == 0)) && (errno == ERANGE)) {
				NSON_FREE_PARSE_BUF();
				bauma_nson_fmtErr(pErrFormatter, "Error parsing unsigned integer: value out of range", pStrWithLen);
				return NULL;
			}
		#endif
		NSON_FREE_PARSE_BUF();
		return bauma_nson_newUnsigned_ext(value, pAlloc);
		#undef NSON_FREE_PARSE_BUF
	}
	if ((len == 4u) && (memcmp(p, "null", 4u) == 0)) {
		return bauma_nson_newNull_ext(pAlloc);
	}
	if ((len == 4u) && (memcmp(p, "true", 4u) == 0)) {
		return bauma_nson_newBool_ext(BAUMA_TRUE, pAlloc);
	}
	if ((len == 5u) && (memcmp(p, "false", 5u) == 0)) {
		return bauma_nson_newBool_ext(BAUMA_FALSE, pAlloc);
	}
	if (((p[0] >= 'a') && (p[0] <= 'z')) || ((p[0] >= 'A') && (p[0] <= 'Z')) || (p[0] == '_'))
	{
		for (i=1u; i<len; ++i) {
			if (((p[i] >= 'a') && (p[i] <= 'z')) || ((p[i] >= 'A') && (p[i] <= 'Z')) ||
                            ((p[i] >= '0') && (p[i] <= '9')) || (p[i] == '_')) {
				/* good */
			}
			else {
				bauma_nson_fmtErr(pErrFormatter, "Error parsing token: invalid", pStrWithLen);
				return NULL;
			}
		}
		return bauma_nson_newStrWithLen_ext(p, len, pAlloc);
	}
	bauma_nson_fmtErr(pErrFormatter, "Error parsing token: invalid", pStrWithLen);
	return NULL;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parse_ext(bauma_nson_StringWithLength *pStrWithLen,
                                                    bauma_StringBuilder *pErrFormatter,
                                                    bauma_IMemAllocator *pAlloc) {
	char c;
	bauma_nson_assert(pStrWithLen != NULL);
	bauma_nson_assert(pAlloc != NULL);
	bauma_nson_skip(pStrWithLen);
	if (pStrWithLen->len == 0) return NULL;
	c = *pStrWithLen->pStr;
	switch(c) {
		case '[': return bauma_nson_parse_array(pStrWithLen, pErrFormatter, pAlloc);
		case '{': return bauma_nson_parse_object(pStrWithLen, pErrFormatter, pAlloc);
		case '"': return bauma_nson_parse_string(pStrWithLen, pErrFormatter, pAlloc);
		default: return bauma_nson_parse_token(pStrWithLen, pErrFormatter, pAlloc);
	}
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parseMem(const void *pMem, size_t len) {
	bauma_nson_StringWithLength strWithLen;
	strWithLen.pBegin = (const char*)pMem;
	strWithLen.pStr = (const char*)pMem;
	strWithLen.len = len;
	return bauma_nson_parse_ext(&strWithLen, NULL, bauma_getDefaultMemAllocator());
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parseStr(const char *pStr) {
	bauma_nson_StringWithLength strWithLen;
	strWithLen.pBegin = pStr;
	strWithLen.pStr = pStr;
	strWithLen.len = strlen(pStr);
	return bauma_nson_parse_ext(&strWithLen, NULL, bauma_getDefaultMemAllocator());
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_get(bauma_NsonNode *pNode, const char *pPath) {
	size_t pathLen;
	bauma_nson_assert(pNode != NULL);
	bauma_nson_assert(pPath != NULL);
	pathLen = strlen(pPath);
	if (pathLen == 0) return pNode;
	while (pathLen > 0) {
		if (*pPath == '[') { /* array indexing */
			char tmp[BAUMA_UINTMAX_MAX_DECIMAL_LENGTH+1]; /* for strtoul(l) to have null term. buf */
			size_t num;
			bauma_uintmax_t idx;
			const char* pEnd = strchr(pPath+1, ']');
			if (pEnd == 0) return NULL;
			num = (size_t)(pEnd - (pPath+1));
			if (num > BAUMA_UINTMAX_MAX_DECIMAL_LENGTH) return NULL;
			memcpy(tmp, pPath+1, num);
			tmp[num] = '\0';
			errno = 0;
#ifdef ULLONG_MAX
			idx = strtoull(tmp, NULL, 10);
#else
			idx = strtoul(tmp, NULL, 10);
#endif
			if (errno != 0) return NULL;
			if (idx > SIZE_MAX) return NULL;
			if (pNode->type != BAUMA_NSON_NODE_TYPE_ARRAY) return NULL;
			if (idx > bauma_Vector_getSize(&pNode->value.v)) return NULL;
			pNode = *bauma_Vector_at(&pNode->value.v, (size_t)idx, bauma_NsonNode*);
			num = (size_t)((pEnd+1) - pPath);
			pPath += num;
			pathLen -= num;
		}
		else { /* find object element by key string */
			size_t num;
			size_t i;
			bauma_NsonNode *pSubNode;
			bauma_bool_t found = BAUMA_FALSE;
			const char* pEnd = strchr(pPath, '/');
			if (pEnd == NULL) pEnd = pPath + pathLen;
			num = (size_t)(pEnd - pPath);
			if (num == 0) return NULL;
			if (pNode->type != BAUMA_NSON_NODE_TYPE_OBJECT) return NULL;
			for (i=0; i<bauma_Vector_getSize(&pNode->value.v); ++i) {
				pSubNode = *bauma_Vector_at(&pNode->value.v, i, bauma_NsonNode*);
				if (pSubNode->type != BAUMA_NSON_NODE_TYPE_STRING) return NULL;
				bauma_assert(pSubNode->value.p != NULL);
				if (strlen(pSubNode->value.p) != num) continue;
				if (memcmp(pSubNode->value.p, pPath, num) == 0) {
					bauma_nson_assert(pSubNode->pObjectValue != NULL);
					pNode = pSubNode->pObjectValue;
					found = BAUMA_TRUE;
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

BAUMA_NSON_DEF bauma_bool_t bauma_nson_getBool_ext(bauma_NsonNode *pNode, const char *pPath, bauma_bool_t defaultWhenNotFound, bauma_bool_t *pOptOutSuccess) {
	bauma_NsonNode *pSubNode;
	bauma_nson_assert(pNode != NULL);
	bauma_nson_set_false(pOptOutSuccess);
	pSubNode = bauma_nson_get(pNode, pPath);
	if (pSubNode == NULL) {
		return defaultWhenNotFound;
	}
	if (pSubNode->type != BAUMA_NSON_NODE_TYPE_BOOL) {
		return defaultWhenNotFound;
	}
	bauma_nson_set_true(pOptOutSuccess);
	return pSubNode->value.b;
}

BAUMA_NSON_DEF bauma_intmax_t bauma_nson_getInt_ext(bauma_NsonNode *pNode, const char *pPath, bauma_intmax_t minValue, bauma_intmax_t maxValue, bauma_intmax_t defaultWhenNotFound, bauma_bool_t *pOptOutSuccess) {
	bauma_NsonNode *pSubNode;
	bauma_intmax_t result;
	bauma_nson_assert(pNode != NULL);
	bauma_nson_set_false(pOptOutSuccess);
	pSubNode = bauma_nson_get(pNode, pPath);
	if (pSubNode == NULL) return defaultWhenNotFound;
	switch(pSubNode->type) {
		case BAUMA_NSON_NODE_TYPE_BOOL: result = (pNode->value.b) ? 1 : 0; break;
		case BAUMA_NSON_NODE_TYPE_SIGNED: result = pNode->value.si; break;
		case BAUMA_NSON_NODE_TYPE_UNSIGNED: {
			if (pNode->value.ui > (bauma_intmax_t)BAUMA_INTMAX_MAX) return defaultWhenNotFound;
			result = (bauma_intmax_t)pNode->value.ui;
			break;
		}
		case BAUMA_NSON_NODE_TYPE_DOUBLE: {
			result = (bauma_intmax_t)pNode->value.d;
			if (((double)result) != pNode->value.d) return defaultWhenNotFound;
			break;
		}
		default: return defaultWhenNotFound;
	}
	if ((result < minValue) || (result > maxValue)) return defaultWhenNotFound;
	bauma_nson_set_true(pOptOutSuccess);
	return result;
}

BAUMA_NSON_DEF bauma_uintmax_t bauma_nson_getUint_ext(bauma_NsonNode *pNode, const char *pPath, bauma_uintmax_t minValue, bauma_uintmax_t maxValue, bauma_uintmax_t defaultWhenNotFound, bauma_bool_t *pOptOutSuccess) {
	bauma_NsonNode *pSubNode;
	bauma_uintmax_t result;
	bauma_nson_assert(pNode != NULL);
	bauma_nson_set_false(pOptOutSuccess);
	pSubNode = bauma_nson_get(pNode, pPath);
	if (pSubNode == NULL) return defaultWhenNotFound;
	switch(pSubNode->type) {
		case BAUMA_NSON_NODE_TYPE_BOOL: result = (pNode->value.b) ? 1u : 0u; break;
		case BAUMA_NSON_NODE_TYPE_SIGNED: {
			if (pNode->value.si < 0) return defaultWhenNotFound;
			result = (bauma_uintmax_t)pNode->value.si;
			break;
		}
		case BAUMA_NSON_NODE_TYPE_UNSIGNED: {
			result = pNode->value.ui;
			break;
		}
		case BAUMA_NSON_NODE_TYPE_DOUBLE: {
			result = (bauma_uintmax_t)pNode->value.d;
			if (((double)result) != pNode->value.d) return defaultWhenNotFound;
			break;
		}
		default: return defaultWhenNotFound;
	}
	if ((result < minValue) || (result > maxValue)) return defaultWhenNotFound;
	bauma_nson_set_true(pOptOutSuccess);
	return result;
}

BAUMA_NSON_DEF double bauma_nson_getDouble_ext(bauma_NsonNode *pNode, const char *pPath, double minValue, double maxValue, double defaultWhenNotFound, bauma_bool_t *pOptOutSuccess) {
	bauma_NsonNode *pSubNode;
	double result;
	bauma_nson_assert(pNode != NULL);
	bauma_nson_set_false(pOptOutSuccess);
	pSubNode = bauma_nson_get(pNode, pPath);
	if (pSubNode == NULL) return defaultWhenNotFound;
	switch(pSubNode->type) {
		case BAUMA_NSON_NODE_TYPE_BOOL: result = (pNode->value.b) ? 1.0 : 0.0; break;
		case BAUMA_NSON_NODE_TYPE_SIGNED: {
			result = (double)pNode->value.si;
			if ((bauma_intmax_t)result != pNode->value.si) return defaultWhenNotFound;
			break;
		}
		case BAUMA_NSON_NODE_TYPE_UNSIGNED: {
			result = (double)pNode->value.ui;
			if ((bauma_uintmax_t)result != pNode->value.ui) return defaultWhenNotFound;
			break;
		}
		case BAUMA_NSON_NODE_TYPE_DOUBLE: {
			result = pNode->value.d;
			break;
		}
		default: return defaultWhenNotFound;
	}
	if ((result < minValue) || (result > maxValue)) return defaultWhenNotFound;
	bauma_nson_set_true(pOptOutSuccess);
	return result;
}

BAUMA_NSON_DEF bauma_bool_t bauma_nson_getString_ext(bauma_StringBuilder *pDst, bauma_NsonNode *pNode, const char *pPath, const char *pDefaultWhenNotFound) {
	bauma_NsonNode *pSubNode;
	bauma_nson_assert(pDst != NULL);
	bauma_nson_assert(pNode != NULL);
	pSubNode = bauma_nson_get(pNode, pPath);
	if (pSubNode == NULL) {
		bauma_StringBuilder_appendStr(pDst, pDefaultWhenNotFound);
		return BAUMA_FALSE;
	}
	switch(pSubNode->type) {
		case BAUMA_NSON_NODE_TYPE_BOOL:     bauma_StringBuilder_appendBool(pDst, pNode->value.b);      break;
		case BAUMA_NSON_NODE_TYPE_SIGNED:   bauma_StringBuilder_appendSigned(pDst, pNode->value.si);   break;
		case BAUMA_NSON_NODE_TYPE_UNSIGNED: bauma_StringBuilder_appendUnsigned(pDst, pNode->value.ui); break;
		case BAUMA_NSON_NODE_TYPE_DOUBLE:   bauma_StringBuilder_appendDouble(pDst, pNode->value.d);    break;
		case BAUMA_NSON_NODE_TYPE_STRING:   bauma_StringBuilder_appendStr(pDst, pNode->value.p);       break;
		default: {
			bauma_StringBuilder_appendStr(pDst, pDefaultWhenNotFound);
			return BAUMA_FALSE;
		}
	}
	return BAUMA_TRUE;
}

BAUMA_NSON_DEF void bauma_nson_putIndent(bauma_StringBuilder *pDst, const char *pIndent, size_t indentSize, size_t indentLevel) {
	size_t i;
	bauma_nson_assert(pDst != NULL);
	bauma_nson_assert(pIndent != NULL);
	for (i=0; i<indentLevel; ++i) {
		bauma_StringBuilder_appendStrWithLen(pDst, pIndent, indentSize);
	}
}

/*! Checks if the string could be printed without escapes or double quotes in NSON */
BAUMA_NSON_DEF bauma_bool_t bauma_nson_isSimpleToken(const char *pStr, size_t len) {
	if (len == 0) return BAUMA_FALSE;
	if ((len == 4u) && (memcmp(pStr, "null",  4u) == 0)) return BAUMA_FALSE;
	if ((len == 4u) && (memcmp(pStr, "true",  4u) == 0)) return BAUMA_FALSE;
	if ((len == 5u) && (memcmp(pStr, "false", 5u) == 0)) return BAUMA_FALSE;
	if (((*pStr >= 'a') && (*pStr <= 'z')) ||
	    ((*pStr >= 'A') && (*pStr <= 'Z')) ||
            (*pStr == '_')) {
		/* good: initial char must not be a number */
	}
	else {
		return BAUMA_FALSE;
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
			return BAUMA_FALSE;
		}
		++pStr;
		--len;
	}
	return BAUMA_TRUE;
}

BAUMA_NSON_DEF void bauma_jsonEscape(bauma_StringBuilder *pDst, const char *pStr, size_t len) {
	bauma_StringBuilder_appendChar(pDst, '"', 1u);
	while (len > 0) {
		switch (*pStr) {
		case '"':  bauma_StringBuilder_appendStrWithLen(pDst, "\\\"", 2u); break;
		case '\\': bauma_StringBuilder_appendStrWithLen(pDst, "\\\\", 2u); break;
		case '\b': bauma_StringBuilder_appendStrWithLen(pDst, "\\b",  2u); break;
		case '\f': bauma_StringBuilder_appendStrWithLen(pDst, "\\f",  2u); break;
		case '\n': bauma_StringBuilder_appendStrWithLen(pDst, "\\n",  2u); break;
		case '\r': bauma_StringBuilder_appendStrWithLen(pDst, "\\r",  2u); break;
		case '\t': bauma_StringBuilder_appendStrWithLen(pDst, "\\t",  2u); break;
		default: {
			if (((unsigned char)*pStr <= 0x1F) || ((unsigned char)*pStr == 0x7F)) {
				/* control chars */
				char buf[7];
				size_t n;
#if BAUMA_MODERN_C
				n = (size_t)snprintf(buf, sizeof(buf), "\\u%04X", (int)((unsigned char)*pStr));
#else
				n = (size_t)sprintf(buf, "\\u%04X", (int)((unsigned char)*pStr));
#endif
				bauma_nson_assert(n == 6u);
				bauma_StringBuilder_appendStrWithLen(pDst, buf, n);
			}
			else {
				bauma_StringBuilder_appendChar(pDst, *pStr, 1u);
			}
			break;
		}
		}
		++pStr;
		--len;
	}
	bauma_StringBuilder_appendChar(pDst, '"', 1u);
}

BAUMA_NSON_DEF void bauma_nson_toString_impl(bauma_StringBuilder *pDst, bauma_NsonNode *pNode, bauma_NsonDialect dialect, bauma_bool_t pretty, bauma_bool_t initialIndent, const char *pIndent, size_t indentSize, size_t indentLevel) {
	bauma_nson_assert(pDst != NULL);
	bauma_nson_assert(pNode != NULL);
	bauma_nson_assert(pIndent != NULL);
	if (pretty && initialIndent) {
		bauma_nson_putIndent(pDst, pIndent, indentSize, indentLevel);
	}
	switch(pNode->type) {
		case BAUMA_NSON_NODE_TYPE_NULL:     bauma_StringBuilder_appendStrWithLen(pDst, "null", 4u);    break;
		case BAUMA_NSON_NODE_TYPE_BOOL:     bauma_StringBuilder_appendBool(pDst, pNode->value.b);      break;
		case BAUMA_NSON_NODE_TYPE_SIGNED:   bauma_StringBuilder_appendSigned(pDst, pNode->value.si);   break;
		case BAUMA_NSON_NODE_TYPE_UNSIGNED: bauma_StringBuilder_appendUnsigned(pDst, pNode->value.ui); break;
		case BAUMA_NSON_NODE_TYPE_DOUBLE:   bauma_StringBuilder_appendDouble(pDst, pNode->value.d);    break;
		case BAUMA_NSON_NODE_TYPE_STRING: {
			const char *p = pNode->value.p;
			size_t len = strlen(pNode->value.p);
			if ((dialect == BAUMA_NSON_DIALECT_NSON) && bauma_nson_isSimpleToken(p, len)) {
				bauma_StringBuilder_appendStrWithLen(pDst, p, len);
			}
			else {
				bauma_jsonEscape(pDst, p, len);
			}
			break;
		}
		case BAUMA_NSON_NODE_TYPE_ARRAY: {
			size_t i, size;
			bauma_NsonNode *pSubNode;
			bauma_StringBuilder_appendChar(pDst, '[', 1u);
			if (pretty) {
				bauma_StringBuilder_appendChar(pDst, '\n', 1u);
			}
			++indentLevel;
			size = bauma_Vector_getSize(&pNode->value.v);
			for (i=0; i<size; ++i) {
				pSubNode = *bauma_Vector_at(&pNode->value.v, i, bauma_NsonNode*);
				bauma_nson_toString_impl(pDst, pSubNode, dialect, pretty, BAUMA_TRUE, pIndent, indentSize, indentLevel);
				if (((dialect == BAUMA_NSON_DIALECT_NSON) && (size > 1)) || ((i+1) < size)) {
					bauma_StringBuilder_appendChar(pDst, ',', 1u);
				}
				if (pretty) {
					bauma_StringBuilder_appendChar(pDst, '\n', 1u);
				}
			}
			--indentLevel;
			if (pretty) {
				bauma_nson_putIndent(pDst, pIndent, indentSize, indentLevel);
			}
			bauma_StringBuilder_appendChar(pDst, ']', 1u);
			break;
		}
		case BAUMA_NSON_NODE_TYPE_OBJECT: {
			size_t i, size;
			bauma_NsonNode *pKeyNode, *pValueNode;
			bauma_StringBuilder_appendChar(pDst, '{', 1u);
			if (pretty) {
				bauma_StringBuilder_appendChar(pDst, '\n', 1u);
			}
			++indentLevel;
			size = bauma_Vector_getSize(&pNode->value.v);
			for (i=0; i<size; ++i) {
				pKeyNode = *bauma_Vector_at(&pNode->value.v, i, bauma_NsonNode*);
				pValueNode = pKeyNode->pObjectValue;
				bauma_nson_toString_impl(pDst, pKeyNode, dialect, pretty, BAUMA_TRUE, pIndent, indentSize, indentLevel);
				bauma_StringBuilder_appendChar(pDst, ':', 1u);
				if (pretty) {
					bauma_StringBuilder_appendChar(pDst, ' ', 1u);
				}
				bauma_nson_toString_impl(pDst, pValueNode, dialect, pretty, BAUMA_FALSE, pIndent, indentSize, indentLevel);
				if (((dialect == BAUMA_NSON_DIALECT_NSON) && (size > 1)) || ((i+1) < size)) {
					bauma_StringBuilder_appendChar(pDst, ',', 1u);
				}
				if (pretty) {
					bauma_StringBuilder_appendChar(pDst, '\n', 1u);
				}
			}
			--indentLevel;
			if (pretty) {
				bauma_nson_putIndent(pDst, pIndent, indentSize, indentLevel);
			}
			bauma_StringBuilder_appendChar(pDst, '}', 1u);
			break;
		}
		default: break;
	}
}

BAUMA_NSON_DEF void bauma_nson_toString_ext(bauma_StringBuilder *pDst, bauma_NsonNode *pNode, bauma_NsonDialect dialect, bauma_bool_t pretty, const char *pIndent) {
	bauma_nson_assert(pDst != NULL);
	bauma_nson_assert(pNode != NULL);
	bauma_nson_assert(pIndent != NULL);
	bauma_nson_toString_impl(pDst, pNode, dialect, pretty, BAUMA_TRUE, pIndent, strlen(pIndent), 0);
	if (pretty) {
		bauma_StringBuilder_appendChar(pDst, '\n', 1u);
	}
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BAUMA_NSON_IMPLEMENTATION */

#ifdef BAUMA_NSON_TEST

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

void test_new(void) {
	bauma_NsonNode *pNode;
	bauma_NsonNode *pSubNode;
	pNode = bauma_nson_new();
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_INVALID);
	BAUMA_EXPECT(pNode->pObjectValue == NULL);
	BAUMA_EXPECT(pNode->pAlloc == bauma_getDefaultMemAllocator());
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newNull();
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_NULL);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newBool(BAUMA_FALSE);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_BOOL);
	BAUMA_EXPECT(!pNode->value.b);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newBool(BAUMA_TRUE);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_BOOL);
	BAUMA_EXPECT(pNode->value.b);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newSigned(BAUMA_INTMAX_MIN);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_SIGNED);
	BAUMA_EXPECT(pNode->value.si == BAUMA_INTMAX_MIN);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newSigned(BAUMA_INTMAX_MAX);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_SIGNED);
	BAUMA_EXPECT(pNode->value.si == BAUMA_INTMAX_MAX);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newUnsigned(0u);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_UNSIGNED);
	BAUMA_EXPECT(pNode->value.ui == 0u);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newUnsigned(BAUMA_UINTMAX_MAX);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_UNSIGNED);
	BAUMA_EXPECT(pNode->value.ui == BAUMA_UINTMAX_MAX);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newDouble(3.14);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_DOUBLE);
	BAUMA_EXPECT(fabs(pNode->value.d - 3.14) < 0.001);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newStr("Hello");
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pNode->value.p, "Hello") == 0);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newStrWithLen("Hello", 3u);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pNode->value.p, "Hel") == 0);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newArray();
	bauma_nson_array_append(pNode, bauma_nson_newStr("Hello"));
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_ARRAY);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_newObject();
	bauma_nson_object_append(pNode, bauma_nson_newStr("Key"), bauma_nson_newStr("Hello"));
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_OBJECT);
	pSubNode = *bauma_Vector_at(&pNode->value.v, 0, bauma_NsonNode*);
	BAUMA_EXPECT(strcmp(pSubNode->value.p, "Key") == 0);
	BAUMA_EXPECT(pSubNode->pObjectValue != NULL);
	BAUMA_EXPECT(strcmp(pSubNode->pObjectValue->value.p, "Hello") == 0);
	bauma_nson_delete(pNode);
}

void test_parse(void) {
	bauma_NsonNode *pNode;
	pNode = bauma_nson_parseStr("null");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_NULL);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("true");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_BOOL);
	BAUMA_EXPECT(pNode->value.b == BAUMA_TRUE);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("false");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_BOOL);
	BAUMA_EXPECT(pNode->value.b == BAUMA_FALSE);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("4711");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_UNSIGNED);
	BAUMA_EXPECT(pNode->value.ui == 4711);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("-4711");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_SIGNED);
	BAUMA_EXPECT(pNode->value.si == -4711);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("3.14");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_DOUBLE);
	BAUMA_EXPECT(fabs(pNode->value.d - 3.14) <= 0.001);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("Hello");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pNode->value.p, "Hello") == 0);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("[]");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_ARRAY);
	BAUMA_EXPECT(bauma_Vector_getSize(&pNode->value.v) == 0);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("[1]");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_ARRAY);
	BAUMA_EXPECT(bauma_Vector_getSize(&pNode->value.v) == 1);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("[1 Hello]");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_ARRAY);
	BAUMA_EXPECT(bauma_Vector_getSize(&pNode->value.v) == 2);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("[1,Hello,]");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_ARRAY);
	BAUMA_EXPECT(bauma_Vector_getSize(&pNode->value.v) == 2);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("{A:1,B:2}");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_OBJECT);
	BAUMA_EXPECT(bauma_Vector_getSize(&pNode->value.v) == 2);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("{A 1 B 2}");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_OBJECT);
	BAUMA_EXPECT(bauma_Vector_getSize(&pNode->value.v) == 2);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr(" // comment \n[1 /* comment */ 2] // comment");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_ARRAY);
	BAUMA_EXPECT(bauma_Vector_getSize(&pNode->value.v) == 2);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("\"Hello\"");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pNode->value.p, "Hello") == 0);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("\"\\r\\n\"");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pNode->value.p, "\r\n") == 0);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("\"\\\"\"");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pNode->value.p, "\"") == 0);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("\"\\u0001\"");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pNode->value.p, "\x01") == 0);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("\"H\\u00E4llo\"");
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pNode->value.p, "H\xC3\xA4llo") == 0);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr(
		"{\n"
		"	name: Anybody,\n"
		"	age: 42, // comment\n"
		"	hobbies: [reading, writing, swimming,]\n"
		"}\n"
	);
	BAUMA_EXPECT(pNode != NULL);
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_OBJECT);
	bauma_nson_delete(pNode);
}

void test_get(void) {
	bauma_NsonNode *pNode, *pSubNode;
	pNode = bauma_nson_parseStr(
		"{\n"
		"	name: Anybody,\n"
		"	age: 42, // comment\n"
		"	hobbies: [reading, writing, swimming,]\n"
		"}\n"
	);
	BAUMA_EXPECT(pNode != NULL);
	pSubNode = bauma_nson_get(pNode, "name");
	BAUMA_EXPECT(pSubNode != NULL);
	BAUMA_EXPECT(pSubNode->type = BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pSubNode->value.p, "Anybody") == 0);
	pSubNode = bauma_nson_get(pNode, "age");
	BAUMA_EXPECT(pSubNode != NULL);
	BAUMA_EXPECT(pSubNode->type = BAUMA_NSON_NODE_TYPE_UNSIGNED);
	BAUMA_EXPECT(pSubNode->value.ui == 42u);
	pSubNode = bauma_nson_get(pNode, "hobbies/[2]");
	BAUMA_EXPECT(pSubNode != NULL);
	BAUMA_EXPECT(pSubNode->type = BAUMA_NSON_NODE_TYPE_STRING);
	BAUMA_EXPECT(strcmp(pSubNode->value.p, "swimming") == 0);
	bauma_nson_delete(pNode);
}

void test_toString(void) {
	/*
	BAUMA_NSON_DEF void bauma_nson_toString_ext(bauma_StringBuilder *pDst, bauma_NsonNode *pNode, bauma_NsonDialect dialect, bauma_bool_t pretty, const char *pIndent) {
	*/
	bauma_StringBuilder b;
	bauma_NsonNode *pNode;
	bauma_StringBuilder_construct(&b);
	pNode = bauma_nson_parseStr("null");
	BAUMA_EXPECT(pNode != NULL);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_JSON, BAUMA_FALSE, "");
	bauma_nson_delete(pNode);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "null") == 0);
	bauma_StringBuilder_clear(&b);
	pNode = bauma_nson_parseStr("true");
	BAUMA_EXPECT(pNode != NULL);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_JSON, BAUMA_FALSE, "");
	bauma_nson_delete(pNode);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "true") == 0);
	bauma_StringBuilder_clear(&b);
	pNode = bauma_nson_parseStr("false");
	BAUMA_EXPECT(pNode != NULL);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_JSON, BAUMA_FALSE, "");
	bauma_nson_delete(pNode);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "false") == 0);
	bauma_StringBuilder_clear(&b);
	pNode = bauma_nson_parseStr("0");
	BAUMA_EXPECT(pNode != NULL);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_JSON, BAUMA_FALSE, "");
	bauma_nson_delete(pNode);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "0") == 0);
	bauma_StringBuilder_clear(&b);
	pNode = bauma_nson_parseStr("\"Hello\"");
	BAUMA_EXPECT(pNode != NULL);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_JSON, BAUMA_FALSE, "");
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "\"Hello\"") == 0);
	bauma_StringBuilder_clear(&b);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_NSON, BAUMA_TRUE, "");
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "Hello\n") == 0);
	bauma_nson_delete(pNode);
	bauma_StringBuilder_clear(&b);
	pNode = bauma_nson_parseStr("[]");
	BAUMA_EXPECT(pNode != NULL);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_JSON, BAUMA_FALSE, "");
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "[]") == 0);
	bauma_StringBuilder_clear(&b);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_JSON, BAUMA_TRUE, "\t");
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "[\n]\n") == 0);
	bauma_StringBuilder_clear(&b);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("[1]");
	BAUMA_EXPECT(pNode != NULL);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_JSON, BAUMA_FALSE, "");
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "[1]") == 0);
	bauma_StringBuilder_clear(&b);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_JSON, BAUMA_TRUE, "\t");
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "[\n\t1\n]\n") == 0);
	bauma_StringBuilder_clear(&b);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("[1,2]");
	BAUMA_EXPECT(pNode != NULL);
	bauma_nson_toString(&b, pNode);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "[\n\t1,\n\t2\n]\n") == 0);
	bauma_StringBuilder_clear(&b);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_NSON, BAUMA_TRUE, "\t");
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b), "[\n\t1,\n\t2,\n]\n") == 0);
	bauma_StringBuilder_clear(&b);
	bauma_nson_delete(pNode);
	pNode = bauma_nson_parseStr("{key:1,value:2}");
	BAUMA_EXPECT(pNode != NULL);
	bauma_nson_toString(&b, pNode);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b),
		"{\n"
		"	\"key\": 1,\n"
		"	\"value\": 2\n"
		"}\n"
	) == 0);
	bauma_StringBuilder_clear(&b);
	bauma_nson_toString_ext(&b, pNode, BAUMA_NSON_DIALECT_NSON, BAUMA_TRUE, "");
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&b),
		"{\n"
		"key: 1,\n"
		"value: 2,\n"
		"}\n"
	) == 0);
	bauma_StringBuilder_clear(&b);
	bauma_nson_delete(pNode);
	bauma_StringBuilder_destruct(&b);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BAUMA_TEST(test_new);
	BAUMA_TEST(test_parse);
	BAUMA_TEST(test_get);
	BAUMA_TEST(test_toString);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BAUMA_NSON_TEST */

#endif /* BAUMA_NSON_H_INCLUDED */
