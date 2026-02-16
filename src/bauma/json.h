#ifndef bauma_json_H_INCLUDED
#define bauma_json_H_INCLUDED

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

Bau 'ma's minimalistic JSON parser. Also allows some specialities to make JSON
more pleasant:

- Allows trailing commata in lists, such as [1,2,3,] without errors
- Allows C- and C++ style comments
- Allows object/map keys to be written without quotes as long as they don't contain white space and are
  in the range of [a-z][A-Z][0-9]_- without initial number character

*/

#include <bauma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef unsigned int bauma_JsonNodeType;

#define BAUMA_JSON_NODE_TYPE_INVALID  0
#define BAUMA_JSON_NODE_TYPE_NULL     1
#define BAUMA_JSON_NODE_TYPE_BOOL     2
#define BAUMA_JSON_NODE_TYPE_SIGNED   3
#define BAUMA_JSON_NODE_TYPE_UNSIGNED 4
#define BAUMA_JSON_NODE_TYPE_DOUBLE   5
#define BAUMA_JSON_NODE_TYPE_STRING   6
#define BAUMA_JSON_NODE_TYPE_ARRAY    7
#define BAUMA_JSON_NODE_TYPE_OBJECT   8
#define BAUMA_JSON_NODE_NUM_OF_TYPES  9

typedef union bauma_JsonNodeUnion {
	bauma_bool_t    b;
	bauma_intmax_t  si;
	bauma_uintmax_t ui;
	double          d;
	char            *p;
	bauma_Vector    v; /* for map, this points to all keys; for array, this points to all values */
} bauma_JsonNodeUnion;

typedef struct bauma_JsonNode {
	bauma_JsonNodeType    type;
	bauma_JsonNodeUnion   value;
	char                  *pKey; /* Key string for map items, else NULL */
	bauma_pDynmem_handler pMemHandler;
} bauma_JsonNode;

BAUMA_DEF bauma_JsonNode *bauma_json_new_ext(bauma_pDynmem_handler pMemHandler);
#define bauma_json_new() bauma_json_new_ext(&bauma_default_dynmem_handler)

BAUMA_DEF bauma_JsonNode *bauma_json_newNull_ext(bauma_pDynmem_handler pMemHandler);
#define bauma_json_newNull() bauma_json_newNull_ext(&bauma_default_dynmem_handler)

BAUMA_DEF bauma_JsonNode *bauma_json_newBool_ext(bauma_bool_t value, bauma_pDynmem_handler pMemHandler);
#define bauma_json_newBool(value) bauma_json_newBool_ext((value), &bauma_default_dynmem_handler)

BAUMA_DEF bauma_JsonNode *bauma_json_newSigned_ext(bauma_intmax_t value, bauma_pDynmem_handler pMemHandler);
#define bauma_json_newSigned(value) bauma_json_newSigned_ext((value), &bauma_default_dynmem_handler)

BAUMA_DEF bauma_JsonNode *bauma_json_newUnsigned_ext(bauma_uintmax_t value, bauma_pDynmem_handler pMemHandler);
#define bauma_json_newUnsigned(value) bauma_json_newUnsigned_ext((value), &bauma_default_dynmem_handler)

BAUMA_DEF bauma_JsonNode *bauma_json_newDouble_ext(double value, bauma_pDynmem_handler pMemHandler);
#define bauma_json_newDouble(value) bauma_json_newDouble_ext((value), &bauma_default_dynmem_handler)

BAUMA_DEF bauma_JsonNode *bauma_json_newStr_ext(const char *pValue, bauma_pDynmem_handler pMemHandler);
#define bauma_json_newStr(value) bauma_json_newStr_ext((value), &bauma_default_dynmem_handler)

BAUMA_DEF bauma_JsonNode *bauma_json_newStrWithLen_ext(const char *pValue, size_t valueLen, bauma_pDynmem_handler pMemHandler);
#define bauma_json_newStrWithLen(value, len) bauma_json_newStrWithLen_ext((value), (len), &bauma_default_dynmem_handler)

BAUMA_DEF bauma_JsonNode *bauma_json_newArray_ext(bauma_pDynmem_handler pMemHandler);
#define bauma_json_newArray() bauma_json_newArray_ext(&bauma_default_dynmem_handler)
BAUMA_DEF void bauma_json_array_append(bauma_JsonNode* pNode, bauma_JsonNode* pArrayElement);

BAUMA_DEF bauma_JsonNode *bauma_json_newObject_ext(bauma_pDynmem_handler pMemHandler);
#define bauma_json_newObject() bauma_json_newObject_ext(&bauma_default_dynmem_handler)
BAUMA_DEF void bauma_json_object_append(bauma_JsonNode* pNode, const char *pKey, bauma_JsonNode *pValue);

BAUMA_DEF void bauma_json_delete(bauma_JsonNode *pNode);

BAUMA_DEF bauma_JsonNode *bauma_json_parse_ext(bauma_IInputStream *pStream,
                                               bauma_StringBuilder* pErrFormatter,
                                               bauma_pDynmem_handler pMemHandler);

#define bauma_json_parse(pStream) bauma_json_parse_ext((pStream), NULL, &bauma_default_dynmem_handler)

BAUMA_DEF bauma_JsonNode *bauma_json_parseStr_ext(const char *pStr,
                                                  bauma_StringBuilder* pErrFormatter,
                                                  bauma_pDynmem_handler pMemHandler);

#define bauma_json_parseStr(pStr) bauma_json_parseStr_ext((pStr), NULL, &bauma_default_dynmem_handler)

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BAUMA_JSON_IMPLEMENTATION

#include <string.h>
#include <ctype.h>

#ifdef bauma_json_custom_assert
	#define bauma_json_assert(x) bauma_json_custom_assert(x)
#else
	#include <assert.h>
	#define bauma_json_assert(x) assert(x)
#endif


#ifdef __cplusplus
	extern "C" {
#endif

static void bauma_json_NodePtr_destruct(void *ppNode) {
	if (ppNode != NULL) {
		bauma_JsonNode* p = *(bauma_JsonNode**)ppNode;
		bauma_json_delete(p);
	}
}

BAUMA_DEF bauma_JsonNode *bauma_json_new_ext(bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p;
	bauma_json_assert(pMemHandler != NULL);
	p = (bauma_JsonNode*)(*pMemHandler)(NULL, sizeof(bauma_JsonNode), NULL);
	memset(p, 0, sizeof(*p));
	p->pMemHandler = pMemHandler;
	return p;
}

BAUMA_DEF bauma_JsonNode *bauma_json_newNull_ext(bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p = bauma_json_new_ext(pMemHandler);
	p->type = BAUMA_JSON_NODE_TYPE_NULL;
	return p;
}

BAUMA_DEF bauma_JsonNode *bauma_json_newBool_ext(bauma_bool_t value, bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p = bauma_json_new_ext(pMemHandler);
	p->type = BAUMA_JSON_NODE_TYPE_BOOL;
	p->value.b = value;
	return p;
}

BAUMA_DEF bauma_JsonNode *bauma_json_newSigned_ext(bauma_intmax_t value, bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p = bauma_json_new_ext(pMemHandler);
	p->type = BAUMA_JSON_NODE_TYPE_SIGNED;
	p->value.si = value;
	return p;
}

BAUMA_DEF bauma_JsonNode *bauma_json_newUnsigned_ext(bauma_uintmax_t value, bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p = bauma_json_new_ext(pMemHandler);
	p->type = BAUMA_JSON_NODE_TYPE_UNSIGNED;
	p->value.ui = value;
	return p;
}

BAUMA_DEF bauma_JsonNode *bauma_json_newDouble_ext(double value, bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p = bauma_json_new_ext(pMemHandler);
	p->type = BAUMA_JSON_NODE_TYPE_DOUBLE;
	p->value.d = value;
	return p;
}

BAUMA_DEF bauma_JsonNode *bauma_json_newStr_ext(const char *pValue, bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p = bauma_json_new_ext(pMemHandler);
	p->type = BAUMA_JSON_NODE_TYPE_STRING;
	p->value.p = bauma_strdup_ext(pValue, pMemHandler);
	return p;
}

BAUMA_DEF bauma_JsonNode *bauma_json_newStrWithLen_ext(const char *pValue, size_t valueLen, bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p = bauma_json_new_ext(pMemHandler);
	p->type = BAUMA_JSON_NODE_TYPE_STRING;
	p->value.p = bauma_strdupn_ext(pValue, valueLen, pMemHandler);
	return p;
}

BAUMA_DEF bauma_JsonNode *bauma_json_newArray_ext(bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p = bauma_json_new_ext(pMemHandler);
	p->type = BAUMA_JSON_NODE_TYPE_ARRAY;
	bauma_Vector_construct_ext(&p->value.v, bauma_JsonNode*, &bauma_json_NodePtr_destruct, pMemHandler);
	return p;
}

BAUMA_DEF void bauma_json_array_append(bauma_JsonNode *pNode, bauma_JsonNode *pArrayElement) {
	bauma_json_assert(pNode != NULL);
	bauma_json_assert(pArrayElement != NULL);
	bauma_json_assert(pNode->type == BAUMA_JSON_NODE_TYPE_ARRAY);
	bauma_Vector_append(&pNode->value.v, bauma_JsonNode*, &pArrayElement);
}

BAUMA_DEF void bauma_json_object_append(bauma_JsonNode* pNode, const char *pKey, bauma_JsonNode *pValue) {
	bauma_json_assert(pNode != NULL);
	bauma_json_assert(pKey != NULL);
	bauma_json_assert(pValue != NULL);
	bauma_json_assert(pNode->type == BAUMA_JSON_NODE_TYPE_OBJECT);
	if (pValue->pKey != NULL) {
		(*pValue->pMemHandler)(pValue->pKey, 0, NULL);
		pValue->pKey = NULL;
	}
	pValue->pKey = bauma_strdup_ext(pKey, pValue->pMemHandler);
	bauma_Vector_append(&pNode->value.v, bauma_JsonNode*, &pValue);
}


BAUMA_DEF bauma_JsonNode *bauma_json_newObject_ext(bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *p = bauma_json_new_ext(pMemHandler);
	p->type = BAUMA_JSON_NODE_TYPE_OBJECT;
	bauma_Vector_construct_ext(&p->value.v, bauma_JsonNode*, &bauma_json_NodePtr_destruct, pMemHandler);
	return p;
}

BAUMA_DEF void bauma_json_delete(bauma_JsonNode *pNode) {
	if (pNode == NULL) {
		return;
	}
	bauma_json_assert(pNode->pMemHandler != NULL);
	if (pNode->pKey != NULL) {
		(*pNode->pMemHandler)(pNode->pKey, 0, NULL);
	}
	switch(pNode->type) {
		case BAUMA_JSON_NODE_TYPE_STRING: {
			(*pNode->pMemHandler)(pNode->value.p, 0, NULL);
			break;
		}
		case BAUMA_JSON_NODE_TYPE_ARRAY: /* fall through */
		case BAUMA_JSON_NODE_TYPE_OBJECT: {
			bauma_Vector_destruct(&pNode->value.v);
			break;
		}
		default: break;
	}
	(*pNode->pMemHandler)(pNode, 0, NULL);
}

static int bauma_json_nextChar(bauma_IInputStream *pStream) {
	int c;
	bauma_json_assert(pStream != NULL);
	do {
		c = (*pStream->pGetChar)(pStream);
		if (c < 0) return -1;
	} while(isspace(c));
	return c;
}

static bauma_JsonNode *bauma_json_parse_array(bauma_IInputStream *pStream,
                                              bauma_StringBuilder* pErrFormatter,
                                              bauma_pDynmem_handler pMemHandler) {
	bauma_JsonNode *pArray = bauma_json_newArray_ext(pMemHandler);
	for(;;) {
		bauma_JsonNode *pArrayElement;
		int c = bauma_json_nextChar(pStream);
		if (c < 0) {
			bauma_json_delete(pArray);
			return NULL;
		}
		if (c == ']') { /* end of array */
			return pArray;
		}
		if (c == ',') continue;
		(*pStream->pUnget)(pStream);
		pArrayElement = bauma_json_parse_ext(pStream, pErrFormatter, pMemHandler);
		if (pArrayElement == NULL) {
			bauma_json_delete(pArray);
			return NULL;
		}
		bauma_json_array_append(pArray, pArrayElement);
	}
	return NULL; /* unreachable */
}

BAUMA_DEF bauma_JsonNode *bauma_json_parse_ext(bauma_IInputStream *pStream,
                                               bauma_StringBuilder* pErrFormatter,
                                               bauma_pDynmem_handler pMemHandler) {
	int c;
	bauma_json_assert(pStream != NULL);
	bauma_json_assert(pMemHandler != NULL);
	c = bauma_json_nextChar(pStream);
	switch(c) {
		case '[': return bauma_json_parse_array(pStream, pErrFormatter, pMemHandler);
		default: return NULL;
	}
}

BAUMA_DEF bauma_JsonNode *bauma_json_parseStr_ext(const char *pStr,
                                                  bauma_StringBuilder* pErrFormatter,
                                                  bauma_pDynmem_handler pMemHandler) {
	bauma_InputStreamFromMemory stream;
	bauma_InputStreamFromMemory_construct(&stream, pStr, strlen(pStr));
	return bauma_json_parse_ext(&stream.base, pErrFormatter, pMemHandler);
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BAUMA_JSON_IMPLEMENTATION */

#ifdef BAUMA_JSON_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void test_something(void) {
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BAUMA_TEST(test_something);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BAUMA_CCAL_TEST */

#endif /* BAUMA_CCAL_H_INCLUDED */
