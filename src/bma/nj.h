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
- Strings that look like identifiers ([A-Za-z_][A-Za-z0-9_]*) and aren't "true", "false" or
  "null" don't need double quotes, so "Hello" can be also written Hello.

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

IMPORTANT: This parser only works in the "C" locale. When the program using it calls setlocale() to change the locale,
the parser will fail or give surprising results. So don't do that.
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

/*! \brief Create a new node in memory without specific type and value */
BMA_DEF bma_NjNode *bma_njNew_ext(bma_IMemAlloc *pAlloc);

/*! \brief Same as bma_njNew_ext, using the default memory allocator */
#define bma_njNew() bma_njNew_ext(bma_getDfltMemAlloc())

/*! \brief Creates a new null-type node */
BMA_DEF bma_NjNode *bma_njNewNull_ext(bma_IMemAlloc *pAlloc);

/*! \brief Creates a new null-type node using the default memory allocator */
#define bma_njNewNull() bma_njNewNull_ext(bma_getDfltMemAlloc())

/*! \brief Creates a new bool-type node (true or false) */
BMA_DEF bma_NjNode *bma_njNewBool_ext(bma_bool_t value, bma_IMemAlloc *pAlloc);

/*! \brief Same as bma_njNewBool_ext, using the default memory allocator */
#define bma_njNewBool(value) bma_njNewBool_ext((value), bma_getDfltMemAlloc())

/*! \brief Creates a new signed integer-type node */
BMA_DEF bma_NjNode *bma_njNewSigned_ext(bma_intmax_t value, bma_IMemAlloc *pAlloc);

/*! \brief Same as bma_njNewSigned_ext, using the default memory allocator */
#define bma_njNewSigned(value) bma_njNewSigned_ext((value), bma_getDfltMemAlloc())

/*! \brief Creates a new unsigned integer-type node */
BMA_DEF bma_NjNode *bma_njNewUnsigned_ext(bma_uintmax_t value, bma_IMemAlloc *pAlloc);

/*! \brief Same as bma_njNewUnsigned_ext, using the default memory allocator */
#define bma_njNewUnsigned(value) bma_njNewUnsigned_ext((value), bma_getDfltMemAlloc())

/*! \brief Creates a new double precision floating point type node */
BMA_DEF bma_NjNode *bma_njNewDouble_ext(double value, bma_IMemAlloc *pAlloc);

/*! \brief Same as bma_njNewDouble_ext, using the default memory allocator */
#define bma_njNewDouble(value) bma_njNewDouble_ext((value), bma_getDfltMemAlloc())

/*!
  \brief Creates a new string-type node
  \param pValue The string value. If xferOwnership is BMA_TRUE, the string is not copied but the pointer is taken over.
                In that case the allocator must match the one where the string was allocated from. If xferOwnership is
                BMA_FALSE, the string is copied into the node. In both cases, the node owns the string and will free
                its stored string when the node is deleted.
*/
BMA_DEF bma_NjNode *bma_njNewStr_ext(const char *pValue, bma_bool_t xferOwnership, bma_IMemAlloc *pAlloc);

/*! \brief Same as bma_njNewStr_ext, using the default memory allocator */
#define bma_njNewStr(value) bma_njNewStr_ext((value), BMA_FALSE, bma_getDfltMemAlloc())

/*! \brief Creates a new string-type node with a specified length. A copy of the string is stored. */
BMA_DEF bma_NjNode *bma_njNewStrWithLen_ext(const char *pValue, size_t valueLen, bma_IMemAlloc *pAlloc);

/*! \brief Same as bma_njNewStrWithLen_ext, using the default memory allocator */
#define bma_njNewStrWithLen(value, len) bma_njNewStrWithLen_ext((value), (len), bma_getDfltMemAlloc())

/*! \brief Creates a new array-type node. After this call, the array is initialized as empty (0 elements). */
BMA_DEF bma_NjNode *bma_njNewArr_ext(bma_IMemAlloc *pAlloc);

/*! \brief Same as bma_njNewArr_ext, using the default memory allocator */
#define bma_njNewArr() bma_njNewArr_ext(bma_getDfltMemAlloc())

/*! \brief Appends an element to an array-type node */
BMA_DEF void bma_njArrAppnd(bma_NjNode* pNode, bma_NjNode* pArrayElement);

/*! \brief Creates a new object-type (map, key-value storage) node */
BMA_DEF bma_NjNode *bma_njNewObj_ext(bma_IMemAlloc *pAlloc);

/*! \brief Same as bma_njNewObj_ext, using the default memory allocator */
#define bma_njNewObj() bma_njNewObj_ext(bma_getDfltMemAlloc())

/*!
  \brief Appends a key-value pair to an object-type node.
  IMPORTANT: The key node must be of type string. The implementation will only check that in debug mode.
  IMPORTANT: The implementation won't check for duplicate keye; the caller must ensure that
*/
BMA_DEF void bma_njObjAppnd(bma_NjNode* pNode, bma_NjNode *pKey, bma_NjNode *pValue);

/*! \brief Deletes a node and all its children; also the allocated values if there are any */
BMA_DEF void bma_njDelete(bma_NjNode *pNode);

/*!
  \brief Parses a document from representation in memory.
  \param pStrWithLen The string to parse, with length. The string doesn't need to be null-terminated.
                     For correct error output, both pointers pBegin and pStr must point to the same string,
                     and len must be the length of that string.
  \param pErrFormatter If not NULL, error messages will be appended to this string builder.
                       If NULL, no error messages are generated.
  \param pAlloc The memory allocator to use for allocating the node and its children.
                Use ccal_getDfltMemAlloc() to use the default allocator.
*/
BMA_DEF bma_NjNode *bma_njParseExt(bma_NjParseSrc *pStrWithLen,
                                   bma_StrBldr *pErrFormatter,
                                   bma_IMemAlloc *pAlloc);

/*! \brief Parses a document from a memory buffer with known length by calling bma_njParseExt */
BMA_DEF bma_NjNode *bma_njParseMem(const void *pMem, size_t len);

/*! \brief Parses a document from a null-terminated string using bma_njParseExt */
BMA_DEF bma_NjNode *bma_njParse(const char *pStr);

/*!
  \brief Looks-up a (sub-)node by using a small DSL to traverse the node tree.
  \param pNode The node to start the search from
  \param pPath The path to the node to look up.
               The path is a string of the form "key1/key2[3]/key4", where keys are object
               keys and numbers in brackets are array indices (0-based). Empty string means the root node itself.
*/
BMA_DEF bma_NjNode *bma_njGet(bma_NjNode *pNode, const char *pPath);

/*! \brief See bma_njGet but specialized for boolean values. */
BMA_DEF bma_bool_t bma_njGetBool_ext(bma_NjNode *pNode, const char *pPath, bma_bool_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess);

/*! \brief See bma_njGetBool but with a default values, meaning returning BMA_FALSE when the node is not found and not reporting errors */
#define bma_njGetBool(pNode, pPath) bma_njGetBool_ext((pNode), (pPath), BMA_FALSE, NULL)

/*! \brief See bma_njGet but specialized for integer values. By passing min/max as boundaries, any integer type
           can be retrieved. Below are the convenience macros for common integer types. */
BMA_DEF bma_intmax_t bma_njGetInt_ext(bma_NjNode *pNode, const char *pPath, bma_intmax_t minValue, bma_intmax_t maxValue, bma_intmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess);

/*! \brief See bma_njGetIntExt but specialized for short integer values and not reporting errors (0 is the fall-back for errors) */
#define bma_njGetShort(pNode, pPath) ((short)bma_njGetInt_ext((pNode), (pPath), SHRT_MIN, SHRT_MAX, 0, NULL))

/*! \brief See bma_njGetIntExt but specialized for integer values and not reporting errors (0 is the fall-back for errors) */
#define bma_njGetInt(pNode, pPath) ((int)bma_njGetInt_ext((pNode), (pPath), INT_MIN, INT_MAX, 0, NULL))

/*! \brief See bma_njGetIntExt but specialized for long integer values and not reporting errors (0 is the fall-back for errors) */
#define bma_njGetLong(pNode, pPath) ((long)bma_njGetInt_ext((pNode), (pPath), LONG_MIN, LONG_MAX, 0, NULL))
#ifdef LLONG_MAX
	/*! \brief See bma_njGetIntExt but specialized for long long integer values and not reporting errors (0 is the fall-back for errors) */
	#define bma_njGetLongLong(pNode, pPath) ((long long)bma_njGetInt_ext((pNode), (pPath), LLONG_MIN, LLONG_MAX, 0, NULL))
#endif

/*! \brief See bma_njGet but specialized for unsigned integer values. */
BMA_DEF bma_uintmax_t bma_njGetUint_ext(bma_NjNode *pNode, const char *pPath, bma_uintmax_t minValue, bma_uintmax_t maxValue, bma_uintmax_t defaultWhenNotFound, bma_bool_t *pOptOutSuccess);

/*! \brief See bma_njGetUintExt but specialized for unsigned short integer values and not reporting errors (0 is the fall-back for errors) */
#define bma_njGetUshort(pNode, pPath) ((unsigned short)bma_njGetUint_ext((pNode), (pPath), 0, USHRT_MAX, 0, NULL))

/*! \brief See bma_njGetUintExt but specialized for unsigned integer values and not reporting errors (0 is the fall-back for errors) */
#define bma_njGetUint(pNode, pPath) ((unsigned int)bma_njGetUint_ext((pNode), (pPath), 0, UINT_MAX, 0, NULL))

/*! \brief See bma_njGetUintExt but specialized for unsigned long integer values and not reporting errors (0 is the fall-back for errors) */
#define bma_njGetUlong(pNode, pPath) ((unsigned long)bma_njGetUint_ext((pNode), (pPath), 0, ULONG_MAX, 0, NULL))
#ifdef ULLONG_MAX
	/*! \brief See bma_njGetUintExt but specialized for unsigned long long integer values and not reporting errors (0 is the fall-back for errors) */
	#define bma_njGetUlongLong(pNode, pPath) ((unsigned long long)bma_njGetUint_ext((pNode), (pPath), 0, ULLONG_MAX, 0, NULL))
#endif

/*! \brief See bma_njGet but specialized for floating point values. */
BMA_DEF double bma_njGetDouble_ext(bma_NjNode *pNode, const char *pPath, double minValue, double maxValue, double defaultWhenNotFound, bma_bool_t *pOptOutSuccess);

/*! \brief See bma_njGetDoubleExt but specialized for float values and not reporting errors (0.0 is the fall-back for errors) */
#define bma_njGetFloat(pNode, pPath) ((float)bma_njGetDouble_ext((pNode), (pPath), -FLT_MAX, FLT_MAX, 0.0, NULL))

/*! \brief See bma_njGetDoubleExt but specialized for double values and not reporting errors (0.0 is the fall-back for errors) */
#define bma_njGetDouble(pNode, pPath) bma_njGetDouble_ext((pNode), (pPath), -DBL_MAX, DBL_MAX, 0.0, NULL)

/*! \brief See bma_njGet but specialized for string values. Strings are appended into the passed string builder. */
BMA_DEF bma_bool_t bma_njGetString_ext(bma_StrBldr *pDst, bma_NjNode *pNode, const char *pPath, const char *pDefaultWhenNotFound);

/*! \brief See bma_njGetStringExt but using an empty string as the default when the node isn't found */
#define bma_njGetString(pDst, pNode, pPath) bma_njGetString_ext((pDst), (pNode), (pPath), "")

/*! \brief Converts a tree of nodes into its string representation, eiter JSON or NJ dialect */
BMA_DEF void bma_njToStr_ext(bma_StrBldr *pDst, bma_NjNode *pNode, bma_NjDialect dialect, bma_bool_t pretty, const char *pIndent);

/*! \brief See bma_njToStrExt but using default parameters: Using JSON, pretty-printed with tab character as indentation */
#define bma_njToStr(pDst, pNode) bma_njToStr_ext((pDst), (pNode), BMA_NJ_DIALECT_JSON, BMA_TRUE, "\t")

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_NJ_IMPL

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#ifdef __cplusplus
	extern "C" {
#endif

#define BMA_NJ_NUM_PARSE_BUF_SZ 64u

/* Max. nesting of arrays/objects; protects against stack overflow on malicious input */
#ifndef BMA_NJ_MAX_DEPTH
	#define BMA_NJ_MAX_DEPTH 256u
#endif

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
	bma_assert(pKey->type == BMA_NJ_NODE_TYPE_STR);
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

BMA_DEF bma_NjNode *bma_njParseVal(bma_NjParseSrc *pStrWithLen,
                                   bma_StrBldr *pErrFormatter,
                                   bma_IMemAlloc *pAlloc,
                                   size_t depth);

BMA_DEF bma_NjNode *bma_njParseArr(bma_NjParseSrc *pStrWithLen,
                                       bma_StrBldr *pErrFormatter,
                                       bma_IMemAlloc *pAlloc,
                                       size_t depth) {
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
		pArrayElement = bma_njParseVal(pStrWithLen, pErrFormatter, pAlloc, depth + 1u);
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
                                       bma_IMemAlloc *pAlloc,
                                       size_t depth) {
	bma_NjNode *pObject = bma_njNewObj_ext(pAlloc);
	char c;
	/* skip the '{' */
	++pStrWithLen->pStr;
	--pStrWithLen->len;
	for(;;) {
		bma_NjNode *pKeyElement;
		bma_NjNode *pValueElement;
		bma_NjParseSrc keyPos;
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
		keyPos = *pStrWithLen;
		pKeyElement = bma_njParseVal(pStrWithLen, pErrFormatter, pAlloc, depth + 1u);
		if (pKeyElement == NULL) {
			bma_njDelete(pObject);
			return NULL;
		}
		if (pKeyElement->type != BMA_NJ_NODE_TYPE_STR) {
			bma_njDelete(pKeyElement);
			bma_njDelete(pObject);
			bma_njFmtErr(pErrFormatter, "Error parsing object: key must be a string", &keyPos);
			return NULL;
		}
		if (!bma_njSkip(pStrWithLen, pErrFormatter)) {
			bma_njDelete(pKeyElement);
			bma_njDelete(pObject);
			return NULL;
		}
		if ((pStrWithLen->len > 0) && (*pStrWithLen->pStr == '}')) {
			bma_njDelete(pKeyElement);
			bma_njDelete(pObject);
			bma_njFmtErr(pErrFormatter, "Error parsing object: missing value for key", pStrWithLen);
			return NULL;
		}
		pValueElement = bma_njParseVal(pStrWithLen, pErrFormatter, pAlloc, depth + 1u);
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
	bma_NjParseSrc tokPos = *pStrWithLen; /* error offsets point at the token start */
	size_t len = 0;
	size_t i;
	bma_bool_t containsDotOrE = BMA_FALSE;
	bma_bool_t containsX = BMA_FALSE;
	bma_uintmax_t uValue;
	char *pEnd;
	while(pStrWithLen->len > 0) {
		char c = *pStrWithLen->pStr;
		char c2 = (pStrWithLen->len > 1u) ? pStrWithLen->pStr[1] : '\0';
		if ((c == ':') || (c == ',') || (c == '[') || (c == ']') ||
		    (c == '{') || (c == '}') || (c == '"') || (c == '#') ||
		    ((c == '/') && ((c2 == '/') || (c2 == '*'))) ||
		    isspace((unsigned char)c)) break;
		if ((c == '.') || (c == 'e') || (c == 'E')) containsDotOrE = BMA_TRUE;
		if ((c == 'x') || (c == 'X')) containsX = BMA_TRUE;
		++len;
		++pStrWithLen->pStr;
		--pStrWithLen->len;
	}
	if (len == 0) {
		bma_njFmtErr(pErrFormatter, "Error parsing token: length is zero", &tokPos);
		return NULL;
	}
	if (((p[0] >= '0') && (p[0] <= '9')) || (p[0] == '.') || (p[0] == '+') || (p[0] == '-')) {
		/* expect token to be an integer of floating point number */
		char parseBuf[BMA_NJ_NUM_PARSE_BUF_SZ];
		char *pParseBuf; /* A null terminated array to have reliable stdlib number parsing */
		if (containsX) {
			bma_njFmtErr(pErrFormatter, "Error parsing number: hex numbers aren't supported", &tokPos);
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
				bma_njFmtErr(pErrFormatter, "Error parsing double: value out of range", &tokPos);
				return NULL;
			}
			if ((value == HUGE_VAL) || (value == -HUGE_VAL)) {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing double: value out of range", &tokPos);
				return NULL;
			}
			if (*pEnd != '\0') {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing double: invalid format", &tokPos);
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
					bma_njFmtErr(pErrFormatter, "Error parsing signed integer: value out of range", &tokPos);
					return NULL;
				}
			#else
				value = (bma_intmax_t)strtol(pParseBuf, &pEnd, 10);
				if (((value == LONG_MAX) || (value == LONG_MIN) || (value == 0)) && (errno == ERANGE)) {
					NJ_FREE_PARSE_BUF();
					bma_njFmtErr(pErrFormatter, "Error parsing signed integer: value out of range", &tokPos);
					return NULL;
				}
			#endif
			if (*pEnd != '\0') {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing signed integer: invalid format", &tokPos);
				return NULL;
			}
			NJ_FREE_PARSE_BUF();
			return bma_njNewSigned_ext(value, pAlloc);
		}
		#ifdef ULLONG_MAX
			uValue = (bma_uintmax_t)strtoull(pParseBuf, &pEnd, 10);
			if (((uValue == ULLONG_MAX) || (uValue == 0)) && (errno == ERANGE)) {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing unsigned integer: value out of range", &tokPos);
				return NULL;
			}
			if (*pEnd != '\0') {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing unsigned integer: invalid format", &tokPos);
				return NULL;
			}
		#else
			uValue = (bma_uintmax_t)strtoul(pParseBuf, &pEnd, 10);
			if (((uValue == ULONG_MAX) || (uValue == 0)) && (errno == ERANGE)) {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing unsigned integer: value out of range", &tokPos);
				return NULL;
			}
			if (*pEnd != '\0') {
				NJ_FREE_PARSE_BUF();
				bma_njFmtErr(pErrFormatter, "Error parsing unsigned integer: invalid format", &tokPos);
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
				bma_njFmtErr(pErrFormatter, "Error parsing token: invalid", &tokPos);
				return NULL;
			}
		}
		return bma_njNewStrWithLen_ext(p, len, pAlloc);
	}
	bma_njFmtErr(pErrFormatter, "Error parsing token: invalid", &tokPos);
	return NULL;
}

BMA_DEF bma_NjNode *bma_njParseVal(bma_NjParseSrc *pStrWithLen,
                                   bma_StrBldr *pErrFormatter,
                                   bma_IMemAlloc *pAlloc,
                                   size_t depth) {
	char c;
	bma_assert(pStrWithLen != NULL);
	bma_assert(pAlloc != NULL);
	if (!bma_njSkip(pStrWithLen, pErrFormatter)) return NULL;
	if (pStrWithLen->len == 0) {
		bma_njFmtErr(pErrFormatter, "Error parsing: unexpected end of data", pStrWithLen);
		return NULL;
	}
	c = *pStrWithLen->pStr;
	if (((c == '[') || (c == '{')) && (depth >= BMA_NJ_MAX_DEPTH)) {
		bma_njFmtErr(pErrFormatter, "Error parsing: nesting too deep", pStrWithLen);
		return NULL;
	}
	switch(c) {
		case '[': return bma_njParseArr(pStrWithLen, pErrFormatter, pAlloc, depth);
		case '{': return bma_njParseObj(pStrWithLen, pErrFormatter, pAlloc, depth);
		case '"': return bma_njParseStr(pStrWithLen, pErrFormatter, pAlloc);
		default: return bma_njParseTok(pStrWithLen, pErrFormatter, pAlloc);
	}
}

BMA_DEF bma_NjNode *bma_njParseExt(bma_NjParseSrc *pStrWithLen,
                                   bma_StrBldr *pErrFormatter,
                                   bma_IMemAlloc *pAlloc) {
	return bma_njParseVal(pStrWithLen, pErrFormatter, pAlloc, 0u);
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
			size_t i;
			bma_uintmax_t idx;
			const char* pEnd = strchr(pPath+1, ']');
			if (pEnd == 0) return NULL;
			num = (size_t)(pEnd - (pPath+1));
			if ((num == 0) || (num > BMA_UINTMAX_MAX_DEC_LEN)) return NULL;
			for (i=0; i<num; ++i) {
				if (!isdigit((unsigned char)pPath[1+i])) return NULL;
			}
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
				if (pSubNode->type != BMA_NJ_NODE_TYPE_STR) continue;
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
			if (result != result) return defaultWhenNotFound;
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
		case BMA_NJ_NODE_TYPE_DBL: {
			double d = pNode->value.d;
			/* JSON has no NaN/Inf */
			if ((d != d) || (d > DBL_MAX) || (d < -DBL_MAX)) {
				bma_StrBldr_appndStrN(pDst, "null", 4u);
			}
			else {
				bma_StrBldr_appndDbl(pDst, d);
			}
			break;
		}
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

void test_objKeys(void) {
	bma_NjNode *pNode, *pKey;
	bma_NjParseSrc src;
	bma_StrBldr err;
	BMA_EXPECT(bma_njParse("{1: a}") == NULL);
	BMA_EXPECT(bma_njParse("{-1: a}") == NULL);
	BMA_EXPECT(bma_njParse("{1.5: a}") == NULL);
	BMA_EXPECT(bma_njParse("{true: a}") == NULL);
	BMA_EXPECT(bma_njParse("{null: a}") == NULL);
	BMA_EXPECT(bma_njParse("{[1]: a}") == NULL);
	BMA_EXPECT(bma_njParse("{{}: a}") == NULL);
	BMA_EXPECT(bma_njParse("{a: 1, 2: b}") == NULL);
	pNode = bma_njParse("{a: 1, \"b c\": 2, \"1\": 3}");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(bma_njGetUint(pNode, "1") == 3u);
	bma_njDelete(pNode);
	/* error offset points at the key */
	bma_StrBldr_ctor(&err);
	src.pBegin = "{a 1  42 b}";
	src.pStr = src.pBegin;
	src.len = strlen(src.pBegin);
	BMA_EXPECT(bma_njParseRoot(&src, &err, bma_getDfltMemAlloc()) == NULL);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&err), "Error parsing object: key must be a string (offset 6)") == 0);
	bma_StrBldr_dtor(&err, NULL);
	/* bma_njGet skips non-string keys instead of aborting the search */
	pNode = bma_njNewObj();
	pKey = bma_njNewUnsigned(1u);
	pKey->pObjectValue = bma_njNewNull();
	bma_NjNodePtrVec_appnd(&pNode->value.v, &pKey);
	bma_njObjAppnd(pNode, bma_njNewStr("x"), bma_njNewUnsigned(7u));
	BMA_EXPECT(bma_njGetUint(pNode, "x") == 7u);
	bma_njDelete(pNode);
}

static char *test_makeNested(size_t depth, char open, char close) {
	char *p = (char*)malloc(2u * depth + 1u);
	memset(p, open, depth);
	memset(p + depth, close, depth);
	p[2u * depth] = '\0';
	return p;
}

void test_depth(void) {
	bma_NjNode *pNode;
	bma_NjParseSrc src;
	bma_StrBldr err;
	char *p;
	p = test_makeNested(BMA_NJ_MAX_DEPTH, '[', ']');
	pNode = bma_njParse(p);
	BMA_EXPECT(pNode != NULL);
	bma_njDelete(pNode);
	free(p);
	p = test_makeNested(BMA_NJ_MAX_DEPTH + 1u, '[', ']');
	BMA_EXPECT(bma_njParse(p) == NULL);
	bma_StrBldr_ctor(&err);
	src.pBegin = p;
	src.pStr = p;
	src.len = strlen(p);
	BMA_EXPECT(bma_njParseRoot(&src, &err, bma_getDfltMemAlloc()) == NULL);
	BMA_EXPECT(strstr(bma_StrBldr_getStr(&err), "nesting too deep") != NULL);
	bma_StrBldr_dtor(&err, NULL);
	free(p);
	/* far beyond any real stack limit */
	p = test_makeNested(1000000u, '[', ']');
	BMA_EXPECT(bma_njParse(p) == NULL);
	free(p);
	p = (char*)malloc(1000001u);
	memset(p, '{', 1000000u);
	p[1000000u] = '\0';
	BMA_EXPECT(bma_njParse(p) == NULL);
	free(p);
	/* objects count too: depth of values inside {a: ...} */
	pNode = bma_njParse("{a: {b: [1, {c: 2}]}}");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(bma_njGetUint(pNode, "a/b/[1]/c") == 2u);
	bma_njDelete(pNode);
}

void test_tokenBoundaries(void) {
	bma_NjNode *pNode;
	bma_NjParseSrc src;
	bma_StrBldr err;
	pNode = bma_njParse("[1//c\n2]");
	BMA_EXPECT((pNode != NULL) && (bma_NjNodePtrVec_getSz(&pNode->value.v) == 2));
	bma_njDelete(pNode);
	pNode = bma_njParse("[a/*c*/b]");
	BMA_EXPECT((pNode != NULL) && (bma_NjNodePtrVec_getSz(&pNode->value.v) == 2));
	bma_njDelete(pNode);
	pNode = bma_njParse("{a:1#c\n}");
	BMA_EXPECT((pNode != NULL) && (bma_njGetUint(pNode, "a") == 1u));
	bma_njDelete(pNode);
	pNode = bma_njParse("[a\"b\"]");
	BMA_EXPECT((pNode != NULL) && (bma_NjNodePtrVec_getSz(&pNode->value.v) == 2));
	bma_njDelete(pNode);
	pNode = bma_njParse("{a\"b\"}");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(strcmp(bma_njGet(pNode, "a")->value.p, "b") == 0);
	bma_njDelete(pNode);
	BMA_EXPECT(bma_njParse("[a/b]") == NULL);
	BMA_EXPECT(bma_njParse("{a}") == NULL);
	BMA_EXPECT(bma_njParse("{a 1 b}") == NULL);
	bma_StrBldr_ctor(&err);
	src.pBegin = "{a }";
	src.pStr = src.pBegin;
	src.len = strlen(src.pBegin);
	BMA_EXPECT(bma_njParseRoot(&src, &err, bma_getDfltMemAlloc()) == NULL);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&err), "Error parsing object: missing value for key (offset 3)") == 0);
	bma_StrBldr_dtor(&err, NULL);
}

static void test_expectErr(const char *pSrc, const char *pExpectedErr) {
	bma_NjParseSrc src;
	bma_StrBldr err;
	bma_StrBldr_ctor(&err);
	src.pBegin = pSrc;
	src.pStr = pSrc;
	src.len = strlen(pSrc);
	BMA_EXPECT(bma_njParseRoot(&src, &err, bma_getDfltMemAlloc()) == NULL);
	if (strcmp(bma_StrBldr_getStr(&err), pExpectedErr) != 0) {
		fprintf(stderr, "Got error '%s'\n", bma_StrBldr_getStr(&err));
	}
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&err), pExpectedErr) == 0);
	bma_StrBldr_dtor(&err, NULL);
}

void test_tokenErrOffsets(void) {
	test_expectErr("[1, 0x10]", "Error parsing number: hex numbers aren't supported (offset 4)");
	test_expectErr("[1, 1.2.3]", "Error parsing double: invalid format (offset 4)");
	test_expectErr("[1, 1e999]", "Error parsing double: value out of range (offset 4)");
	test_expectErr("[1, -12a]", "Error parsing signed integer: invalid format (offset 4)");
	test_expectErr("[1, 99999999999999999999999]", "Error parsing unsigned integer: value out of range (offset 4)");
	test_expectErr("[1, 12a]", "Error parsing unsigned integer: invalid format (offset 4)");
	test_expectErr("[1, ab-c]", "Error parsing token: invalid (offset 4)");
	test_expectErr("[1, $x]", "Error parsing token: invalid (offset 4)");
}

void test_stringErrors(void) {
	BMA_EXPECT(bma_njParse("\"\\q\"") == NULL);
	BMA_EXPECT(bma_njParse("\"\\u12G4\"") == NULL);
	BMA_EXPECT(bma_njParse("\"\\u0000\"") == NULL);
	BMA_EXPECT(bma_njParse("\"\\uD800\"") == NULL);
	BMA_EXPECT(bma_njParse("\"\\u12") == NULL);
	BMA_EXPECT(bma_njParse("\"unterminated") == NULL);
}

void test_numericBoundaries(void) {
	bma_NjNode *pNode;
	bma_StrBldr number;
	bma_StrBldr_ctor(&number);
	bma_StrBldr_appndUnsgnd(&number, BMA_UINTMAX_MAX);
	pNode = bma_njParse(bma_StrBldr_getStr(&number));
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_UNSGND);
	BMA_EXPECT(pNode->value.ui == BMA_UINTMAX_MAX);
	bma_njDelete(pNode);
	bma_StrBldr_clear(&number);
	bma_StrBldr_appndSgnd(&number, BMA_INTMAX_MIN);
	pNode = bma_njParse(bma_StrBldr_getStr(&number));
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->type == BMA_NJ_NODE_TYPE_SGND);
	BMA_EXPECT(pNode->value.si == BMA_INTMAX_MIN);
	bma_njDelete(pNode);
	bma_StrBldr_dtor(&number, NULL);
	pNode = bma_njParse("[-0, 1e3, -2.5e-2, 0.0]");
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(bma_NjNodePtrVec_getSz(&pNode->value.v) == 4u);
	BMA_EXPECT((*bma_NjNodePtrVec_at(&pNode->value.v, 0))->type == BMA_NJ_NODE_TYPE_SGND);
	BMA_EXPECT((*bma_NjNodePtrVec_at(&pNode->value.v, 0))->value.si == 0);
	BMA_EXPECT((*bma_NjNodePtrVec_at(&pNode->value.v, 1))->type == BMA_NJ_NODE_TYPE_DBL);
	BMA_EXPECT((*bma_NjNodePtrVec_at(&pNode->value.v, 1))->value.d == 1000.0);
	BMA_EXPECT(fabs((*bma_NjNodePtrVec_at(&pNode->value.v, 2))->value.d + 0.025) < 0.000001);
	bma_njDelete(pNode);
	BMA_EXPECT(bma_njParse("1e") == NULL);
	BMA_EXPECT(bma_njParse("--1") == NULL);
	BMA_EXPECT(bma_njParse("1.2.3") == NULL);
	BMA_EXPECT(bma_njParse("0x10") == NULL);
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

void test_getIndex(void) {
	bma_NjNode *pNode, *pSubNode;
	pNode = bma_njParse("{a: [x, y, [z]]}");
	BMA_EXPECT(pNode != NULL);
	pSubNode = bma_njGet(pNode, "a/[1]");
	BMA_EXPECT((pSubNode != NULL) && (strcmp(pSubNode->value.p, "y") == 0));
	pSubNode = bma_njGet(pNode, "a/[2]/[0]");
	BMA_EXPECT((pSubNode != NULL) && (strcmp(pSubNode->value.p, "z") == 0));
	pSubNode = bma_njGet(pNode, "a/[01]");
	BMA_EXPECT((pSubNode != NULL) && (strcmp(pSubNode->value.p, "y") == 0));
	BMA_EXPECT(bma_njGet(pNode, "a/[]") == NULL);
	BMA_EXPECT(bma_njGet(pNode, "a/[abc]") == NULL);
	BMA_EXPECT(bma_njGet(pNode, "a/[1x]") == NULL);
	BMA_EXPECT(bma_njGet(pNode, "a/[ 1]") == NULL);
	BMA_EXPECT(bma_njGet(pNode, "a/[1 ]") == NULL);
	BMA_EXPECT(bma_njGet(pNode, "a/[+1]") == NULL);
	BMA_EXPECT(bma_njGet(pNode, "a/[-1]") == NULL);
	BMA_EXPECT(bma_njGet(pNode, "a/[3]") == NULL);
	BMA_EXPECT(bma_njGet(pNode, "a/[1") == NULL);
	BMA_EXPECT(bma_njGet(pNode, "a/[99999999999999999999999]") == NULL);
	bma_njDelete(pNode);
}

void test_getters(void) {
	bma_NjNode *pNode = bma_njNewObj();
	bma_StrBldr result;
	bma_bool_t ok;
	bma_njObjAppnd(pNode, bma_njNewStr("yes"), bma_njNewBool(BMA_TRUE));
	bma_njObjAppnd(pNode, bma_njNewStr("no"), bma_njNewBool(BMA_FALSE));
	bma_njObjAppnd(pNode, bma_njNewStr("signed"), bma_njNewSigned(-7));
	bma_njObjAppnd(pNode, bma_njNewStr("unsigned"), bma_njNewUnsigned(8u));
	bma_njObjAppnd(pNode, bma_njNewStr("integerDouble"), bma_njNewDouble(3.0));
	bma_njObjAppnd(pNode, bma_njNewStr("fraction"), bma_njNewDouble(1.5));
	bma_njObjAppnd(pNode, bma_njNewStr("text"), bma_njNewStr("hello"));
	bma_njObjAppnd(pNode, bma_njNewStr("null"), bma_njNewNull());
	bma_njObjAppnd(pNode, bma_njNewStr("largest"), bma_njNewUnsigned(BMA_UINTMAX_MAX));

	BMA_EXPECT(bma_njGetBool_ext(pNode, "yes", BMA_FALSE, &ok) == BMA_TRUE);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetBool_ext(pNode, "no", BMA_TRUE, &ok) == BMA_FALSE);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetBool_ext(pNode, "missing", BMA_TRUE, &ok) == BMA_TRUE);
	BMA_EXPECT(!ok);
	BMA_EXPECT(bma_njGetBool_ext(pNode, "signed", BMA_TRUE, &ok) == BMA_TRUE);
	BMA_EXPECT(!ok);

	BMA_EXPECT(bma_njGetInt_ext(pNode, "yes", -10, 10, 99, &ok) == 1);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetInt_ext(pNode, "signed", -10, 10, 99, &ok) == -7);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetInt_ext(pNode, "unsigned", -10, 10, 99, &ok) == 8);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetInt_ext(pNode, "integerDouble", -10, 10, 99, &ok) == 3);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetInt_ext(pNode, "fraction", -10, 10, 99, &ok) == 99);
	BMA_EXPECT(!ok);
	BMA_EXPECT(bma_njGetInt_ext(pNode, "signed", -5, 5, 99, &ok) == 99);
	BMA_EXPECT(!ok);
	BMA_EXPECT(bma_njGetInt_ext(pNode, "largest", BMA_INTMAX_MIN, BMA_INTMAX_MAX, 99, &ok) == 99);
	BMA_EXPECT(!ok);

	BMA_EXPECT(bma_njGetUint_ext(pNode, "no", 0u, 10u, 99u, &ok) == 0u);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetUint_ext(pNode, "unsigned", 0u, 10u, 99u, &ok) == 8u);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetUint_ext(pNode, "integerDouble", 0u, 10u, 99u, &ok) == 3u);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetUint_ext(pNode, "signed", 0u, BMA_UINTMAX_MAX, 99u, &ok) == 99u);
	BMA_EXPECT(!ok);
	BMA_EXPECT(bma_njGetUint_ext(pNode, "fraction", 0u, BMA_UINTMAX_MAX, 99u, &ok) == 99u);
	BMA_EXPECT(!ok);
	BMA_EXPECT(bma_njGetUint_ext(pNode, "unsigned", 0u, 5u, 99u, &ok) == 99u);
	BMA_EXPECT(!ok);

	BMA_EXPECT(bma_njGetDouble_ext(pNode, "signed", -10.0, 10.0, 99.0, &ok) == -7.0);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetDouble_ext(pNode, "yes", -10.0, 10.0, 99.0, &ok) == 1.0);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetDouble_ext(pNode, "fraction", -10.0, 10.0, 99.0, &ok) == 1.5);
	BMA_EXPECT(ok);
	BMA_EXPECT(bma_njGetDouble_ext(pNode, "unsigned", 0.0, 5.0, 99.0, &ok) == 99.0);
	BMA_EXPECT(!ok);
	BMA_EXPECT(bma_njGetDouble_ext(pNode, "text", -10.0, 10.0, 99.0, &ok) == 99.0);
	BMA_EXPECT(!ok);

	bma_StrBldr_ctor(&result);
	BMA_EXPECT(bma_njGetString_ext(&result, pNode, "text", "fallback"));
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&result), "hello") == 0);
	bma_StrBldr_clear(&result);
	BMA_EXPECT(!bma_njGetString_ext(&result, pNode, "missing", "fallback"));
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&result), "fallback") == 0);
	bma_StrBldr_clear(&result);
	BMA_EXPECT(!bma_njGetString_ext(&result, pNode, "null", "fallback"));
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&result), "fallback") == 0);
	bma_StrBldr_dtor(&result, NULL);
	bma_njDelete(pNode);
}

typedef struct test_NjMemAlloc {
	bma_IMemAlloc iface;
	size_t numAlloc;
	size_t numFree;
} test_NjMemAlloc;

static void *test_NjMemAlloc_rllc(void *pSelf, void *pOld, size_t newSz, size_t *pOptRealSz) {
	test_NjMemAlloc *pAlloc = (test_NjMemAlloc*)pSelf;
	void *pResult;
	if (newSz == 0u) {
		++pAlloc->numFree;
		free(pOld);
		return NULL;
	}
	++pAlloc->numAlloc;
	pResult = realloc(pOld, newSz);
	if ((pResult != NULL) && (pOptRealSz != NULL)) *pOptRealSz = newSz;
	return pResult;
}

void test_stringOwnership(void) {
	test_NjMemAlloc alloc;
	bma_NjNode *pNode;
	char *pOwned;
	char source[] = "copy me";
	memset(&alloc, 0, sizeof(alloc));
	alloc.iface.pRllc = &test_NjMemAlloc_rllc;
	pOwned = (char*)test_NjMemAlloc_rllc(&alloc, NULL, sizeof("owned"), NULL);
	BMA_EXPECT(pOwned != NULL);
	memcpy(pOwned, "owned", sizeof("owned"));
	pNode = bma_njNewStr_ext(pOwned, BMA_TRUE, &alloc.iface);
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->value.p == pOwned);
	bma_njDelete(pNode);
	pNode = bma_njNewStr_ext(source, BMA_FALSE, &alloc.iface);
	BMA_EXPECT(pNode != NULL);
	BMA_EXPECT(pNode->value.p != source);
	BMA_EXPECT(strcmp(pNode->value.p, source) == 0);
	bma_njDelete(pNode);
	BMA_EXPECT(alloc.numAlloc == alloc.numFree);
}

void test_toStringNonFinite(void) {
	bma_StrBldr b;
	bma_NjNode *pNode;
	bma_bool_t ok;
	volatile double zero = 0.0;
	double dNan = zero / zero;
	double dInf = 1.0 / zero;
	bma_StrBldr_ctor(&b);
	pNode = bma_njNewArr();
	bma_njArrAppnd(pNode, bma_njNewDouble(dNan));
	bma_njArrAppnd(pNode, bma_njNewDouble(dInf));
	bma_njArrAppnd(pNode, bma_njNewDouble(-dInf));
	bma_njArrAppnd(pNode, bma_njNewDouble(1.5));
	bma_njArrAppnd(pNode, bma_njNewDouble(3.0));
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_FALSE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), "[null,null,null,1.5,3]") == 0);
	BMA_EXPECT(bma_njGetDouble_ext(pNode, "[0]", -DBL_MAX, DBL_MAX, 42.0, &ok) == 42.0);
	BMA_EXPECT(!ok);
	BMA_EXPECT(bma_njGetDouble_ext(pNode, "[1]", -DBL_MAX, DBL_MAX, 42.0, &ok) == 42.0);
	BMA_EXPECT(!ok);
	BMA_EXPECT(bma_njGetDouble_ext(pNode, "[3]", -DBL_MAX, DBL_MAX, 42.0, &ok) == 1.5);
	BMA_EXPECT(ok);
	bma_njDelete(pNode);
	bma_StrBldr_dtor(&b, NULL);
}

void test_stringEscaping(void) {
	const char value[] = "\"\\\b\f\n\r\t\x01\x7f";
	const char expected[] = "\"\\\"\\\\\\b\\f\\n\\r\\t\\u0001\\u007F\"";
	bma_StrBldr b;
	bma_NjNode *pNode;
	bma_NjNode *pRoundTrip;
	bma_StrBldr_ctor(&b);
	pNode = bma_njNewStr(value);
	bma_njToStr_ext(&b, pNode, BMA_NJ_DIALECT_JSON, BMA_FALSE, "");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&b), expected) == 0);
	pRoundTrip = bma_njParse(bma_StrBldr_getStr(&b));
	BMA_EXPECT(pRoundTrip != NULL);
	BMA_EXPECT(pRoundTrip->type == BMA_NJ_NODE_TYPE_STR);
	BMA_EXPECT(strcmp(pRoundTrip->value.p, value) == 0);
	bma_njDelete(pRoundTrip);
	bma_njDelete(pNode);
	bma_StrBldr_dtor(&b, NULL);
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
	BMA_TEST(test_objKeys);
	BMA_TEST(test_depth);
	BMA_TEST(test_tokenBoundaries);
	BMA_TEST(test_tokenErrOffsets);
	BMA_TEST(test_stringErrors);
	BMA_TEST(test_numericBoundaries);
	BMA_TEST(test_get);
	BMA_TEST(test_getIndex);
	BMA_TEST(test_getters);
	BMA_TEST(test_stringOwnership);
	BMA_TEST(test_toString);
	BMA_TEST(test_stringEscaping);
	BMA_TEST(test_toStringNonFinite);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_NJ_TEST */

#endif /* BMA_NJ_H_INCLUDED */
