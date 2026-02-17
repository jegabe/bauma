#ifndef BAUMA_CCAL_H_INCLUDED
#define BAUMA_CCAL_H_INCLUDED

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

Bau 'ma's general purpose C container and algorithms library (CCAL).
This is to be some kind of STL-like library, but in the spirit of C.
This means that containers treat their data as opaque (void* plus size_t pair)
and move memory around, just like qsort(), memcpy() and memmove() does.
Fun fact: C had "move semantics" from the beginning *grin*.

CCAL has no dependencies except for some common C89 header such as <string.h>,
<stdlib.h> and <stddef.h>, so it should compile everywhere.

The following data structures and algorithms are supported:

- bauma_Vector: Powerful, general purpose dynamic array, also nestable in other containers

How to include in other projects:

This is an stb-style C header only library.
Just download this file and #include <bauma/ccal.h> and you're done when only declarations
are needed.
When also the function definitions are needed, define the macro BAUMA_CCAL_IMPLEMENTATION
and #include <bauma/ccal.h> after that, but only in one source file. Or just pass ccal.h to
a compiler as C source file and pass "-DBAUMA_CCAL_IMPLEMENTATION" which will give an object file
containg the function definitions, which can be linked afterwards.

*/

#include <stddef.h> /* for size_t, NULL and stuff */
#include <limits.h> /* For size limits */

/* Define this before including when something else is needed */
#ifndef BAUMA_DEBUG
	#ifdef NDEBUG /* Standard way of detecting releas build */
		#define BAUMA_DEBUG 0 /* release build */
	#else
		#define BAUMA_DEBUG 1 /* debug build */
	#endif
#endif

/* Detect old vs. new compilers */
#if defined(__cplusplus) && (__cplusplus >= 201103L)
	#define BAUMA_MODERN_C 1
#elif (!defined(__cplusplus)) && (__STDC_VERSION__ >= 199901L)
	#define BAUMA_MODERN_C 1
#else
	#define BAUMA_MODERN_C 0 /* old compilers */
#endif

/* Find fitting bool data type */
#ifdef __cplusplus
	typedef bool bauma_bool_t;
#else /* C */
	#if _BAUMA_MODERN_C
		typedef _Bool bauma_bool_t;
	#else /* older C compiler */
		typedef unsigned char bauma_bool_t;
	#endif
#endif

#define BAUMA_TRUE  ((bauma_bool_t)1)
#define BAUMA_FALSE ((bauma_bool_t)0)

#define bauma_bool2str(x) ((x) ? "true" : "false")

/* Find fitting (u)intmax_t data type */
#if BAUMA_MODERN_C
	#include <stdint.h>
	typedef intmax_t bauma_intmax_t;
	typedef uintmax_t bauma_uintmax_t;
	#if UINTMAX_MAX == UINT_MAX
		#define BAUMA_INTMAX_PREFIX ""
		#define BAUMA_INTMAX_MIN INT_MIN
		#define BAUMA_INTMAX_MAX INT_MAX
		#define BAUMA_UINTMAX_MAX UINT_MAX
	#elif UINTMAX_MAX == ULONG_MAX
		#define BAUMA_INTMAX_PREFIX "l"
		#define BAUMA_INTMAX_MIN LONG_MIN
		#define BAUMA_INTMAX_MAX LONG_MAX
		#define BAUMA_UINTMAX_MAX ULONG_MAX
	#elif UINTMAX_MAX == ULLONG_MAX
		#define BAUMA_INTMAX_PREFIX "ll"
		#define BAUMA_INTMAX_MIN LLONG_MIN
		#define BAUMA_INTMAX_MAX LLONG_MAX
		#define BAUMA_UINTMAX_MAX ULLONG_MAX
	#else
		#error "Can't detect printf prefix for bauma_(u)intmax_t"
	#endif
	#if UINTMAX_MAX > 0xFFFFFFFFFFFFFFFF
		#error "Can't handle UINTMAX_MAX greater than 64 bits, please fix"
	#elif UINTMAX_MAX >= 0xFFFFFFFFFFFFFFFF
		#define BAUMA_UINTMAX_MAX_DECIMAL_LENGTH 20u
	#elif UINTMAX_MAX >= 0xFFFFFFFF
		#define BAUMA_UINTMAX_MAX_DECIMAL_LENGTH 11u
	#endif
#else /* older compiler */
	/* This is a guess but usually those types reflect
		the maximum */
	typedef ptrdiff_t bauma_intmax_t;
	typedef size_t bauma_uintmax_t;
	#if SIZE_MAX == UINT_MAX
		#define BAUMA_INTMAX_PREFIX ""
		#define BAUMA_INTMAX_MIN INT_MIN
		#define BAUMA_INTMAX_MAX INT_MAX
		#define BAUMA_UINTMAX_MAX UINT_MAX
	#elif SIZE_MAX == ULONG_MAX
		#define BAUMA_INTMAX_PREFIX "l"
		#define BAUMA_INTMAX_MIN LONG_MIN
		#define BAUMA_INTMAX_MAX LONG_MAX
		#define BAUMA_UINTMAX_MAX ULONG_MAX
	#elif defined(ULLONG_MAX) && (SIZE_MAX == ULLONG_MAX)
		#define BAUMA_INTMAX_PREFIX "ll"
		#define BAUMA_INTMAX_MIN LLONG_MIN
		#define BAUMA_INTMAX_MAX LLONG_MAX
		#define BAUMA_UINTMAX_MAX ULLONG_MAX
	#else
		#error "Can't detect printf prefix for bauma_(u)intmax_t"
	#endif
	#if SIZE_MAX > 0xFFFFFFFFFFFFFFFF
		#error "Can't handle UINTMAX_MAX greater than 64 bits, please fix"
	#elif SIZE_MAX >= 0xFFFFFFFFFFFFFFFF
		#define BAUMA_UINTMAX_MAX_DECIMAL_LENGTH 20u
	#elif SIZE_MAX >= 0xFFFFFFFF
		#define BAUMA_UINTMAX_MAX_DECIMAL_LENGTH 11u
	#endif
#endif

/*!
When printf'ing a double with %f, this is the max. num of characters put out:
- up to 309 digits before the decimal for representing x*10^308 in decimal
- The decimal separator itself
- 6 decimal digits after the dot
- maybe a minus sign taking an additional byte
*/
#define BAUMA_DOUBLE_MAX_DECIMAL_LEN 317u

/*
Define this before including when something else is needed,
such as "static inline" or "declspec(dllimport)"
*/
#ifndef BAUMA_DEF
	#ifdef BAUMA_DYNAMIC_LINKAGE
		#if defined(_WIN32) && defined(_MSC_VER)
			#ifdef BAUMA_EXPORT
				#define BAUMA_DEF __declspec(dllexport)
			#else
				#define BAUMA_DEF __declspec(dllimport)
			#endif
		#elif defined(__GNUC__) && (defined(__linux__) || defined(__gnu_linux__))
			#define BAUMA_DEF __attribute__((visibility("default")))
		#else
			#error "Unknown compiler/OS combination. Please fix."
		#endif
	#else
		#define BAUMA_DEF
	#endif
#endif

#ifdef __cplusplus
	extern "C" {
#endif

/*! Initial dynamic array capacity for various containers */
#define BAUMA_INITIAL_CAPACITY 16u

/*! Capacity growth factor for various containers */
#define BAUMA_CAPACITY_GROWTH 2u

#define bauma_min(a, b) ((a) < (b) ? (a) : (b))
#define bauma_max(a, b) ((a) < (b) ? (a) : (b))

typedef struct bauma_IMemAllocator bauma_IMemAllocator;

struct bauma_IMemAllocator {
  void (*pDestroy)(bauma_IMemAllocator *pSelf);
  void *(*pRealloc)(bauma_IMemAllocator *pSelf,
                    void *pOld,
                    size_t newSsize,
                    size_t *pOptRealSize);
};

#ifdef BAUMA_DYNAMIC_LINKAGE
	BAUMA_DEF bauma_IMemAllocator *bauma_getDefaultMemAllocator(void);
#else
	extern const bauma_IMemAllocator bauma_defaultMemAllocator;
	#define bauma_getDefaultMemAllocator() \
		((bauma_IMemAllocator*)&bauma_defaultMemAllocator)
#endif

/*! Uses bauma_getDefaultMemAllocator */
BAUMA_DEF void *bauma_malloc(size_t size);
BAUMA_DEF void bauma_free(void *p);
BAUMA_DEF void *bauma_realloc(void *pOld, size_t newSize);

typedef void (*bauma_pDestructor)(void* pCurrentElement);

typedef void (*bauma_pForEachHandler)(void *pCurrentElement, void *pOptUserData);

/*! \brief Prints error message to stderr and calls exit(1) to terminate the application */
BAUMA_DEF void bauma_exit_err(const char *msg);


/*!
Can be used as element destructor (e.g. for vectors) when the element
type is just a pointer to a memory block allocated with bauma_default_dynmem_handler.
De-references the pointer (after casting it to void**) an calls
bauma_default_dynmem_handler on the de-referenced pointer to free the memory
block. Usage example: bauma_Vector constructed with "char*" as data type where
each element is a heap-allocated string (e.g. created by bauma_strdup).
ATTENTION: This is a very simple implementation; For more complex elements that
also need destruction besides freeing of memory, this function might not do enough.
So this is just for memory blocks containing plain old data.
 */
BAUMA_DEF void bauma_memblock_destructor(void* ppMemBlock);

/*
\brief Allocates a copy of the passed null-terminated string and returns it
\param p A null-terminated string
\param alloc Memory allocator to be used
*/
BAUMA_DEF char* bauma_strdup_ext(const char* p, bauma_IMemAllocator *pAlloc);

/*!
\brief Short-hand of bauma_strdup_ext() using the default allocator
\param p A null-terminated string
*/
#define bauma_strdup(p) bauma_strdup_ext(p, bauma_getDefaultMemAllocator())

/*
\brief Allocates a copy of the passed null-terminated string and returns it
\param p A null-terminated string
\param alloc Memory allocator to be used
*/
BAUMA_DEF char* bauma_strdupn_ext(const char* p, size_t n, bauma_IMemAllocator *pAlloc);

/*!
\brief Short-hand of bauma_strdup_ext() using the default allocator
\param p A null-terminated string
*/
#define bauma_strdupn(p, l) bauma_strdupn_ext(p, l, bauma_getDefaultMemAllocator())

typedef union bauma_VectorDataPtr_ {
/* Used by the implementation: */
	void               *pData;
/* For easier debugging: */
	signed char        *pDataSchar;
	unsigned char      *pDataUchar;
	char               *pDataChar;
	signed short       *pDataSshort;
	unsigned short     *pDataUshort;
	signed int         *pDataSint;
	unsigned int       *pDataUint;
	signed long        *pDataSlong;
	unsigned long      *pDataUlong;
#ifdef ULLONG_MAX
	signed long long   *pDataSlongLong;
	unsigned long long *pDataUlonglong;
#endif
	bauma_bool_t       *pDataBool;
	size_t             *pDataSizeT;
	ptrdiff_t          *pDataPtrDiffT;
	bauma_intmax_t     *pDataIntMax;
	bauma_uintmax_t    *pDataUintMax;
	float              *pDataFloat;
	double             *pDataDouble;
	long double        *pDataLongDouble;
	char*              *pDataCharPtr;
} bauma_VectorDataPtr_;

typedef struct bauma_Vector {
	bauma_VectorDataPtr_      d;
	size_t                    elementSize;
	size_t                    size;
	size_t                    capacity;
	bauma_pDestructor         pDestructor;
	bauma_IMemAllocator       *pAlloc;
#if BAUMA_DEBUG
	const char                *pDataType;
#endif
} bauma_Vector;

#if BAUMA_DEBUG
	#define BAUMA_DEBUG_SUFFIX(x) x ## _D
	#define BAUMA_DEBUG_OPT_PARAM(x) , x
#else
	#define BAUMA_DEBUG_SUFFIX(x) x
	#define BAUMA_DEBUG_OPT_PARAM(x)
#endif

BAUMA_DEF void BAUMA_DEBUG_SUFFIX(bauma_Vector_construct_impl)(
	bauma_Vector *pSelf,
	size_t elemSize, 
	bauma_pDestructor pElementDestructor,
	bauma_IMemAllocator *pAlloc
	BAUMA_DEBUG_OPT_PARAM(const char* pDataType)
);

#define bauma_Vector_construct_ext(pSelf, dataType, pElementDestructor, pMemHandler) \
	BAUMA_DEBUG_SUFFIX(bauma_Vector_construct_impl)( \
		pSelf, \
		sizeof(dataType), \
		pElementDestructor, \
		pMemHandler \
		BAUMA_DEBUG_OPT_PARAM(#dataType) \
	)

#define bauma_Vector_construct(pSelf, dataType, pElementDestructor) \
	BAUMA_DEBUG_SUFFIX(bauma_Vector_construct_impl)( \
		pSelf, \
		sizeof(dataType), \
		pElementDestructor, \
		bauma_getDefaultMemAllocator() \
		BAUMA_DEBUG_OPT_PARAM(#dataType) \
	)

BAUMA_DEF void bauma_Vector_destruct(bauma_Vector *pSelf);

BAUMA_DEF void bauma_Vector_reserve(bauma_Vector *pSelf, size_t num);

BAUMA_DEF void BAUMA_DEBUG_SUFFIX(bauma_Vector_append_impl)(
	bauma_Vector *pSelf,
	void *pElem
	BAUMA_DEBUG_OPT_PARAM(const char* pDataType)
);

#define bauma_Vector_append(pSelf, dataType, pElem) \
	BAUMA_DEBUG_SUFFIX(bauma_Vector_append_impl)( \
		pSelf, \
		pElem \
		BAUMA_DEBUG_OPT_PARAM(#dataType) \
	)

#if BAUMA_DEBUG
	BAUMA_DEF void *bauma_Vector_at_impl_D(bauma_Vector *pSelf, size_t index, size_t elementSize, const char *pDataType);
	#define bauma_Vector_at(pSelf, index, dataType) \
		((dataType*)bauma_Vector_at_impl_D(pSelf, index, sizeof(dataType), #dataType))
#else
	#define bauma_Vector_at(pSelf, index, dataType) \
		((dataType*)(((char*)(pSelf)->d.pData) + (index * (pSelf)->elementSize)))
#endif

#define bauma_Vector_forEach(pSelf, dataType, pElement) \
	for(pElement = ((dataType*)((pSelf)->d.pData)); \
	    pElement < ((dataType*)(((char*)(pSelf)->d.pData) + ((pSelf)->size) * (pSelf)->elementSize)); \
		pElement = ((dataType*)(((char*)(pElement)) + (pSelf)->elementSize)))

BAUMA_DEF void bauma_Vector_callForEach(bauma_Vector *pSelf, bauma_pForEachHandler pFunc, void *pOptUserData);

typedef struct bauma_StringBuilder {
	char* pStr;
	size_t size; /* not counting the null terminator */
	size_t capacity;
	bauma_IMemAllocator *pAlloc;
} bauma_StringBuilder;

BAUMA_DEF void bauma_StringBuilder_construct_ext(bauma_StringBuilder *pSelf, size_t capacity, bauma_IMemAllocator *pAlloc);
#define bauma_StringBuilder_construct(pSelf) bauma_StringBuilder_construct_ext((pSelf), BAUMA_INITIAL_CAPACITY, bauma_getDefaultMemAllocator())
BAUMA_DEF void bauma_StringBuilder_destruct(bauma_StringBuilder *pSelf);
#define bauma_StringBuilder_getStr(pSelf) ((pSelf)->pStr)
#define bauma_StringBuilder_getSize(pSelf) ((pSelf)->size)
BAUMA_DEF char *bauma_StringBuilder_release(bauma_StringBuilder *pSelf);
BAUMA_DEF void bauma_StringBuilder_reserve(bauma_StringBuilder *pSelf, size_t num);
BAUMA_DEF void bauma_StringBuilder_appendGeneric(bauma_StringBuilder *pSelf, size_t capacityIncrease, const char *pFmt, ...);
BAUMA_DEF void bauma_StringBuilder_appendStr(bauma_StringBuilder *pSelf, const char *p);
BAUMA_DEF void bauma_StringBuilder_appendStrWithLen(bauma_StringBuilder *pSelf, const char *p, size_t len);
BAUMA_DEF void bauma_StringBuilder_appendChar(bauma_StringBuilder *pSelf, int c, size_t count);
BAUMA_DEF void bauma_StringBuilder_appendSigned(bauma_StringBuilder *pSelf, bauma_intmax_t i);
BAUMA_DEF void bauma_StringBuilder_appendUnsigned(bauma_StringBuilder *pSelf, bauma_uintmax_t i);
BAUMA_DEF void bauma_StringBuilder_appendDouble(bauma_StringBuilder *pSelf, double d);
BAUMA_DEF void bauma_StringBuilder_appendBool(bauma_StringBuilder *pSelf, bauma_bool_t b);

typedef struct bauma_IInputStream {
	int (*pGetChar)(struct bauma_IInputStream *pSelf);
	size_t (*pGet)(struct bauma_IInputStream *pSelf, char* pBuf, size_t bufSize);
	void (*pUnget)(struct bauma_IInputStream *pSelf);
} bauma_IInputStream;

typedef struct bauma_InputStreamFromMemory {
	bauma_IInputStream base;
	const unsigned char *pMem;
	size_t size;
} bauma_InputStreamFromMemory;

BAUMA_DEF void bauma_InputStreamFromMemory_construct(bauma_InputStreamFromMemory *pSelf, const void *pData, size_t size);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BAUMA_CCAL_IMPLEMENTATION

#include <string.h> /* for all kind of memory and string things */
#include <stdarg.h> /* for variadic functions */
#ifdef bauma_custom_assert
	#define bauma_assert(x) bauma_custom_assert(x)
#else
	#include <assert.h>
	#define bauma_assert(x) assert(x)
#endif
#ifndef bauma_custom_dynmem_handler
	#include <stdlib.h> /* for malloc(), free(), exit() etc. */
#endif
#ifndef bauma_custom_exit_err
	#include <stdio.h> /* to print to stderr before leaving */
	#include <stdlib.h> /* for malloc(), free(), exit() etc. */
#endif

#ifdef __cplusplus
	extern "C" {
#endif

BAUMA_DEF void bauma_exit_err(const char *msg) {
#ifdef bauma_custom_exit_err
	bauma_custom_exit_err(msg);
#else
	fprintf(stderr, "Error: %s. Terminating.\n", msg);
	fflush(stderr);
	abort();
#endif
}

static void bauma_defaultMemAllocator_destroy(const bauma_IMemAllocator *pSelf) {
	(void)pSelf;
}

static void *bauma_defaultMemAllocator_realloc(const bauma_IMemAllocator *pSelf,
                                               void *pOld,
                                               size_t newSize,
                                               size_t *pOptRealSize) {
#ifdef bauma_custom_dynmem_handler
	bauma_custom_dynmem_handler(pOld, newSize, pOptRealSize);
#else
	void *p;
	if (pOptRealSize != NULL) {
		*pOptRealSize = 0;
	}
	if (pOld == NULL) {
		if (newSize == 0) {
			return NULL; /* Null pointer is a legitimate memory block of size zero */
		}
		p = malloc(newSize);
		if (p == NULL) {
			bauma_exit_err("Out of memory");
		}
		if (pOptRealSize != NULL) {
			*pOptRealSize = newSize;
		}
		return p;
	}
	if (newSize == 0) {
		free(pOld);
		return NULL;
	}
	p = realloc(pOld, newSize);
	if (p == NULL) {
		bauma_exit_err("Out of memory");
	}
	if (pOptRealSize != NULL) {
		*pOptRealSize = newSize;
	}
	return p;
#endif
}

#ifdef BAUMA_DYNAMIC_LINKAGE

static const bauma_IMemAllocator bauma_defaultMemAllocator = {
	 &bauma_defaultMemAllocator_destroy
	,&bauma_defaultMemAllocator_realloc
};

BAUMA_DEF bauma_IMemAllocator *bauma_getDefaultMemAllocator(void) {
	return (bauma_IMemAllocator*)&bauma_defaultMemAllocator;
}

/* To make the following code faster */
#define bauma_getDefaultMemAllocator() \
	((bauma_IMemAllocator*)&bauma_defaultMemAllocator)

#else

const bauma_IMemAllocator bauma_defaultMemAllocator = {
	 &bauma_defaultMemAllocator_destroy
	,&bauma_defaultMemAllocator_realloc
};

#endif

BAUMA_DEF void *bauma_malloc(size_t size) {
	bauma_IMemAllocator* pAlloc = bauma_getDefaultMemAllocator();
	return (*pAlloc->pRealloc)(pAlloc, NULL, size, NULL);
}

BAUMA_DEF void bauma_free(void *p) {
	bauma_IMemAllocator* pAlloc = bauma_getDefaultMemAllocator();
	(void)(*pAlloc->pRealloc)(pAlloc, p, 0, NULL);
}

BAUMA_DEF void *bauma_realloc(void *pOld, size_t newSize) {
	bauma_IMemAllocator* pAlloc = bauma_getDefaultMemAllocator();
	return (*pAlloc->pRealloc)(pAlloc, pOld, newSize, NULL);
}

BAUMA_DEF void bauma_memblock_destructor(void* ppMemBlock) {
	void* pMemBlock = *(void**)ppMemBlock;
	bauma_free(pMemBlock);
}

BAUMA_DEF char* bauma_strdup_ext(const char* p, bauma_IMemAllocator *pAlloc) {
	size_t l;
	char *c;
	bauma_assert((p != NULL) && "String must not be NULL");
	bauma_assert((pAlloc != NULL) && "Allocator must not be NULL");
	l = strlen(p) + 1u; /* +1 to include the null terminator */
	c = (char*)((pAlloc->pRealloc)(pAlloc, NULL, l, NULL));
	memcpy(c, p, l);
	return c;
}

BAUMA_DEF char* bauma_strdupn_ext(const char* p, size_t n, bauma_IMemAllocator *pAlloc) {
	char *c;
	bauma_assert((p != NULL) && "String must not be NULL");
	bauma_assert((pAlloc != NULL) && "Allocator must not be NULL");
	c = (char*)((*pAlloc->pRealloc)(pAlloc, NULL, n + 1u, NULL));
	memcpy(c, p, n);
	c[n] = '\0';
	return c;
}

BAUMA_DEF void BAUMA_DEBUG_SUFFIX(bauma_Vector_construct_impl)(
	bauma_Vector *pSelf,
	size_t elemSize, 
	bauma_pDestructor pElementDestructor,
	bauma_IMemAllocator *pAlloc
	BAUMA_DEBUG_OPT_PARAM(const char* pDataType)
) {
	bauma_assert(pSelf != NULL);
	bauma_assert(elemSize > 0);
	bauma_assert(pAlloc != NULL);
	pSelf->d.pData = NULL;
	pSelf->elementSize = elemSize;
	pSelf->size = 0;
	pSelf->capacity = 0;
	pSelf->pDestructor = pElementDestructor;
	pSelf->pAlloc = pAlloc;
#if BAUMA_DEBUG
	bauma_assert(pDataType != NULL);
	pSelf->pDataType = pDataType;
#endif
}

BAUMA_DEF void bauma_Vector_destruct(bauma_Vector *pSelf) {
	bauma_assert(pSelf != NULL);
	if (pSelf->pDestructor) {
		char *p = ((char*)pSelf->d.pData) + (pSelf->size * pSelf->elementSize);
		size_t i = pSelf->size;
		while(i-- > 0) {
			p -= pSelf->elementSize;
			(*pSelf->pDestructor)(p);
		}
	}
	(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->d.pData, 0, NULL);
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BAUMA_DEF void bauma_Vector_reserve(bauma_Vector *pSelf, size_t num) {
	size_t newCap;
	size_t realNewCap;
	bauma_assert(pSelf != NULL);
	bauma_assert(pSelf->pAlloc != NULL);
	if ((pSelf->size + num) <= (pSelf->capacity)) {
		return;
	}
	if (pSelf->capacity == 0) {
		newCap = BAUMA_INITIAL_CAPACITY;
	}
	else {
		newCap = pSelf->capacity;
	}
	while((pSelf->size + num) > newCap) {
		newCap *= BAUMA_CAPACITY_GROWTH;
	}
	pSelf->d.pData = (*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->d.pData, newCap * pSelf->elementSize, &realNewCap);
	pSelf->capacity = realNewCap / pSelf->elementSize;
}

BAUMA_DEF void BAUMA_DEBUG_SUFFIX(bauma_Vector_append_impl)(
	bauma_Vector *pSelf,
	void *pElem
	BAUMA_DEBUG_OPT_PARAM(const char* pDataType)
) {
	char* p;
	bauma_assert(pSelf != NULL);
	bauma_assert(strcmp(pDataType, pSelf->pDataType) == 0);
	bauma_Vector_reserve(pSelf, 1u);
	p = ((char*)pSelf->d.pData) + (pSelf->size * pSelf->elementSize);
	memcpy(p, pElem, pSelf->elementSize);
	++pSelf->size;
}

#if BAUMA_DEBUG

BAUMA_DEF void *bauma_Vector_at_impl_D(bauma_Vector *pSelf, size_t index, size_t elementSize, const char *pDataType) {
	bauma_assert(pSelf != NULL);
	bauma_assert(pSelf->pDataType != NULL);
	bauma_assert(pSelf->elementSize == elementSize);
	bauma_assert(strcmp(pDataType, pSelf->pDataType) == 0);
	bauma_assert(index < pSelf->size);
	return ((char*)pSelf->d.pData) + (index * elementSize);
}

#endif

BAUMA_DEF void bauma_Vector_callForEach(bauma_Vector *pSelf, bauma_pForEachHandler pFunc, void *pOptUserData) {
	size_t i;
	char *p;
	bauma_assert(pSelf != NULL);
	bauma_assert(pFunc != NULL);
	p = (char*)pSelf->d.pData;
	for (i=0; i<pSelf->size; ++i) {
		(*pFunc)(p, pOptUserData);
		p += pSelf->elementSize;
	}
}

/*
  An empty StringBuilder shouldn't need to allocate memory; on the other hand,
  the contained string always should point to something valid and
  null-terminated.
  BAUMA_NULLSTR is used for the case that nothing has been appended yet,
  so that bauma_StringBuilder_getStr() always returns a valid string.
*/
static const char BAUMA_NULLSTR_[1] = {'\0'};
#define BAUMA_NULLSTR ((char*)BAUMA_NULLSTR_)

BAUMA_DEF void bauma_StringBuilder_construct_ext(bauma_StringBuilder *pSelf, size_t capacity, bauma_IMemAllocator *pAlloc) {
	bauma_assert(pSelf != NULL);
	bauma_assert(capacity > 0);
	bauma_assert(pAlloc != NULL);
	pSelf->pStr = BAUMA_NULLSTR;
	pSelf->size = 0;
	pSelf->capacity = 0;
	pSelf->pAlloc = pAlloc;
}

BAUMA_DEF void bauma_StringBuilder_destruct(bauma_StringBuilder *pSelf) {
	bauma_assert(pSelf != NULL);
	bauma_assert(pSelf->pAlloc != NULL);
	if (pSelf->pStr != BAUMA_NULLSTR) {
		(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pStr, 0, NULL);
	}
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BAUMA_DEF char *bauma_StringBuilder_release(bauma_StringBuilder *pSelf) {
	char *p;
	bauma_assert(pSelf != NULL);
	bauma_assert(pSelf->pStr != NULL);
	bauma_assert(pSelf->pStr[pSelf->size] == '\0');
	bauma_assert(pSelf->pAlloc != NULL);
	if (pSelf->pStr == BAUMA_NULLSTR) {
		/* Give client always something on the heap */
		p = (*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, NULL, 1u, NULL);
		p[0] = '\0';
	}
	else {
		p = pSelf->pStr;
		pSelf->pStr = BAUMA_NULLSTR;
	}
	pSelf->size = 0;
	pSelf->capacity = 0;
	return p;
}

BAUMA_DEF void bauma_StringBuilder_reserve(bauma_StringBuilder *pSelf, size_t num) {
	size_t newCap;
	size_t realNewCap;
	bauma_assert(pSelf != NULL);
	bauma_assert(pSelf->pAlloc != NULL);
	if ((pSelf->size + num) <= (pSelf->capacity)) {
		return;
	}
	if (pSelf->capacity == 0) {
		newCap = BAUMA_INITIAL_CAPACITY;
	}
	else {
		newCap = pSelf->capacity;
	}
	while((pSelf->size + num) > newCap) {
		newCap *= BAUMA_CAPACITY_GROWTH;
	}
	bauma_assert(pSelf->pAlloc != NULL);
	if (pSelf->pStr == BAUMA_NULLSTR) {
		pSelf->pStr = (*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, NULL, newCap + 1u, &realNewCap);
		pSelf->pStr[0] = '\0';
	}
	else {
		pSelf->pStr = (*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pStr, newCap + 1u, &realNewCap);
	}
	pSelf->capacity = realNewCap - 1u;
}

BAUMA_DEF void bauma_StringBuilder_appendGeneric(bauma_StringBuilder *pSelf, size_t capacityIncrease, const char *pFmt, ...) {
	va_list ap;
	int numPrinted;
	bauma_assert(pSelf != NULL);
	bauma_assert(capacityIncrease > 0);
	bauma_StringBuilder_reserve(pSelf, capacityIncrease);
	++capacityIncrease; /* reserve aways allocates one byte more */
	va_start(ap, pFmt);
#if BAUMA_MODERN_C
	numPrinted = vsnprintf(pSelf->pStr + pSelf->size, capacityIncrease, pFmt, ap);
#else
	numPrinted = vsprintf(pSelf->pStr + pSelf->size, pFmt, ap);
#endif
	va_end(ap);
	if (numPrinted < 0) {
		bauma_exit_err("vs(n)printf returned negative value in bauma_StringBuilder_appendGeneric");
	}
	bauma_assert(numPrinted < capacityIncrease);
	if ((size_t)numPrinted >= capacityIncrease) {
		bauma_exit_err("Too small capacityIncrease in bauma_StringBuilder_appendGeneric, memory may be corrupted");
	}
	pSelf->size += (size_t)numPrinted;
}

BAUMA_DEF void bauma_StringBuilder_appendStr(bauma_StringBuilder *pSelf, const char *p) {
	size_t l;
	bauma_assert(pSelf != NULL);
	bauma_assert(p != NULL);
	l = strlen(p);
	bauma_StringBuilder_reserve(pSelf, l);
	memcpy(pSelf->pStr + pSelf->size, p, l + 1u);
	pSelf->size += l;
}

BAUMA_DEF void bauma_StringBuilder_appendStrWithLen(bauma_StringBuilder *pSelf, const char *p, size_t len) {
	bauma_assert(pSelf != NULL);
	bauma_assert(p != NULL);
	bauma_StringBuilder_reserve(pSelf, len);
	memcpy(pSelf->pStr + pSelf->size, p, len);
	pSelf->size += len;
	pSelf->pStr[pSelf->size] = '\0';

}

BAUMA_DEF void bauma_StringBuilder_appendChar(bauma_StringBuilder *pSelf, int c, size_t count) {
	size_t i;
	char* p;
	bauma_assert(pSelf != NULL);
	bauma_StringBuilder_reserve(pSelf, count);
	p = pSelf->pStr + pSelf->size;
	for (i=0; i<count; ++i) {
		*p++ = (char)((unsigned char)c);
	}
	pSelf->size += count;
	pSelf->pStr[pSelf->size] = '\0';
}

BAUMA_DEF void bauma_StringBuilder_appendSigned(bauma_StringBuilder *pSelf, bauma_intmax_t i) {
	bauma_StringBuilder_appendGeneric(pSelf, BAUMA_UINTMAX_MAX_DECIMAL_LENGTH, "%" BAUMA_INTMAX_PREFIX "d", i);
}

BAUMA_DEF void bauma_StringBuilder_appendUnsigned(bauma_StringBuilder *pSelf, bauma_uintmax_t i) {
	bauma_StringBuilder_appendGeneric(pSelf, BAUMA_UINTMAX_MAX_DECIMAL_LENGTH, "%" BAUMA_INTMAX_PREFIX "u", i);
}

BAUMA_DEF void bauma_StringBuilder_appendDouble(bauma_StringBuilder *pSelf, double d) {
	bauma_StringBuilder_appendGeneric(pSelf, BAUMA_DOUBLE_MAX_DECIMAL_LEN, "%f", d);
}

BAUMA_DEF void bauma_StringBuilder_appendBool(bauma_StringBuilder *pSelf, bauma_bool_t b) {
	if (b) {
		bauma_StringBuilder_appendStrWithLen(pSelf, "true", 4u);
	}
	else {
		bauma_StringBuilder_appendStrWithLen(pSelf, "false", 5u);
	}
}

static int bauma_InputStreamFromMemory_getChar(bauma_IInputStream *pSelf_) {
	bauma_InputStreamFromMemory *pSelf;
	bauma_assert(pSelf_ != NULL);
	pSelf = (bauma_InputStreamFromMemory*)pSelf_;
	if (pSelf->size > 0) {
		int result = *pSelf->pMem;
		++pSelf->pMem;
		--pSelf->size;
		return result;
	}
	return -1;
}

static size_t bauma_InputStreamFromMemory_get(bauma_IInputStream *pSelf_, char* pBuf, size_t bufSize) {
	size_t num;
	bauma_InputStreamFromMemory *pSelf;
	bauma_assert(pSelf_ != NULL);
	pSelf = (bauma_InputStreamFromMemory*)pSelf_;
	num = bauma_min(pSelf->size, bufSize);
	memcpy(pBuf, pSelf->pMem, num);
	pSelf->pMem += num;
	pSelf->size -= num;
	return num;
}

static void bauma_InputStreamFromMemory_unget(bauma_IInputStream *pSelf_) {
	bauma_InputStreamFromMemory *pSelf;
	bauma_assert(pSelf_ != NULL);
	pSelf = (bauma_InputStreamFromMemory*)pSelf_;
	--pSelf->pMem;
	++pSelf->size;

}

BAUMA_DEF void bauma_InputStreamFromMemory_construct(bauma_InputStreamFromMemory *pSelf, const void *pData, size_t size) {
	bauma_assert(pSelf != NULL);
	pSelf->base.pGetChar = &bauma_InputStreamFromMemory_getChar;
	pSelf->base.pGet = &bauma_InputStreamFromMemory_get;
	pSelf->base.pUnget = &bauma_InputStreamFromMemory_unget;
}


#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BAUMA_CCAL_IMPLEMENTATION */

#ifdef BAUMA_CCAL_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

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

void test_strdup(void) {
	char* p;
	p = bauma_strdup("Hello");
	BAUMA_EXPECT(p != NULL);
	BAUMA_EXPECT(strcmp(p, "Hello") == 0);
	bauma_free(p);
}

typedef struct Ints {
	int *items;
	size_t size;
	size_t capacity;
} Ints;

void test_dummy_int_destruct(int* p) {
	(void)p;
}

void test_vector_construct(void) {
	bauma_Vector v;
	bauma_Vector_construct(&v, int, (bauma_pDestructor)&test_dummy_int_destruct);
	BAUMA_EXPECT(v.d.pData == NULL);
	BAUMA_EXPECT(v.elementSize == sizeof(int));
	BAUMA_EXPECT(v.size == 0);
	BAUMA_EXPECT(v.capacity == 0);
	BAUMA_EXPECT(v.pDestructor == (bauma_pDestructor)&test_dummy_int_destruct);
	BAUMA_EXPECT(v.pAlloc == bauma_getDefaultMemAllocator());
#if BAUMA_DEBUG
	BAUMA_EXPECT(strcmp(v.pDataType, "int") == 0);
#endif
	bauma_Vector_destruct(&v);
}

void test_vector_append(void) {
	bauma_Vector v;
	int x = 42;
	bauma_Vector_construct(&v, int, NULL);
	bauma_Vector_append(&v, int, &x);
	BAUMA_EXPECT(v.size == 1u);
	BAUMA_EXPECT(v.capacity == BAUMA_INITIAL_CAPACITY);
	BAUMA_EXPECT(*(int*)v.d.pData == 42);
	bauma_Vector_destruct(&v);
}

void test_vector_at(void) {
	bauma_Vector v;
	int x = 42;
	bauma_Vector_construct(&v, int, NULL);
	bauma_Vector_append(&v, int, &x);
	x = 43;
	bauma_Vector_append(&v, int, &x);
	BAUMA_EXPECT(v.size == 2u);
	BAUMA_EXPECT(*bauma_Vector_at(&v, 0, int) == 42);
	BAUMA_EXPECT(*bauma_Vector_at(&v, 1, int) == 43);
	bauma_Vector_destruct(&v);
}

void test_vector_forEach(void) {
	bauma_Vector v;
	int x = 42;
	int* pElem;
	size_t loopSize = 0;
	bauma_bool_t first = BAUMA_TRUE;
	bauma_Vector_construct(&v, int, NULL);
	bauma_Vector_append(&v, int, &x);
	x = 43;
	bauma_Vector_append(&v, int, &x);
	bauma_Vector_forEach(&v, int, pElem) {
		if (first) {
			BAUMA_EXPECT(*pElem == 42);
		}
		else {
			BAUMA_EXPECT(*pElem == 43);
		}
		first = BAUMA_FALSE;
		++loopSize;
	}
	BAUMA_EXPECT(loopSize == 2u);
	bauma_Vector_destruct(&v);
}

void test_vector_of_strings(void) {
	bauma_Vector v;
	char *p;
	bauma_Vector_construct(&v, char*, &bauma_memblock_destructor);
	p = bauma_strdup("one");
	bauma_Vector_append(&v, char*, &p);
	p = bauma_strdup("two");
	bauma_Vector_append(&v, char*, &p);
	p = bauma_strdup("three");
	bauma_Vector_append(&v, char*, &p);
	BAUMA_EXPECT(strcmp(*bauma_Vector_at(&v, 0, char*), "one") == 0);
	BAUMA_EXPECT(strcmp(*bauma_Vector_at(&v, 1, char*), "two") == 0);
	BAUMA_EXPECT(strcmp(*bauma_Vector_at(&v, 2, char*), "three") == 0);
	bauma_Vector_destruct(&v);
}

void test_dummy_forEachCallBack(int* p, bauma_Vector* pUserData) {
	bauma_Vector_append(pUserData, int, p);
}

void test_vector_callForEach(void) {
	bauma_Vector v;
	bauma_Vector v2;
	int x;
	bauma_Vector_construct(&v, int, NULL);
	bauma_Vector_construct(&v2, int, NULL);
	x = 0;
	bauma_Vector_append(&v, int, &x);
	x = 1;
	bauma_Vector_append(&v, int, &x);
	x = 2;
	bauma_Vector_append(&v, int, &x);
	bauma_Vector_callForEach(&v, (bauma_pForEachHandler)&test_dummy_forEachCallBack, &v2);
	BAUMA_EXPECT(v2.size == 3);
	BAUMA_EXPECT(*bauma_Vector_at(&v2, 0, int) == 0);
	BAUMA_EXPECT(*bauma_Vector_at(&v2, 1, int) == 1);
	BAUMA_EXPECT(*bauma_Vector_at(&v2, 2, int) == 2);
	bauma_Vector_destruct(&v2);
	bauma_Vector_destruct(&v);
}

void test_stringBuilder_construct(void) {
	bauma_StringBuilder sb;
	bauma_StringBuilder_construct(&sb);
	BAUMA_EXPECT(sb.pStr = BAUMA_NULLSTR);
	BAUMA_EXPECT(sb.size == 0);
	BAUMA_EXPECT(sb.capacity == 0);
	BAUMA_EXPECT(sb.pAlloc = bauma_getDefaultMemAllocator());
	bauma_StringBuilder_destruct(&sb);
}

void test_stringBuilder_release(void) {
	bauma_StringBuilder sb;
	char *p;
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendStr(&sb, "Hello");
	p = bauma_StringBuilder_release(&sb);
	BAUMA_EXPECT(strcmp(p, "Hello") == 0);
	bauma_free(p);
	/* State after release() should be same as after construction */
	BAUMA_EXPECT(sb.pStr = BAUMA_NULLSTR);
	BAUMA_EXPECT(sb.size == 0);
	BAUMA_EXPECT(sb.capacity == 0);
	BAUMA_EXPECT(sb.pAlloc = bauma_getDefaultMemAllocator());
	bauma_StringBuilder_destruct(&sb);
	/* Release of empty string should heap-allocate: */
	bauma_StringBuilder_construct(&sb);
	p = bauma_StringBuilder_release(&sb);
	bauma_StringBuilder_destruct(&sb);
	BAUMA_EXPECT(strcmp(p, "") == 0);
	bauma_free(p);
}

void test_stringBuilder_reserve(void) {
	bauma_StringBuilder sb;
	bauma_StringBuilder_construct(&sb);
	BAUMA_EXPECT(sb.pStr == BAUMA_NULLSTR);
	BAUMA_EXPECT(sb.size == 0);
	BAUMA_EXPECT(sb.capacity == 0);
	bauma_StringBuilder_reserve(&sb, 1u);
	BAUMA_EXPECT(sb.pStr != BAUMA_NULLSTR);
	BAUMA_EXPECT(sb.pStr[0] == '\0');
	BAUMA_EXPECT(sb.size == 0);
	BAUMA_EXPECT(sb.capacity == BAUMA_INITIAL_CAPACITY);
	bauma_StringBuilder_destruct(&sb);
}

void test_stringBuilder_appendGeneric(void) {
	bauma_StringBuilder sb;
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendGeneric(&sb, 3u, "%d", 123);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), "123") == 0);
	bauma_StringBuilder_destruct(&sb);
}

void test_stringBuilder_appendStr(void) {
	bauma_StringBuilder sb;
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendStr(&sb, "Hello");
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), "Hello") == 0);
	bauma_StringBuilder_destruct(&sb);
}

void test_stringBuilder_appendStrWithLen(void) {
	bauma_StringBuilder sb;
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendStrWithLen(&sb, "Hello", 3u);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), "Hel") == 0);
	bauma_StringBuilder_destruct(&sb);
}

void test_stringBuilder_appendChar(void) {
	bauma_StringBuilder sb;
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendChar(&sb, 'A', 3u);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), "AAA") == 0);
	bauma_StringBuilder_destruct(&sb);
}

void test_stringBuilder_appendSigned(void) {
	bauma_StringBuilder sb;
	char cmp[BAUMA_UINTMAX_MAX_DECIMAL_LENGTH + 1u] = {0};
	/* min */
	sprintf(cmp, "%" BAUMA_INTMAX_PREFIX "d", BAUMA_INTMAX_MIN);
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendSigned(&sb, BAUMA_INTMAX_MIN);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), cmp) == 0);
	bauma_StringBuilder_destruct(&sb);
	/* 0 */
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendSigned(&sb, 0);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), "0") == 0);
	bauma_StringBuilder_destruct(&sb);
	/* max */
	memset(cmp, 0, sizeof(cmp));
	sprintf(cmp, "%" BAUMA_INTMAX_PREFIX "d", BAUMA_INTMAX_MAX);
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendSigned(&sb, BAUMA_INTMAX_MAX);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), cmp) == 0);
	bauma_StringBuilder_destruct(&sb);
}

void test_stringBuilder_appendUnsigned(void) {
	bauma_StringBuilder sb;
	char cmp[BAUMA_UINTMAX_MAX_DECIMAL_LENGTH + 1u] = {0};
	/* 0 */
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendUnsigned(&sb, 0);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), "0") == 0);
	bauma_StringBuilder_destruct(&sb);
	/* max */
	sprintf(cmp, "%" BAUMA_INTMAX_PREFIX "u", BAUMA_UINTMAX_MAX);
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendUnsigned(&sb, BAUMA_UINTMAX_MAX);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), cmp) == 0);
	bauma_StringBuilder_destruct(&sb);
}

void test_stringBuilder_appendDouble(void) {
	bauma_StringBuilder sb;
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendDouble(&sb, -DBL_MAX);
	bauma_StringBuilder_appendDouble(&sb, 0.0);
	bauma_StringBuilder_appendDouble(&sb, DBL_MAX);
	bauma_StringBuilder_appendDouble(&sb, -DBL_MIN);
	bauma_StringBuilder_appendDouble(&sb, DBL_MIN);
	bauma_StringBuilder_destruct(&sb);
}

void test_stringBuilder_appendBool(void) {
	bauma_StringBuilder sb;
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendBool(&sb, BAUMA_TRUE);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), "true") == 0);
	bauma_StringBuilder_destruct(&sb);
	bauma_StringBuilder_construct(&sb);
	bauma_StringBuilder_appendBool(&sb, BAUMA_FALSE);
	BAUMA_EXPECT(strcmp(bauma_StringBuilder_getStr(&sb), "false") == 0);
	bauma_StringBuilder_destruct(&sb);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BAUMA_TEST(test_strdup);
	BAUMA_TEST(test_vector_construct);
	BAUMA_TEST(test_vector_append);
	BAUMA_TEST(test_vector_at);
	BAUMA_TEST(test_vector_forEach);
	BAUMA_TEST(test_vector_of_strings);
	BAUMA_TEST(test_vector_callForEach);
	BAUMA_TEST(test_stringBuilder_construct);
	BAUMA_TEST(test_stringBuilder_release);
	BAUMA_TEST(test_stringBuilder_reserve);
	BAUMA_TEST(test_stringBuilder_appendGeneric);
	BAUMA_TEST(test_stringBuilder_appendStr);
	BAUMA_TEST(test_stringBuilder_appendStrWithLen);
	BAUMA_TEST(test_stringBuilder_appendChar);
	BAUMA_TEST(test_stringBuilder_appendSigned);
	BAUMA_TEST(test_stringBuilder_appendUnsigned);
	BAUMA_TEST(test_stringBuilder_appendDouble);
	BAUMA_TEST(test_stringBuilder_appendBool);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BAUMA_CCAL_TEST */

#endif /* BAUMA_CCAL_H_INCLUDED */
