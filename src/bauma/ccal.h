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
	typedef long bauma_intmax_t;
	typedef unsigned long bauma_uintmax_t;
	#define BAUMA_INTMAX_PREFIX "l"
	#define BAUMA_INTMAX_MIN LONG_MIN
	#define BAUMA_INTMAX_MAX LONG_MAX
	#define BAUMA_UINTMAX_MAX ULONG_MAX
	#if ULONG_MAX > 0xFFFFFFFFFFFFFFFF
		#error "Can't handle UINTMAX_MAX greater than 64 bits, please fix"
	#elif ULONG_MAX >= 0xFFFFFFFFFFFFFFFF
		#define BAUMA_UINTMAX_MAX_DECIMAL_LENGTH 20u
	#elif ULONG_MAX >= 0xFFFFFFFF
		#define BAUMA_UINTMAX_MAX_DECIMAL_LENGTH 11u
	#endif
#endif

#ifdef __cplusplus
	#if BAUMA_MODERN_C /* C++11 or newer */
		typedef max_align_t bauma_max_align_t;
	#else
		typedef double bauma_max_align_t;
	#endif
#else /* C */
	#if (__STDC_VERSION__ >= 201112L)
		#ifdef _MSC_VER
			/* Visual Studio misses that */
			typedef double bauma_max_align_t;
		#else
			typedef max_align_t bauma_max_align_t;
		#endif
	#else
		typedef double bauma_max_align_t;
	#endif
#endif

/*! For %g to printf double' that don't loose precision when re-parsed */
#define BAUMA_DOUBLE_ROUNDTRIP_PRECISION_STR "17"

/*!
When printf'ing a double with %.17g, this is the max. num of characters put out
inclusive a possible null terminator.
Example: "-9.999999999999999e+308"
*/
#define BAUMA_DOUBLE_MAX_DECIMAL_LEN 25u


/*
Define this before including when something else is needed,
such as "static inline" or "declspec(dllimport)"
*/
#ifndef BAUMA_CCAL_DEF
	#define BAUMA_CCAL_DEF
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

extern const bauma_IMemAllocator bauma_defaultMemAllocator_;
#define bauma_getDefaultMemAllocator() \
	((bauma_IMemAllocator*)&bauma_defaultMemAllocator_)

/*! Uses bauma_getDefaultMemAllocator */
BAUMA_CCAL_DEF void *bauma_malloc(size_t size);
BAUMA_CCAL_DEF void bauma_free(void *p);
BAUMA_CCAL_DEF void *bauma_realloc(void *pOld, size_t newSize);

#define bauma_new(ppData, dataType, ctor) \
	do { \
		(*ppData) = (dataType*)bauma_malloc(sizeof(dataType)); \
		ctor(*ppData); \
	} while(0)

#define bauma_delete(ppData, dtor) \
	do { \
		dtor(*ppData); \
		bauma_free(*ppData); \
	} while(0)

typedef void (*bauma_pDestructor)(void* pCurrentElement);

typedef void (*bauma_pForEachHandler)(void *pCurrentElement, void *pOptUserData);

typedef size_t (*bauma_pHashFunction)(const void *pKey);

typedef bauma_bool_t (*bauma_pEqualsFunction)(const void *pLhs, const void *pRhs);

/*! \brief Prints error message to stderr and calls exit(1) to terminate the application */
BAUMA_CCAL_DEF void bauma_exit_err(const char *msg);

/*!
Returns the proper memory alignment for a data type of known size.
For char, this will return 1 and so on. The maximum possible value is
the alignment of bauma_memalign_t. The return value is guaranteed to
be a power of two.
 */
BAUMA_CCAL_DEF size_t bauma_alignof(size_t dataTypeSize);

/*!
Does run-time structure alignment (insert gaps where needed) to fit a new data type
at a known offset by conditionally increasing that offset.
Example for a compiler-generated "gapped" structure:
\code
struct X { char a, int b; };
\endcode  
If that gaps need to be calculated at run-time rather than compile time,
bauma_memgap() can be called
with bauma_memgap(sizeof(char), sizeof(int)) and will return a suitable offset
to fit an int after a char. In this example, 3 or 7 will be added depending
of the size of int and on the machine word width. The result is not a gap value
but the absolute offset.
\param offset A data structure offset (from beginning) in bytes
\param followingTypeSize Size of a data type that has to be put at that offset
\return Adjusted offset which might contain gap bytes
*/
BAUMA_CCAL_DEF size_t bauma_memgap(size_t offset, size_t followingTypeSize);

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
BAUMA_CCAL_DEF void bauma_memblock_destructor(void* ppMemBlock);

BAUMA_CCAL_DEF size_t bauma_strHasher(const void *ppStr);

BAUMA_CCAL_DEF bauma_bool_t bauma_strEquals(const void* ppLhs, const void *ppRhs);

/*
\brief Allocates a copy of the passed null-terminated string and returns it
\param p A null-terminated string
\param alloc Memory allocator to be used
*/
BAUMA_CCAL_DEF char* bauma_strdup_ext(const char* p, bauma_IMemAllocator *pAlloc);

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
BAUMA_CCAL_DEF char *bauma_strdupn_ext(const char* p, size_t n, bauma_IMemAllocator *pAlloc);

/*!
\brief Short-hand of bauma_strdup_ext() using the default allocator
\param p A null-terminated string
*/
#define bauma_strdupn(p, l) bauma_strdupn_ext(p, l, bauma_getDefaultMemAllocator())

BAUMA_CCAL_DEF void *bauma_memmem(const void *pHayStack, size_t hayStackSize, const void *pNeedle, size_t needleSize);

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

BAUMA_CCAL_DEF void BAUMA_DEBUG_SUFFIX(bauma_Vector_construct_impl)(
	bauma_Vector *pSelf,
	size_t elemSize, 
	bauma_pDestructor pElementDestructor,
	bauma_IMemAllocator *pAlloc
	BAUMA_DEBUG_OPT_PARAM(const char* pDataType)
);

#define bauma_Vector_construct_ext(pSelf, dataType, pElementDestructor, pAlloc) \
	BAUMA_DEBUG_SUFFIX(bauma_Vector_construct_impl)( \
		pSelf, \
		sizeof(dataType), \
		pElementDestructor, \
		pAlloc \
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

BAUMA_CCAL_DEF void bauma_Vector_destruct(bauma_Vector *pSelf);

BAUMA_CCAL_DEF void bauma_Vector_reserve(bauma_Vector *pSelf, size_t num);

BAUMA_CCAL_DEF void BAUMA_DEBUG_SUFFIX(bauma_Vector_append_impl)(
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
	BAUMA_CCAL_DEF void *bauma_Vector_at_impl_D(bauma_Vector *pSelf, size_t index, size_t elementSize, const char *pDataType);
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

#define bauma_Vector_getSize(pSelf) ((const size_t)((pSelf)->size))

#define bauma_Vector_getCapacity(pSelf) ((const size_t)((pSelf)->capacity))

#define bauma_Vector_getAlloc(pSelf) ((bauma_IMemAllocator* const)((pSelf)->pAlloc))

BAUMA_CCAL_DEF void bauma_Vector_callForEach(bauma_Vector *pSelf, bauma_pForEachHandler pFunc, void *pOptUserData);

typedef struct bauma_HashMapBucketHdr_ bauma_HashMapBucketHdr_;

typedef struct bauma_HashMap {
	bauma_HashMapBucketHdr_   **pBuckets;
	size_t                    numOfBuckets;
	size_t                    size;
	size_t                    keySize;
	size_t                    valueSize;
	size_t                    keyOffset;
	size_t                    valueOffset;
	size_t                    hashKeyValueStructSize;
	bauma_pDestructor         pKeyDestructor;
	bauma_pDestructor         pValueDestructor;
	bauma_pHashFunction       pKeyHash;
	bauma_pEqualsFunction     pKeyEquals;
	bauma_IMemAllocator       *pAlloc;
#if BAUMA_DEBUG
	const char                *pKeyDataType;
	const char                *pValueDataType;
#endif
} bauma_HashMap;

BAUMA_CCAL_DEF void BAUMA_DEBUG_SUFFIX(bauma_HashMap_construct_impl)(
	bauma_HashMap *pSelf,
	size_t keySize,
	size_t valueSize,
	bauma_pDestructor pKeyDestructor,
	bauma_pDestructor pValueDestructor,
	bauma_pHashFunction pKeyHasher,
	bauma_pEqualsFunction pKeyEquals,
	bauma_IMemAllocator *pAlloc
	BAUMA_DEBUG_OPT_PARAM(const char* pKeyDataType)
	BAUMA_DEBUG_OPT_PARAM(const char* pValueDataType)
);

#define bauma_HashMap_construct_ext(pSelf, keyType, valueType, pKeyDestructor, pValueDestructor, pKeyHasher, pKeyEquals, pAlloc) \
	BAUMA_DEBUG_SUFFIX(bauma_HashMap_construct_impl)( \
		pSelf, \
		sizeof(keyType), \
		sizeof(valueType), \
		pKeyDestructor, \
		pValueDestructor, \
		pKeyHasher, \
		pKeyEquals, \
		pAlloc \
		BAUMA_DEBUG_OPT_PARAM(#keyType) \
		BAUMA_DEBUG_OPT_PARAM(#valueType) \
	)

#define bauma_HashMap_construct(pSelf, keyType, valueType, pKeyDestructor, pValueDestructor, pKeyHasher, pKeyEquals) \
	BAUMA_DEBUG_SUFFIX(bauma_HashMap_construct_impl)( \
		pSelf, \
		sizeof(keyType), \
		sizeof(valueType), \
		pKeyDestructor, \
		pValueDestructor, \
		pKeyHasher, \
		pKeyEquals, \
		bauma_getDefaultMemAllocator() \
		BAUMA_DEBUG_OPT_PARAM(#keyType) \
		BAUMA_DEBUG_OPT_PARAM(#valueType) \
	)

BAUMA_CCAL_DEF void bauma_HashMap_destruct(bauma_HashMap *pSelf);

#define bauma_HashMap_getSize(pSelf) ((const size_t)((pSelf)->size))

BAUMA_CCAL_DEF bauma_bool_t BAUMA_DEBUG_SUFFIX(bauma_HashMap_put_impl)(
	bauma_HashMap *pSelf,
	void *pKey,
	void *pValue
	BAUMA_DEBUG_OPT_PARAM(const char* pKeyDataType)
	BAUMA_DEBUG_OPT_PARAM(const char* pValueDataType)
);

#define bauma_HashMap_put(pSelf, pKey, pValue, keyType, valueType) \
	BAUMA_DEBUG_SUFFIX(bauma_HashMap_put_impl)(pSelf, \
						   pKey, \
						   pValue \
						   BAUMA_DEBUG_OPT_PARAM(#keyType) \
						   BAUMA_DEBUG_OPT_PARAM(#valueType))

BAUMA_CCAL_DEF void* BAUMA_DEBUG_SUFFIX(bauma_HashMap_get_impl)(
	bauma_HashMap *pSelf,
	const void *pKey
	BAUMA_DEBUG_OPT_PARAM(const char* pKeyDataType)
	BAUMA_DEBUG_OPT_PARAM(const char* pValueDataType)
);

#define bauma_HashMap_get(pSelf, pKey, keyType, valueType) \
	((valueType*)BAUMA_DEBUG_SUFFIX(bauma_HashMap_get_impl)(pSelf, \
	                                                        pKey \
	                                                        BAUMA_DEBUG_OPT_PARAM(#keyType) \
	                                                        BAUMA_DEBUG_OPT_PARAM(#valueType)))

BAUMA_CCAL_DEF void bauma_HashMap_clear(bauma_HashMap* pSelf);

typedef struct bauma_StringBuilder {
	char* pStr;
	size_t size; /* not counting the null terminator */
	size_t capacity;
	bauma_IMemAllocator *pAlloc;
} bauma_StringBuilder;

BAUMA_CCAL_DEF void bauma_StringBuilder_construct_ext(bauma_StringBuilder *pSelf, bauma_IMemAllocator *pAlloc);
#define bauma_StringBuilder_construct(pSelf) bauma_StringBuilder_construct_ext((pSelf), bauma_getDefaultMemAllocator())
BAUMA_CCAL_DEF void bauma_StringBuilder_destruct(bauma_StringBuilder *pSelf);
#define bauma_StringBuilder_getStr(pSelf) ((pSelf)->pStr)
#define bauma_StringBuilder_getSize(pSelf) ((pSelf)->size)
BAUMA_CCAL_DEF char *bauma_StringBuilder_release(bauma_StringBuilder *pSelf);
BAUMA_CCAL_DEF void bauma_StringBuilder_reserve(bauma_StringBuilder *pSelf, size_t num);
BAUMA_CCAL_DEF void bauma_StringBuilder_appendGeneric(bauma_StringBuilder *pSelf, size_t capacityIncrease, const char *pFmt, ...);
BAUMA_CCAL_DEF void bauma_StringBuilder_appendStr(bauma_StringBuilder *pSelf, const char *p);
BAUMA_CCAL_DEF void bauma_StringBuilder_appendStrWithLen(bauma_StringBuilder *pSelf, const char *p, size_t len);
BAUMA_CCAL_DEF void bauma_StringBuilder_appendChar(bauma_StringBuilder *pSelf, int c, size_t count);
BAUMA_CCAL_DEF void bauma_StringBuilder_appendSigned(bauma_StringBuilder *pSelf, bauma_intmax_t i);
BAUMA_CCAL_DEF void bauma_StringBuilder_appendUnsigned(bauma_StringBuilder *pSelf, bauma_uintmax_t i);
BAUMA_CCAL_DEF void bauma_StringBuilder_appendDouble(bauma_StringBuilder *pSelf, double d);
BAUMA_CCAL_DEF void bauma_StringBuilder_appendBool(bauma_StringBuilder *pSelf, bauma_bool_t b);
BAUMA_CCAL_DEF void bauma_StringBuilder_appendCodePointUtf8(bauma_StringBuilder *pSelf, unsigned long codePoint);
BAUMA_CCAL_DEF void bauma_StringBuilder_clear(bauma_StringBuilder *pSelf);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BAUMA_CCAL_IMPLEMENTATION

#include <string.h> /* for all kind of memory and string things */
#include <stdarg.h> /* for variadic functions */
#ifdef bauma_ccal_custom_assert
	#define bauma_ccal_assert(x) bauma_ccal_custom_assert(x)
#else
	#include <assert.h>
	#define bauma_ccal_assert(x) assert(x)
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

typedef struct bauma_AlignFinder_ {
	char c;
	bauma_max_align_t a;
} bauma_AlignFinder_;

#define BAUMA_MAX_ALIGNMENT offsetof(bauma_AlignFinder_, a)

BAUMA_CCAL_DEF bauma_bool_t bauma_is_power_of_two(size_t value) {
	return (bauma_bool_t)((value > 0) && !(value & (value - 1)));
}

BAUMA_CCAL_DEF size_t bauma_alignof(size_t dataTypeSize) {
	bauma_ccal_assert(bauma_is_power_of_two(BAUMA_MAX_ALIGNMENT));
	bauma_ccal_assert(dataTypeSize > 0);
	if (dataTypeSize < BAUMA_MAX_ALIGNMENT) {
		/* next-power-of-two bit twiddling */
		--dataTypeSize;
		dataTypeSize |= dataTypeSize >> 1;
		dataTypeSize |= dataTypeSize >> 2;
		dataTypeSize |= dataTypeSize >> 4;
		dataTypeSize |= dataTypeSize >> 8;
		dataTypeSize |= dataTypeSize >> 16;
#if SIZE_MAX > 0xFFFFFFFF
		dataTypeSize |= dataTypeSize >> 32;
#endif
#if SIZE_MAX > 0xFFFFFFFFFFFFFFFF
		dataTypeSize |= dataTypeSize >> 64;
#endif
		++dataTypeSize;		
		bauma_ccal_assert(bauma_is_power_of_two(dataTypeSize));
		return dataTypeSize;
	}
	return BAUMA_MAX_ALIGNMENT;
}

BAUMA_CCAL_DEF size_t bauma_memgap(size_t offset, size_t followingTypeSize) {
	/* This is a power of two: */
	size_t alignOfFollowing = bauma_alignof(followingTypeSize);
	--alignOfFollowing; /* make bit field out of it */
	offset += alignOfFollowing;
	offset &= ~alignOfFollowing;
	return offset;
}

BAUMA_CCAL_DEF void bauma_exit_err(const char *msg) {
#ifdef bauma_custom_exit_err
	bauma_custom_exit_err(msg);
#else
	fprintf(stderr, "Error: %s. Terminating.\n", msg);
	fflush(stderr);
	abort();
#endif
}

BAUMA_CCAL_DEF void bauma_defaultMemAllocator_destroy(bauma_IMemAllocator *pSelf) {
	(void)pSelf;
}

BAUMA_CCAL_DEF void *bauma_defaultMemAllocator_realloc(bauma_IMemAllocator *pSelf,
                                               void *pOld,
                                               size_t newSize,
                                               size_t *pOptRealSize) {
#ifdef bauma_custom_dynmem_handler
	bauma_custom_dynmem_handler(pOld, newSize, pOptRealSize);
#else
	void *p;
	(void)pSelf;
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

const bauma_IMemAllocator bauma_defaultMemAllocator_ = {
	 &bauma_defaultMemAllocator_destroy
	,&bauma_defaultMemAllocator_realloc
};

BAUMA_CCAL_DEF void *bauma_malloc(size_t size) {
	bauma_IMemAllocator* pAlloc = bauma_getDefaultMemAllocator();
	return (*pAlloc->pRealloc)(pAlloc, NULL, size, NULL);
}

BAUMA_CCAL_DEF void bauma_free(void *p) {
	bauma_IMemAllocator* pAlloc = bauma_getDefaultMemAllocator();
	(void)(*pAlloc->pRealloc)(pAlloc, p, 0, NULL);
}

BAUMA_CCAL_DEF void *bauma_realloc(void *pOld, size_t newSize) {
	bauma_IMemAllocator* pAlloc = bauma_getDefaultMemAllocator();
	return (*pAlloc->pRealloc)(pAlloc, pOld, newSize, NULL);
}

BAUMA_CCAL_DEF void bauma_memblock_destructor(void* ppMemBlock) {
	void* pMemBlock = *(void**)ppMemBlock;
	bauma_free(pMemBlock);
}

BAUMA_CCAL_DEF size_t bauma_strHasher(const void *ppStr) {
	const char* p;
	size_t result = 0;
	bauma_ccal_assert(ppStr != NULL);
	p = *(const char**)ppStr;
	bauma_ccal_assert(p != 0);
	while(*p) {
		result = (31u * result) + (size_t)((unsigned char)*p);
		++p;
	}
	return result;
}

BAUMA_CCAL_DEF bauma_bool_t bauma_strEquals(const void* ppLhs, const void *ppRhs) {
	const char* pLhs;
	const char* pRhs;
	bauma_ccal_assert(ppLhs != NULL);
	bauma_ccal_assert(ppRhs != NULL);
	pLhs = *(const char**)ppLhs;
	pRhs = *(const char**)ppRhs;
	return (strcmp(pLhs, pRhs) == 0);
}


BAUMA_CCAL_DEF char* bauma_strdup_ext(const char* p, bauma_IMemAllocator *pAlloc) {
	size_t l;
	char *c;
	bauma_ccal_assert((p != NULL) && "String must not be NULL");
	bauma_ccal_assert((pAlloc != NULL) && "Allocator must not be NULL");
	l = strlen(p) + 1u; /* +1 to include the null terminator */
	c = (char*)((pAlloc->pRealloc)(pAlloc, NULL, l, NULL));
	memcpy(c, p, l);
	return c;
}

BAUMA_CCAL_DEF char* bauma_strdupn_ext(const char* p, size_t n, bauma_IMemAllocator *pAlloc) {
	char *c;
	bauma_ccal_assert((p != NULL) && "String must not be NULL");
	bauma_ccal_assert((pAlloc != NULL) && "Allocator must not be NULL");
	c = (char*)((*pAlloc->pRealloc)(pAlloc, NULL, n + 1u, NULL));
	memcpy(c, p, n);
	c[n] = '\0';
	return c;
}

BAUMA_CCAL_DEF void *bauma_memmem(const void *pHayStack, size_t hayStackSize, const void *pNeedle, size_t needleSize) {
    const unsigned char *h = (const unsigned char *)pHayStack;
    const unsigned char *n = (const unsigned char *)pNeedle;
    const unsigned char *p = h;
    const unsigned char *end = h + hayStackSize - needleSize + 1;
    if (needleSize == 0) return (void *)h;
    if (needleSize > hayStackSize) return NULL;
    while ((p = memchr(p, n[0], end - p)) != NULL) {
        if (memcmp(p, n, needleSize) == 0) {
            return (void *)p;
        }
        ++p;
    }
    return NULL;
}

BAUMA_CCAL_DEF void BAUMA_DEBUG_SUFFIX(bauma_Vector_construct_impl)(
	bauma_Vector *pSelf,
	size_t elemSize, 
	bauma_pDestructor pElementDestructor,
	bauma_IMemAllocator *pAlloc
	BAUMA_DEBUG_OPT_PARAM(const char* pDataType)
) {
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(elemSize > 0);
	bauma_ccal_assert(pAlloc != NULL);
	pSelf->d.pData = NULL;
	pSelf->elementSize = elemSize;
	pSelf->size = 0;
	pSelf->capacity = 0;
	pSelf->pDestructor = pElementDestructor;
	pSelf->pAlloc = pAlloc;
#if BAUMA_DEBUG
	bauma_ccal_assert(pDataType != NULL);
	pSelf->pDataType = pDataType;
#endif
}

BAUMA_CCAL_DEF void bauma_Vector_destruct(bauma_Vector *pSelf) {
	bauma_ccal_assert(pSelf != NULL);
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

BAUMA_CCAL_DEF void bauma_Vector_reserve(bauma_Vector *pSelf, size_t num) {
	size_t newCap;
	size_t realNewCap;
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(pSelf->pAlloc != NULL);
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

BAUMA_CCAL_DEF void BAUMA_DEBUG_SUFFIX(bauma_Vector_append_impl)(
	bauma_Vector *pSelf,
	void *pElem
	BAUMA_DEBUG_OPT_PARAM(const char* pDataType)
) {
	char* p;
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(strcmp(pDataType, pSelf->pDataType) == 0);
	bauma_Vector_reserve(pSelf, 1u);
	p = ((char*)pSelf->d.pData) + (pSelf->size * pSelf->elementSize);
	memcpy(p, pElem, pSelf->elementSize);
	++pSelf->size;
}

#if BAUMA_DEBUG

BAUMA_CCAL_DEF void *bauma_Vector_at_impl_D(bauma_Vector *pSelf, size_t index, size_t elementSize, const char *pDataType) {
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(pSelf->pDataType != NULL);
	bauma_ccal_assert(pSelf->elementSize == elementSize);
	bauma_ccal_assert(strcmp(pDataType, pSelf->pDataType) == 0);
	bauma_ccal_assert(index < pSelf->size);
	return ((char*)pSelf->d.pData) + (index * elementSize);
}

#endif

BAUMA_CCAL_DEF void bauma_Vector_callForEach(bauma_Vector *pSelf, bauma_pForEachHandler pFunc, void *pOptUserData) {
	size_t i;
	char *p;
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(pFunc != NULL);
	p = (char*)pSelf->d.pData;
	for (i=0; i<pSelf->size; ++i) {
		(*pFunc)(p, pOptUserData);
		p += pSelf->elementSize;
	}
}

struct bauma_HashMapBucketHdr_ {
	size_t size;
	size_t capacity;
	/* followed by dynamically allocated array of [hashCode, key, value], [hashCode, key, value], ... */
};

typedef union bauma_HashMapBucketHdrSize_ {
	bauma_HashMapBucketHdr_ h;
	bauma_max_align_t a;
} bauma_HashMapBucketHdrSize_;

#define BAUMA_HASHMAP_BUCKET_HDR_SIZE sizeof(bauma_HashMapBucketHdrSize_)

BAUMA_CCAL_DEF void BAUMA_DEBUG_SUFFIX(bauma_HashMap_construct_impl)(
	bauma_HashMap *pSelf,
	size_t keySize,
	size_t valueSize,
	bauma_pDestructor pKeyDestructor,
	bauma_pDestructor pValueDestructor,
	bauma_pHashFunction pKeyHasher,
	bauma_pEqualsFunction pKeyEquals,
	bauma_IMemAllocator *pAlloc
	BAUMA_DEBUG_OPT_PARAM(const char* pKeyDataType)
	BAUMA_DEBUG_OPT_PARAM(const char* pValueDataType)
) {
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(keySize > 0);
	bauma_ccal_assert(valueSize > 0);
	bauma_ccal_assert(pKeyHasher != NULL);
	bauma_ccal_assert(pKeyEquals != NULL);
	bauma_ccal_assert(pAlloc != NULL);
	pSelf->pBuckets = NULL;
	pSelf->numOfBuckets = 0;
	pSelf->size = 0;
	pSelf->keySize = keySize;
	pSelf->valueSize = valueSize;
	/* data structure stored in the buckets: dynamic array of [hashCode, key, value] */
	pSelf->keyOffset = bauma_memgap(sizeof(size_t) /* hash code */, pSelf->keySize);
	pSelf->valueOffset = bauma_memgap(pSelf->keyOffset + pSelf->keySize, pSelf->valueSize);
	pSelf->hashKeyValueStructSize = bauma_memgap(pSelf->valueOffset + pSelf->valueSize, sizeof(size_t) /* next hashCode */);
	pSelf->pKeyDestructor = pKeyDestructor;
	pSelf->pValueDestructor = pValueDestructor;
	pSelf->pKeyHash = pKeyHasher;
	pSelf->pKeyEquals = pKeyEquals;
	pSelf->pAlloc = pAlloc;
#if BAUMA_DEBUG
	pSelf->pKeyDataType = pKeyDataType;
	pSelf->pValueDataType = pValueDataType;
#endif
}

BAUMA_CCAL_DEF void bauma_HashMap_destruct(bauma_HashMap *pSelf) {
	size_t i;
	bauma_ccal_assert(pSelf != NULL);
	bauma_HashMap_clear(pSelf);
	for(i=0; i<pSelf->numOfBuckets; ++i) {
		bauma_HashMapBucketHdr_* pHdr;
		pHdr = pSelf->pBuckets[i];
		if (pHdr == NULL) continue;
		(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pHdr, 0, NULL);
	}
	(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pBuckets, 0, NULL);
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BAUMA_CCAL_DEF void bauma_HashMap_clear(bauma_HashMap* pSelf) {
	size_t i;
	bauma_ccal_assert(pSelf != NULL);
	for(i=0; i<pSelf->numOfBuckets; ++i) {
		bauma_HashMapBucketHdr_* pHdr;
		pHdr = pSelf->pBuckets[i];
		if (pHdr == NULL) continue;
		if ((pSelf->pKeyDestructor != NULL) || (pSelf->pValueDestructor != NULL)) {
			char* p;
			size_t j;
			p = ((char*)pHdr) + BAUMA_HASHMAP_BUCKET_HDR_SIZE;
			for (j=0; j<pHdr->size; ++j) {
				if (pSelf->pKeyDestructor != NULL) {
					(*pSelf->pKeyDestructor)(p + pSelf->keyOffset);
				}
				if (pSelf->pValueDestructor != NULL) {
					(*pSelf->pValueDestructor)(p + pSelf->valueOffset);
				}
				p += pSelf->hashKeyValueStructSize;
			}
		}
		pHdr->size = 0;
	}
	pSelf->size = 0;
}

#define BAUMA_HASHMAP_BUCKET_SIZE 8u

BAUMA_CCAL_DEF void bauma_HashMap_reserve(bauma_HashMap *pSelf, size_t num) {
	size_t newNumOfBuckets;
	size_t realSize;
	size_t i;
	bauma_HashMapBucketHdr_ **pNewBuckets;
	bauma_ccal_assert(pSelf != NULL);
	if ((pSelf->size + num) <= (pSelf->numOfBuckets * BAUMA_HASHMAP_BUCKET_SIZE)) return;
	newNumOfBuckets = (pSelf->numOfBuckets == 0) ? BAUMA_INITIAL_CAPACITY : pSelf->numOfBuckets;
	while((pSelf->size + num) > (newNumOfBuckets * BAUMA_HASHMAP_BUCKET_SIZE)) {
		newNumOfBuckets *= BAUMA_CAPACITY_GROWTH;
	}
	pNewBuckets = (bauma_HashMapBucketHdr_**)(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, NULL, newNumOfBuckets * sizeof(bauma_HashMapBucketHdr_*), &realSize);
	newNumOfBuckets = realSize / sizeof(bauma_HashMapBucketHdr_*);
	memset(pNewBuckets, 0, newNumOfBuckets * sizeof(bauma_HashMapBucketHdr_*));
	for (i=0; i<pSelf->numOfBuckets; ++i) {
		bauma_HashMapBucketHdr_ *pBucket;
		char *p;
		size_t j;
		pBucket = pSelf->pBuckets[i];
		if (pBucket == NULL) continue;
		p = ((char*)pBucket) + BAUMA_HASHMAP_BUCKET_HDR_SIZE;
		for (j=0; j<pBucket->size; ++j) {
			size_t bucketIdx;
			bauma_HashMapBucketHdr_ *pNewBucket;
			size_t hashCode = *(size_t*)p;
			char *q;
			bucketIdx = hashCode % newNumOfBuckets;
			pNewBucket = pNewBuckets[bucketIdx];
			if ((pNewBucket == NULL) || (pNewBucket->size >= pNewBucket->capacity)) {
				bauma_bool_t wasNull = (pNewBucket == NULL);
				size_t newBucketCapacity = wasNull ? BAUMA_HASHMAP_BUCKET_SIZE : (pNewBucket->capacity * BAUMA_CAPACITY_GROWTH);
				pNewBucket = (bauma_HashMapBucketHdr_*)(pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pNewBucket, BAUMA_HASHMAP_BUCKET_HDR_SIZE + (newBucketCapacity * pSelf->hashKeyValueStructSize), &realSize);
				realSize -= BAUMA_HASHMAP_BUCKET_HDR_SIZE;
				realSize /= pSelf->hashKeyValueStructSize;
				if (wasNull) pNewBucket->size = 0;
				pNewBucket->capacity = realSize;
				pNewBuckets[bucketIdx] = pNewBucket;
			}
			q = ((char*)pNewBucket) + BAUMA_HASHMAP_BUCKET_HDR_SIZE + (pNewBucket->size * pSelf->hashKeyValueStructSize);
			memcpy(q, p, pSelf->hashKeyValueStructSize);
			++pNewBucket->size;
			p += pSelf->hashKeyValueStructSize;
		}
		(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pBucket, 0, NULL);
	}
	(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pBuckets, 0, NULL);
	pSelf->pBuckets = pNewBuckets;
	pSelf->numOfBuckets = newNumOfBuckets;
}

BAUMA_CCAL_DEF bauma_bool_t BAUMA_DEBUG_SUFFIX(bauma_HashMap_put_impl)(
	bauma_HashMap *pSelf,
	void *pKey,
	void *pValue
	BAUMA_DEBUG_OPT_PARAM(const char* pKeyDataType)
	BAUMA_DEBUG_OPT_PARAM(const char* pValueDataType)
) {
	size_t hashCode;
	size_t bucketIdx;
	bauma_HashMapBucketHdr_ *pBucket;
	char *p;
	size_t i;
	size_t realSize;
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(pKey != NULL);
	bauma_ccal_assert(pValue != NULL);
#if BAUMA_DEBUG
	bauma_ccal_assert(strcmp(pKeyDataType, pSelf->pKeyDataType) == 0);
	bauma_ccal_assert(strcmp(pValueDataType, pSelf->pValueDataType) == 0);
#endif
	bauma_HashMap_reserve(pSelf, 1u);
	hashCode = (*pSelf->pKeyHash)(pKey);
	bucketIdx = hashCode % pSelf->numOfBuckets;
	pBucket = pSelf->pBuckets[bucketIdx];
	if (pBucket == NULL) {
		pBucket = (bauma_HashMapBucketHdr_*)(pSelf->pAlloc->pRealloc)(pSelf->pAlloc, NULL, BAUMA_HASHMAP_BUCKET_HDR_SIZE + (BAUMA_HASHMAP_BUCKET_SIZE * pSelf->hashKeyValueStructSize), &realSize);
		realSize -= BAUMA_HASHMAP_BUCKET_HDR_SIZE;
		realSize /= pSelf->hashKeyValueStructSize;
		pBucket->size = 0;
		pBucket->capacity = realSize;
		pSelf->pBuckets[bucketIdx] = pBucket;
	}
	p = ((char*)pBucket) + BAUMA_HASHMAP_BUCKET_HDR_SIZE;
	for (i = 0; i < pBucket->size; ++i) {
		if ((*(size_t*)p == hashCode) &&
		    (*pSelf->pKeyEquals)(p + pSelf->keyOffset, pKey)) {
			/* Key already exists, update value */
			if (pSelf->pKeyDestructor != NULL) {
				(*pSelf->pKeyDestructor)(pKey);
			}
			if (pSelf->pValueDestructor != NULL) {
				(*pSelf->pValueDestructor)(p + pSelf->valueOffset);
			}
			memcpy(p + pSelf->valueOffset, pValue, pSelf->valueSize);
			return BAUMA_FALSE;
		}
		p += pSelf->hashKeyValueStructSize;
	}
	if (pBucket->size >= pBucket->capacity) {
		size_t newCapacity = pBucket->capacity * BAUMA_CAPACITY_GROWTH;
		pBucket = (bauma_HashMapBucketHdr_*)(pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pBucket, BAUMA_HASHMAP_BUCKET_HDR_SIZE + (newCapacity * pSelf->hashKeyValueStructSize), &realSize);
		realSize -= BAUMA_HASHMAP_BUCKET_HDR_SIZE;
		realSize /= pSelf->hashKeyValueStructSize;
		pBucket->capacity = realSize;
		pSelf->pBuckets[bucketIdx] = pBucket;
	}
	p = ((char*)pBucket) + BAUMA_HASHMAP_BUCKET_HDR_SIZE + (pBucket->size * pSelf->hashKeyValueStructSize);
	*(size_t*)p = hashCode;
	memcpy(p + pSelf->keyOffset, pKey, pSelf->keySize);
	memcpy(p + pSelf->valueOffset, pValue, pSelf->valueSize);
	++pBucket->size;
	++pSelf->size;
	return BAUMA_TRUE;
}
		
BAUMA_CCAL_DEF void* BAUMA_DEBUG_SUFFIX(bauma_HashMap_get_impl)(
	bauma_HashMap *pSelf,
	const void *pKey
	BAUMA_DEBUG_OPT_PARAM(const char* pKeyDataType)
	BAUMA_DEBUG_OPT_PARAM(const char* pValueDataType)
) {
	size_t hashCode;
	bauma_HashMapBucketHdr_ *pBucket;
	char *p;
	size_t i;
	bauma_ccal_assert(pSelf != NULL);
#if BAUMA_DEBUG
	bauma_ccal_assert(strcmp(pKeyDataType, pSelf->pKeyDataType) == 0);
	bauma_ccal_assert(strcmp(pValueDataType, pSelf->pValueDataType) == 0);
#endif
	if (pSelf->numOfBuckets == 0) return NULL;
	hashCode = (*pSelf->pKeyHash)(pKey);
	pBucket = pSelf->pBuckets[hashCode % pSelf->numOfBuckets];
	if (pBucket == NULL) return NULL;
	p = ((char*)pBucket) + BAUMA_HASHMAP_BUCKET_HDR_SIZE;
	for (i=0; i<pBucket->size; ++i) {
		
		if ((*(size_t*)p == hashCode) &&
		    (*pSelf->pKeyEquals)(p + pSelf->keyOffset, pKey)) {
			return p + pSelf->valueOffset;
		}
		p += pSelf->hashKeyValueStructSize;
	}
	return NULL;
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

BAUMA_CCAL_DEF void bauma_StringBuilder_construct_ext(bauma_StringBuilder *pSelf, bauma_IMemAllocator *pAlloc) {
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(pAlloc != NULL);
	pSelf->pStr = BAUMA_NULLSTR;
	pSelf->size = 0;
	pSelf->capacity = 0;
	pSelf->pAlloc = pAlloc;
}

BAUMA_CCAL_DEF void bauma_StringBuilder_destruct(bauma_StringBuilder *pSelf) {
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(pSelf->pAlloc != NULL);
	if (pSelf->pStr != BAUMA_NULLSTR) {
		(*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pStr, 0, NULL);
	}
#if BAUMA_DEBUG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BAUMA_CCAL_DEF char *bauma_StringBuilder_release(bauma_StringBuilder *pSelf) {
	char *p;
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(pSelf->pStr != NULL);
	bauma_ccal_assert(pSelf->pStr[pSelf->size] == '\0');
	bauma_ccal_assert(pSelf->pAlloc != NULL);
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

BAUMA_CCAL_DEF void bauma_StringBuilder_reserve(bauma_StringBuilder *pSelf, size_t num) {
	size_t newCap;
	size_t realNewCap;
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(pSelf->pAlloc != NULL);
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
	bauma_ccal_assert(pSelf->pAlloc != NULL);
	if (pSelf->pStr == BAUMA_NULLSTR) {
		pSelf->pStr = (*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, NULL, newCap + 1u, &realNewCap);
		pSelf->pStr[0] = '\0';
	}
	else {
		pSelf->pStr = (*pSelf->pAlloc->pRealloc)(pSelf->pAlloc, pSelf->pStr, newCap + 1u, &realNewCap);
	}
	pSelf->capacity = realNewCap - 1u;
}

BAUMA_CCAL_DEF void bauma_StringBuilder_appendGeneric(bauma_StringBuilder *pSelf, size_t capacityIncrease, const char *pFmt, ...) {
	va_list ap;
	int numPrinted;
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(capacityIncrease > 0);
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
	bauma_ccal_assert((size_t)numPrinted < capacityIncrease);
	if ((size_t)numPrinted >= capacityIncrease) {
		bauma_exit_err("Too small capacityIncrease in bauma_StringBuilder_appendGeneric, memory may be corrupted");
	}
	pSelf->size += (size_t)numPrinted;
}

BAUMA_CCAL_DEF void bauma_StringBuilder_appendStr(bauma_StringBuilder *pSelf, const char *p) {
	size_t l;
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(p != NULL);
	l = strlen(p);
	bauma_StringBuilder_reserve(pSelf, l);
	memcpy(pSelf->pStr + pSelf->size, p, l + 1u);
	pSelf->size += l;
}

BAUMA_CCAL_DEF void bauma_StringBuilder_appendStrWithLen(bauma_StringBuilder *pSelf, const char *p, size_t len) {
	bauma_ccal_assert(pSelf != NULL);
	bauma_ccal_assert(p != NULL);
	bauma_StringBuilder_reserve(pSelf, len);
	memcpy(pSelf->pStr + pSelf->size, p, len);
	pSelf->size += len;
	pSelf->pStr[pSelf->size] = '\0';
}

BAUMA_CCAL_DEF void bauma_StringBuilder_appendChar(bauma_StringBuilder *pSelf, int c, size_t count) {
	size_t i;
	char* p;
	bauma_ccal_assert(pSelf != NULL);
	bauma_StringBuilder_reserve(pSelf, count);
	p = pSelf->pStr + pSelf->size;
	for (i=0; i<count; ++i) {
		*p++ = (char)((unsigned char)c);
	}
	pSelf->size += count;
	pSelf->pStr[pSelf->size] = '\0';
}

BAUMA_CCAL_DEF void bauma_StringBuilder_appendSigned(bauma_StringBuilder *pSelf, bauma_intmax_t i) {
	bauma_StringBuilder_appendGeneric(pSelf, BAUMA_UINTMAX_MAX_DECIMAL_LENGTH, "%" BAUMA_INTMAX_PREFIX "d", i);
}

BAUMA_CCAL_DEF void bauma_StringBuilder_appendUnsigned(bauma_StringBuilder *pSelf, bauma_uintmax_t i) {
	bauma_StringBuilder_appendGeneric(pSelf, BAUMA_UINTMAX_MAX_DECIMAL_LENGTH, "%" BAUMA_INTMAX_PREFIX "u", i);
}

BAUMA_CCAL_DEF void bauma_StringBuilder_appendDouble(bauma_StringBuilder *pSelf, double d) {
	bauma_StringBuilder_appendGeneric(pSelf, BAUMA_DOUBLE_MAX_DECIMAL_LEN, "%." BAUMA_DOUBLE_ROUNDTRIP_PRECISION_STR "g" , d);
}

BAUMA_CCAL_DEF void bauma_StringBuilder_appendBool(bauma_StringBuilder *pSelf, bauma_bool_t b) {
	if (b) {
		bauma_StringBuilder_appendStrWithLen(pSelf, "true", 4u);
	}
	else {
		bauma_StringBuilder_appendStrWithLen(pSelf, "false", 5u);
	}
}

BAUMA_CCAL_DEF void bauma_StringBuilder_appendCodePointUtf8(bauma_StringBuilder *pSelf, unsigned long codePoint) {
	unsigned char c[4];
	size_t l;
	bauma_ccal_assert(codePoint <= 0x10FFFF);
	if (codePoint <= 0x7F) {
		c[0] = (unsigned char)(codePoint);
		l = 1u;
	}
	else if (codePoint <= 0x7FF) {
		c[0] = (unsigned char)(0xC0 | (codePoint >> 6));
		c[1] = (unsigned char)(0x80 | (codePoint & 0x3F));
		l = 2u;
	}
	else if (codePoint <= 0xFFFF) {
		c[0] = (unsigned char)(0xE0 | (codePoint >> 12));
		c[1] = (unsigned char)(0x80 | ((codePoint >> 6) & 0x3F));
		c[2] = (unsigned char)(0x80 | (codePoint & 0x3F));
		l = 3u;
	}
	else if (codePoint <= 0x10FFFF) {
		c[0] = (unsigned char)(0xF0 | (codePoint >> 18));
		c[1] = (unsigned char)(0x80 | ((codePoint >> 12) & 0x3F));
		c[2] = (unsigned char)(0x80 | ((codePoint >> 6) & 0x3F));
		c[3] = (unsigned char)(0x80 | (codePoint & 0x3F));
		l = 4;
	}
	else {
		l = 0;
	}
	bauma_StringBuilder_appendStrWithLen(pSelf, (const char*)&c[0], l);
}

BAUMA_CCAL_DEF void bauma_StringBuilder_clear(bauma_StringBuilder *pSelf) {
	bauma_ccal_assert(pSelf != NULL);
	if (pSelf->capacity > 0) {
		pSelf->pStr[0] = '\0';
		pSelf->size = 0;
	}

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
	BAUMA_EXPECT(sb.pStr == BAUMA_NULLSTR);
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
	BAUMA_EXPECT(sb.pStr == BAUMA_NULLSTR);
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

void test_hashMap_constructDestruct(void) {
	bauma_HashMap h;
	bauma_HashMap_construct(&h, char*, char*, &bauma_memblock_destructor, &bauma_memblock_destructor, &bauma_strHasher, &bauma_strEquals);
	bauma_HashMap_destruct(&h);
}

void test_hashMap_putGet(void) {
	bauma_HashMap h;
	char *pKey;
	char *pValue;
	const char** ppFoundValue;
	bauma_bool_t isNewValue;
	bauma_HashMap_construct(&h, char*, char*, &bauma_memblock_destructor, &bauma_memblock_destructor, &bauma_strHasher, &bauma_strEquals);
	pKey = bauma_strdup("Hello");
	pValue = bauma_strdup("Hallo");
	isNewValue = bauma_HashMap_put(&h, &pKey, &pValue, char*, char*);
	BAUMA_EXPECT(isNewValue);
	pKey = bauma_strdup("Goodbye");
	pValue = bauma_strdup("Auf Wiedersehen");
	isNewValue = bauma_HashMap_put(&h, &pKey, &pValue, char*, char*);
	BAUMA_EXPECT(isNewValue);
	pKey = bauma_strdup("Hello");
	pValue = bauma_strdup("NewValue");
	isNewValue = bauma_HashMap_put(&h, &pKey, &pValue, char*, char*);
	BAUMA_EXPECT(!isNewValue);
	pKey = (char*)"Hello";
	ppFoundValue = bauma_HashMap_get(&h, &pKey, char*, char*);
	BAUMA_EXPECT(ppFoundValue != NULL);
	BAUMA_EXPECT(strcmp(*ppFoundValue, "NewValue") == 0);
	pKey = (char*)"Goodbye";
	ppFoundValue = bauma_HashMap_get(&h, &pKey, char*, char*);
	BAUMA_EXPECT(ppFoundValue != NULL);
	BAUMA_EXPECT(strcmp(*ppFoundValue, "Auf Wiedersehen") == 0);
	pKey = (char*)"Something else";
	ppFoundValue = bauma_HashMap_get(&h, &pKey, char*, char*);
	BAUMA_EXPECT(ppFoundValue == NULL);
	bauma_HashMap_destruct(&h);
}

static size_t test_intHasher(const void* p) {
	return (size_t)(*(const int*)p);
}

static bauma_bool_t test_intEquals(const void* pLhs, const void* pRhs) {
	return (*(const int*)pLhs) == (*(const int*)pRhs);
}

void test_hashMap_manyElements(void) {
	bauma_HashMap h;
	int key, value, *pFoundValue;
	size_t i;
	size_t num = 10000;
	bauma_HashMap_construct(&h, int, int, NULL, NULL, &test_intHasher, &test_intEquals);
	for (i=0; i<num; ++i) {
		key = (int)i;
		value = (int)(i);
		bauma_HashMap_put(&h, &key, &value, int, int);
	}
	BAUMA_EXPECT(bauma_HashMap_getSize(&h) == num);
	for (i=0; i<num; ++i) {
		key = (int)i;
		pFoundValue = bauma_HashMap_get(&h, &key, int, int);
		BAUMA_EXPECT(pFoundValue != NULL);
		BAUMA_EXPECT(*pFoundValue == (int)i);
	}
	bauma_HashMap_destruct(&h);
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
	BAUMA_TEST(test_hashMap_constructDestruct);
	BAUMA_TEST(test_hashMap_putGet);
	BAUMA_TEST(test_hashMap_manyElements);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BAUMA_CCAL_TEST */

#endif /* BAUMA_CCAL_H_INCLUDED */
