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

#include <bauma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef BAUMA_NSON_DEF
	#define BAUMA_NSON_DEF
#endif

typedef struct bauma_nson_StringWithLength {
	const char *pStr;
	size_t len;
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

typedef union bauma_NsonNodeUnion {
	bauma_bool_t    b;
	bauma_intmax_t  si;
	bauma_uintmax_t ui;
	double          d;
	char            *p;
	bauma_Vector    v; /* for maps and arrays, this points to all values */
} bauma_NsonNodeUnion;

typedef struct bauma_NsonNode {
	bauma_NsonNodeType    type;
	bauma_NsonNodeUnion   value;
	char                  *pKey; /* Key string for map items, else NULL */
	bauma_IMemAllocator*  pAlloc;
} bauma_NsonNode;

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_new_ext(bauma_IMemAllocator* pAlloc);
#define bauma_nson_new() bauma_nson_new_ext(bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newNull_ext(bauma_IMemAllocator* pAlloc);
#define bauma_nson_newNull() bauma_nson_newNull_ext(bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newBool_ext(bauma_bool_t value, bauma_IMemAllocator* pAlloc);
#define bauma_nson_newBool(value) bauma_nson_newBool_ext((value), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newSigned_ext(bauma_intmax_t value, bauma_IMemAllocator* pAlloc);
#define bauma_nson_newSigned(value) bauma_nson_newSigned_ext((value), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newUnsigned_ext(bauma_uintmax_t value, bauma_IMemAllocator* pAlloc);
#define bauma_nson_newUnsigned(value) bauma_nson_newUnsigned_ext((value), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newDouble_ext(double value, bauma_IMemAllocator* pAlloc);
#define bauma_nson_newDouble(value) bauma_nson_newDouble_ext((value), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newStr_ext(const char *pValue, bauma_bool_t xferOwnership, bauma_IMemAllocator* pAlloc);
#define bauma_nson_newStr(value) bauma_nson_newStr_ext((value), BAUMA_FALSE, bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newStrWithLen_ext(const char *pValue, size_t valueLen, bauma_IMemAllocator* pAlloc);
#define bauma_nson_newStrWithLen(value, len) bauma_nson_newStrWithLen_ext((value), (len), bauma_getDefaultMemAllocator())

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newArray_ext(bauma_IMemAllocator* pAlloc);
#define bauma_nson_newArray() bauma_nson_newArray_ext(bauma_getDefaultMemAllocator())
BAUMA_NSON_DEF void bauma_nson_array_append(bauma_NsonNode* pNode, bauma_NsonNode* pArrayElement);

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newObject_ext(bauma_IMemAllocator* pAlloc);
#define bauma_nson_newObject() bauma_nson_newObject_ext(bauma_getDefaultMemAllocator())
BAUMA_NSON_DEF void bauma_nson_object_append(bauma_NsonNode* pNode, const char *pKey, bauma_bool_t xferKeyOwnership, bauma_NsonNode *pValue);

BAUMA_NSON_DEF void bauma_nson_delete(bauma_NsonNode *pNode);

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parse_ext(bauma_nson_StringWithLength *pStrWithLen,
                                               bauma_StringBuilder *pErrFormatter,
                                               bauma_IMemAllocator *pAlloc);

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parseMem(const void *pMem, size_t len);
BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parseStr(const char *pStr);


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

static void bauma_nson_NodePtr_destruct(void *ppNode) {
	if (ppNode != NULL) {
		bauma_NsonNode* p = *(bauma_NsonNode**)ppNode;
		bauma_nson_delete(p);
	}
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_new_ext(bauma_IMemAllocator* pAlloc) {
	bauma_NsonNode *p;
	bauma_nson_assert(pAlloc != NULL);
	p = (bauma_NsonNode*)(*pAlloc->pRealloc)(pAlloc, NULL, sizeof(bauma_NsonNode), NULL);
	memset(p, 0, sizeof(*p));
	p->pAlloc = pAlloc;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newNull_ext(bauma_IMemAllocator* pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_NULL;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newBool_ext(bauma_bool_t value, bauma_IMemAllocator* pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_BOOL;
	p->value.b = value;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newSigned_ext(bauma_intmax_t value, bauma_IMemAllocator* pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_SIGNED;
	p->value.si = value;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newUnsigned_ext(bauma_uintmax_t value, bauma_IMemAllocator* pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_UNSIGNED;
	p->value.ui = value;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newDouble_ext(double value, bauma_IMemAllocator* pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_DOUBLE;
	p->value.d = value;
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newStr_ext(const char *pValue, bauma_bool_t xferOwnership, bauma_IMemAllocator* pAlloc) {
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

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newStrWithLen_ext(const char *pValue, size_t valueLen, bauma_IMemAllocator* pAlloc) {
	bauma_NsonNode *p = bauma_nson_new_ext(pAlloc);
	p->type = BAUMA_NSON_NODE_TYPE_STRING;
	p->value.p = bauma_strdupn_ext(pValue, valueLen, pAlloc);
	return p;
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newArray_ext(bauma_IMemAllocator* pAlloc) {
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

BAUMA_NSON_DEF void bauma_nson_object_append(bauma_NsonNode* pNode, const char *pKey, bauma_bool_t xferKeyOwnership, bauma_NsonNode *pValue) {
	bauma_nson_assert(pNode != NULL);
	bauma_nson_assert(pKey != NULL);
	bauma_nson_assert(pValue != NULL);
	bauma_nson_assert(pNode->type == BAUMA_NSON_NODE_TYPE_OBJECT);
	if (pValue->pKey != NULL) {
		(*pValue->pAlloc->pRealloc)(pValue->pAlloc, pValue->pKey, 0, NULL);
		pValue->pKey = NULL;
	}
	if (xferKeyOwnership) {
		pValue->pKey = (char*)pKey;
	}
	else {
		pValue->pKey = bauma_strdup_ext(pKey, pValue->pAlloc);
	}
	bauma_Vector_append(&pNode->value.v, bauma_NsonNode*, &pValue);
}


BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_newObject_ext(bauma_IMemAllocator* pAlloc) {
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
	if (pNode->pKey != NULL) {
		(*pNode->pAlloc->pRealloc)(pNode->pAlloc, pNode->pKey, 0, NULL);
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

static void bauma_nson_skip(bauma_nson_StringWithLength *pStrWithLen) {
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
			pEnd = strstr(pStrWithLen->pStr+2, "*/");
			if (pEnd == NULL) return;
			pEnd += 2u;
			numSkip = pEnd - pStrWithLen->pStr;
			pStrWithLen->pStr += numSkip;
			pStrWithLen->len -= numSkip;
		}
		else if (((c == '/' ) && (c2 == '/')) || (c == '#')) {
			const char* pNl;
			size_t numSkip;
			pNl = (const char*)memchr(pStrWithLen->pStr+2, '\n', pStrWithLen->len - 2u);
			if (pNl == NULL) {
				pNl = (const char*)memchr(pStrWithLen->pStr+2, '\r', pStrWithLen->len - 2u);
			}
			if (pNl == NULL) {
				return;
			}
			numSkip = pNl - pStrWithLen->pStr;
			pStrWithLen->pStr += numSkip;
			pStrWithLen->len -= numSkip;
		}
		else {
			break;
		}
	}
}

static bauma_NsonNode *bauma_nson_parse_array(bauma_nson_StringWithLength *pStrWithLen,
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
			return NULL;
		}
		char c = *pStrWithLen->pStr;
		if (c == ']') { /* end of array */
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			return pArray;
		}
		bauma_nson_skip(pStrWithLen);
		pArrayElement = bauma_nson_parse_ext(pStrWithLen, pErrFormatter, pAlloc);
		if (pArrayElement == NULL) {
			bauma_nson_delete(pArray);
			return NULL;
		}
		bauma_nson_array_append(pArray, pArrayElement);
	}
	return NULL; /* unreachable */
}

static bauma_NsonNode *bauma_nson_parse_object(bauma_nson_StringWithLength *pStrWithLen,
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
			return NULL;
		}
		char c = *pStrWithLen->pStr;
		if (c == '}') { /* end of object */
			++pStrWithLen->pStr;
			--pStrWithLen->len;
			return pObject;
		}
		bauma_nson_skip(pStrWithLen);
		pKeyElement = bauma_nson_parse_ext(pStrWithLen, pErrFormatter, pAlloc);
		if (pKeyElement == NULL) {
			bauma_nson_delete(pObject);
			return NULL;
		}
		if (pKeyElement->type != BAUMA_NSON_NODE_TYPE_STRING) {
			bauma_nson_delete(pKeyElement);
			bauma_nson_delete(pObject);
			return NULL;
		}
		bauma_nson_skip(pStrWithLen);
		if (pStrWithLen->len == 0) {
			bauma_nson_delete(pKeyElement);
			bauma_nson_delete(pObject);
			return NULL;
		}
		pValueElement = bauma_nson_parse_ext(pStrWithLen, pErrFormatter, pAlloc);
		if (pValueElement == NULL) {
			bauma_nson_delete(pKeyElement);
			bauma_nson_delete(pObject);
			return NULL;
		}
		bauma_nson_object_append(pObject, pKeyElement->value.p, BAUMA_TRUE, pValueElement);
		pKeyElement->value.p = NULL; /* ownership transfered */
		pKeyElement->type = BAUMA_NSON_NODE_TYPE_INVALID;
		bauma_nson_delete(pKeyElement);
	}
	return NULL; /* unreachable */
}

static bauma_NsonNode *bauma_nson_parse_string(bauma_nson_StringWithLength *pStrWithLen,
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
							goto out;
						}
						buf[0] = pStrWithLen->pStr[2];
						buf[1] = pStrWithLen->pStr[3];
						buf[2] = pStrWithLen->pStr[4];
						buf[3] = pStrWithLen->pStr[5];
						buf[4] = '\0';
						errno = 0;
						hex = strtoul(buf, NULL, 16);
						if ((hex == 0) && (errno == ERANGE)) goto out;
						bauma_StringBuilder_appendCodePointUtf8(&b, hex);
						pStrWithLen->pStr += 6;
						pStrWithLen->len -= 6u;
						break;
					}
					default: goto out;
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

static bauma_NsonNode *bauma_nson_parse_token(bauma_nson_StringWithLength *pStrWithLen,
                                              bauma_StringBuilder *pErrFormatter,
                                              bauma_IMemAllocator *pAlloc) {
	const char *p = pStrWithLen->pStr;
	size_t len = 0;
	size_t i;
	bauma_bool_t containsDot = BAUMA_FALSE;
	while(pStrWithLen->len > 0) {
		char c = *pStrWithLen->pStr;
		if ((c == ':') || (c == ',') || (c == '[') || (c == ']') || (c == '{') || (c == '}') || isspace(c)) break;
		if (c == '.') containsDot = BAUMA_TRUE;
		++len;
		++pStrWithLen->pStr;
		--pStrWithLen->len;
	}
	if (len == 0) {
		return NULL;
	}
	if (((p[0] >= '0') && (p[0] <= '9')) || (p[0] == '.') || (p[0] == '+') || (p[0] == '-')) {
		/* expect token to be an integer of floating point number */
		errno = 0;
		if (containsDot) { /* parse as double */
			double value;
			value = strtod(p, NULL);
			if ((value == 0.0) && (errno == ERANGE)) {
				return NULL;
			}
			if (value == HUGE_VAL) {
				return NULL;
			}
			return bauma_nson_newDouble_ext(value, pAlloc);
		}
		if (p[0] == '-') {
			bauma_intmax_t value;
			#ifdef LLONG_MAX
				value = (bauma_intmax_t)strtoll(p, NULL, 10);
				if (((value == LLONG_MAX) || (value == LLONG_MIN) || (value == 0)) && (errno == ERANGE)) {
					return NULL;
				}
			#else
				value = (bauma_intmax_t)strtol(p, NULL, 10);
				if (((value == LONG_MAX) || (value == LONG_MIN) || (value == 0)) && (errno == ERANGE)) {
					return NULL;
				}
			#endif
			return bauma_nson_newSigned_ext(value, pAlloc);
		}
		bauma_uintmax_t value;
		#ifdef ULLONG_MAX
			value = (bauma_uintmax_t)strtoull(p, NULL, 10);
			if (((value == ULLONG_MAX) || (value == 0)) && (errno == ERANGE)) {
				return NULL;
			}
		#else
			value = (bauma_uintmax_t)strtoul(p, NULL, 10);
			if (((value == ULONG_MAX) || (value == 0)) && (errno == ERANGE)) {
				return NULL;
			}
		#endif
		return bauma_nson_newUnsigned_ext(value, pAlloc);
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
				return NULL;
			}
		}
		return bauma_nson_newStrWithLen_ext(p, len, pAlloc);
	}
	return NULL;
}


BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parse_ext(bauma_nson_StringWithLength *pStrWithLen,
                                               bauma_StringBuilder *pErrFormatter,
                                               bauma_IMemAllocator *pAlloc) {
	char c;
	bauma_nson_assert(pStrWithLen != NULL);
	bauma_nson_assert(pAlloc != NULL);
	bauma_nson_assert(pStrWithLen->pStr[pStrWithLen->len] == '\0');
	bauma_nson_skip(pStrWithLen);
	if (pStrWithLen->len == 0) return NULL;
	if (pStrWithLen->pStr[pStrWithLen->len] != '\0') return NULL;
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
	strWithLen.pStr = (const char*)pMem;
	strWithLen.len = len;
	return bauma_nson_parse_ext(&strWithLen, NULL, bauma_getDefaultMemAllocator());
}

BAUMA_NSON_DEF bauma_NsonNode *bauma_nson_parseStr(const char *pStr) {
	bauma_nson_StringWithLength strWithLen;
	strWithLen.pStr = pStr;
	strWithLen.len = strlen(pStr);
	return bauma_nson_parse_ext(&strWithLen, NULL, bauma_getDefaultMemAllocator());
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
	BAUMA_EXPECT(pNode->pKey == NULL);
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
	bauma_nson_object_append(pNode, "Key", BAUMA_FALSE, bauma_nson_newStr("Hello"));
	BAUMA_EXPECT(pNode->type == BAUMA_NSON_NODE_TYPE_OBJECT);
	pSubNode = *bauma_Vector_at(&pNode->value.v, 0, bauma_NsonNode*);
	BAUMA_EXPECT(strcmp(pSubNode->pKey, "Key") == 0);
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
	pNode = bauma_nson_parseStr("[1 /* comment */ 2] // comment");
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
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BAUMA_TEST(test_new);
	BAUMA_TEST(test_parse);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BAUMA_NSON_TEST */

#endif /* BAUMA_NSON_H_INCLUDED */
