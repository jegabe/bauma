#ifndef BMA_CCAL_H_INCLUDED
#define BMA_CCAL_H_INCLUDED

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

- bma_Vec: Powerful, general purpose dynamic array, also nestable in other containers

How to include in other projects:

This is an stb-style C header only library.
Just download this file and #include <bauma/ccal.h> and you're done when only declarations
are needed.
When also the function definitions are needed, define the macro BMA_CCAL_IMPL
and #include <bauma/ccal.h> after that, but only in one source file. Or just pass ccal.h to
a compiler as C source file and pass "-DBMA_CCAL_IMPL" which will give an object file
containg the function definitions, which can be linked afterwards.

*/

#include <stddef.h> /* for size_t, NULL and stuff */
#include <limits.h> /* For size limits */

/* Define this before including when something else is needed */
#ifndef BMA_DBG
	#ifdef NDEBUG /* Standard way of detecting releas build */
		#define BMA_DBG 0 /* release build */
	#else
		#define BMA_DBG 1 /* debug build */
	#endif
#endif

/* Detect old vs. new compilers */
#if defined(__cplusplus) && (__cplusplus >= 201103L)
	#define BMA_MDRN_C 1
#elif (!defined(__cplusplus)) && (__STDC_VERSION__ >= 199901L)
	#define BMA_MDRN_C 1
#else
	#define BMA_MDRN_C 0 /* old compilers */
#endif

/* Find fitting bool data type */
#ifdef __cplusplus
	typedef bool bma_bool_t;
#else /* C */
	#if BMA_MDRN_C
		typedef _Bool bma_bool_t;
	#else /* older C compiler */
		typedef unsigned char bma_bool_t;
	#endif
#endif

#define BMA_TRUE  ((bma_bool_t)1)
#define BMA_FALSE ((bma_bool_t)0)

#define bma_bool2str(x) ((x) ? "true" : "false")

/* Find fitting (u)intmax_t data type */
#if BMA_MDRN_C
	#include <stdint.h>
	typedef intmax_t bma_intmax_t;
	typedef uintmax_t bma_uintmax_t;
	#if UINTMAX_MAX == UINT_MAX
		#define BMA_INTMAX_PRFX ""
		#define BMA_INTMAX_MIN INT_MIN
		#define BMA_INTMAX_MAX INT_MAX
		#define BMA_UINTMAX_MAX UINT_MAX
	#elif UINTMAX_MAX == ULONG_MAX
		#define BMA_INTMAX_PRFX "l"
		#define BMA_INTMAX_MIN LONG_MIN
		#define BMA_INTMAX_MAX LONG_MAX
		#define BMA_UINTMAX_MAX ULONG_MAX
	#elif UINTMAX_MAX == ULLONG_MAX
		#define BMA_INTMAX_PRFX "ll"
		#define BMA_INTMAX_MIN LLONG_MIN
		#define BMA_INTMAX_MAX LLONG_MAX
		#define BMA_UINTMAX_MAX ULLONG_MAX
	#else
		#error "Can't detect printf prefix for bma_(u)intmax_t"
	#endif
	#if UINTMAX_MAX > 0xFFFFFFFFFFFFFFFF
		#error "Can't handle UINTMAX_MAX greater than 64 bits, please fix"
	#elif UINTMAX_MAX >= 0xFFFFFFFFFFFFFFFF
		#define BMA_UINTMAX_MAX_DEC_LEN 20u
	#elif UINTMAX_MAX >= 0xFFFFFFFF
		#define BMA_UINTMAX_MAX_DEC_LEN 11u
	#endif
#else /* older compiler */
	/* This is a guess but usually those types reflect
		the maximum */
	typedef long bma_intmax_t;
	typedef unsigned long bma_uintmax_t;
	#define BMA_INTMAX_PRFX "l"
	#define BMA_INTMAX_MIN LONG_MIN
	#define BMA_INTMAX_MAX LONG_MAX
	#define BMA_UINTMAX_MAX ULONG_MAX
	#if ULONG_MAX > 0xFFFFFFFFFFFFFFFF
		#error "Can't handle UINTMAX_MAX greater than 64 bits, please fix"
	#elif ULONG_MAX >= 0xFFFFFFFFFFFFFFFF
		#define BMA_UINTMAX_MAX_DEC_LEN 20u
	#elif ULONG_MAX >= 0xFFFFFFFF
		#define BMA_UINTMAX_MAX_DEC_LEN 11u
	#endif
#endif

/* Find type with max. alignment requirements*/
#ifdef __cplusplus
	#if BMA_MDRN_C /* C++11 or newer */
		typedef max_align_t bma_max_align_t;
	#else
		typedef double bma_max_align_t;
	#endif
#else /* C */
	#if (__STDC_VERSION__ >= 201112L)
		#ifdef _MSC_VER
			/* Visual Studio misses that */
			typedef double bma_max_align_t;
		#else
			typedef max_align_t bma_max_align_t;
		#endif
	#else
		typedef double bma_max_align_t;
	#endif
#endif

/*! For %g to printf double' that don't loose precision when re-parsed */
#define BMA_DBL_DEC_PRECSN_STR "17"

/*!
When printf'ing a double with %.17g, this is the max. num of characters put out
inclusive a possible null terminator.
Example: "-9.999999999999999e+308"
*/
#define BMA_DBL_PRNT_NUM 25u

/*
Define this before including when something else is needed,
such as "static inline" or "declspec(dllimport)"
*/
#ifndef BMA_DEF
	#define BMA_DEF
#endif

#ifdef __cplusplus
	extern "C" {
#endif

/*! Initial dynamic array cap for various containers */
#define BMA_INIT_CAP 16u

/*! Capacity increase for various containers */
#define bma_cap_incr(cap) ((cap) + ((cap) / 2u)) /* cap growth 1.5 */

#define bma_min(a, b) ((a) < (b) ? (a) : (b))
#define bma_max(a, b) ((a) < (b) ? (a) : (b))

typedef struct bma_IMemAlloc {
	void *(*pRllc)(void *pSelf,
	               void *pOld,
	               size_t newSz,
	               size_t *pOptRealSz);
} bma_IMemAlloc;

extern const bma_IMemAlloc bma_dfltMemAlloc;

#define bma_getDfltMemAlloc() \
	((bma_IMemAlloc* const)&bma_dfltMemAlloc)

#define bma_malloc_ext(pAlloc, datatype) \
	((datatype*)((*(pAlloc)->pRllc)((pAlloc), NULL, sizeof(datatype), NULL)))

#define bma_malloc(datatype) bma_malloc_ext(bma_getDfltMemAlloc(), datatype)

#define bma_free_ext(pAlloc, p) \
	((void)((*(pAlloc)->pRllc)(pAlloc, (p), 0, NULL)))

#define bma_free(p) bma_free_ext(bma_getDfltMemAlloc(), (p))

BMA_DEF void *bma_realloc_impl(bma_IMemAlloc *pAlloc, void *pOld, size_t typeSize, size_t newCnt, size_t *pOptRealNewCnt);

#define bma_realloc_ext(pAlloc, type, pOld, newCnt, pRealNewCnt) \
	(type*)bma_realloc_impl((pAlloc), (void*)(pOld), sizeof(type), (newCnt), (pRealNewCnt))

#define bma_realloc(pOld, type, newCnt) \
	(type*)bma_realloc_impl(bma_getDfltMemAlloc(), (void*)(pOld), sizeof(type), (newCnt), NULL)


typedef void (*bma_dtor_t)(void *p, bma_IMemAlloc *pOptAlloc);

typedef void (*bma_each_t)(void *pCurrentElement, void *pOptUsrDta);

typedef size_t (*bma_hash_t)(const void *pKey);

typedef bma_bool_t (*bma_eq_t)(const void *pLhs, const void *pRhs);

/*! \brief Prints error message to stderr and calls abort() to terminate the application */
BMA_DEF void bma_exit_err(const char *msg);

/*!
Returns the proper memory alignment for a data type of known size.
For char, this will return 1 and so on. The maximum possible value is
the alignment of bma_memalign_t. The return value is guaranteed to
be a power of two.
 */
BMA_DEF size_t bma_alignof(size_t typeSz);

/*!
Does run-time structure alignment (insert gaps where needed) to fit a new data type
at a known offset by conditionally increasing that offset.
Example for a compiler-generated "gapped" structure:
\code
struct X { char a, int b; };
\endcode  
If that gaps need to be calculated at run-time rather than compile time,
bma_memgap() can be called
with bma_memgap(sizeof(char), sizeof(int)) and will return a suitable offset
to fit an int after a char. In this example, 3 or 7 will be added depending
of the size of int and on the machine word width. The result is not a gap value
but the absolute offset.
\param offset A data structure offset (from beginning) in bytes
\param followingTypeSize Size of a data type that has to be put at that offset
\return Adjusted offset which might contain gap bytes
*/
BMA_DEF size_t bma_memgap(size_t offset, size_t followingTypeSize);

/*!
Can be used as element destructor (e.g. for vectors) when the element
type is just a pointer to a memory block allocated with some allocator.
De-references the pointer (after casting it to void**) an calls
the passed allocator on the de-referenced pointer to free the memory
block. Usage example: bma_Vec constructed with "char*" as data type where
each element is a heap-allocated string (e.g. created by bma_strdup).
ATTENTION: This is a very simple implementation; For more complex elements that
also need destruction besides freeing of memory, this function might not do enough.
So this is just for memory blocks containing plain old data.
 */
BMA_DEF void bma_memblck_dtor(void* ppMemBlock, bma_IMemAlloc *pAlloc);

BMA_DEF size_t bma_strhash(const void *ppStr);

BMA_DEF bma_bool_t bma_streq(const void* ppLhs, const void *ppRhs);

/*
\brief Allocates a copy of the passed null-terminated string and returns it
\param p A null-terminated string
\param alloc Memory allocator to be used
*/
BMA_DEF char* bma_strdup_ext(const char* p, bma_IMemAlloc *pAlloc);

/*!
\brief Short-hand of bma_strdup_ext() using the default allocator
\param p A null-terminated string
*/
#define bma_strdup(p) bma_strdup_ext(p, bma_getDfltMemAlloc())

/*
\brief Allocates a copy of the passed null-terminated string and returns it
\param p A null-terminated string
\param alloc Memory allocator to be used
*/
BMA_DEF char *bma_strndup_ext(const char* p, size_t n, bma_IMemAlloc *pAlloc);

/*!
\brief Short-hand of bma_strdup_ext() using the default allocator
\param p A null-terminated string
*/
#define bma_strndup(p, l) bma_strndup_ext(p, l, bma_getDfltMemAlloc())

BMA_DEF void *bma_memmem(const void *pHayStack, size_t hayStackSize, const void *pNeedle, size_t needleSize);

/*!
\brief A string that knows its length and isn't necessarily
null terminated. Useful for hash map keys where null terminated strings
are too limiting.
Initializing is so simple that no specific ctor is provided, but
a dtor for usage inside of containers and hash/compare algos
for usage as hash map key
*/
typedef struct bma_StrN {
	char *p;
	size_t len;
} bma_StrN;

BMA_DEF size_t bma_StrN_hash(const bma_StrN *pSelf);
BMA_DEF bma_bool_t bma_StrN_eq(const bma_StrN *pLhs, const bma_StrN *pRhs);

/*
  Pre-requisite for using this: member "p" is heap-allocad with same pAlloc
*/
BMA_DEF void bma_StrN_dtor(bma_StrN *pSelf, bma_IMemAlloc *pAlloc);


typedef struct bma_Vec {
	void           *pData;
	size_t         elemSz;
	size_t         size;
	size_t         cap;
	bma_dtor_t     pElemDtor;
	bma_IMemAlloc  *pAlloc;
#if BMA_DBG
	const char     *pType;
#endif
} bma_Vec;

#if BMA_DBG
	#define BMA_DBG_SFFX(x) x ## _D
	#define BMA_DBG_OPT_PARAM(x) , x
#else
	#define BMA_DBG_SFFX(x) x
	#define BMA_DBG_OPT_PARAM(x)
#endif

BMA_DEF void BMA_DBG_SFFX(bma_Vec_ctor_impl)(
	bma_Vec *pSelf,
	size_t elemSize, 
	bma_dtor_t pElemDtor,
	bma_IMemAlloc *pAlloc
	BMA_DBG_OPT_PARAM(const char* pType)
);

#define bma_Vec_ctor_ext(pSelf, type, pElemDtor, pAlloc) \
	BMA_DBG_SFFX(bma_Vec_ctor_impl)( \
		pSelf, \
		sizeof(type), \
		pElemDtor, \
		pAlloc \
		BMA_DBG_OPT_PARAM(#type) \
	)

#define bma_Vec_ctor(pSelf, type, pElemDtor) \
	BMA_DBG_SFFX(bma_Vec_ctor_impl)( \
		pSelf, \
		sizeof(type), \
		pElemDtor, \
		bma_getDfltMemAlloc() \
		BMA_DBG_OPT_PARAM(#type) \
	)

BMA_DEF void bma_Vec_dtor(bma_Vec *pSelf, bma_IMemAlloc *pAlloc);

BMA_DEF void bma_Vec_rsrv(bma_Vec *pSelf, size_t num);

BMA_DEF void BMA_DBG_SFFX(bma_Vec_appnd_impl)(
	bma_Vec *pSelf,
	void *pElem
	BMA_DBG_OPT_PARAM(const char* pType)
);

#define bma_Vec_appnd(pSelf, type, pElem) \
	BMA_DBG_SFFX(bma_Vec_appnd_impl)( \
		pSelf, \
		pElem \
		BMA_DBG_OPT_PARAM(#type) \
	)

#if BMA_DBG
	BMA_DEF void *bma_Vec_at_impl_D(bma_Vec *pSelf, size_t index, size_t elemSz, const char *pType);
	#define bma_Vec_at(pSelf, index, type) \
		((type*)bma_Vec_at_impl_D(pSelf, index, sizeof(type), #type))
	BMA_DEF void *bma_Vec_getData_impl_D(bma_Vec *pSelf, const char *pType);
	#define bma_Vec_getData(pSelf, type) \
		((type*)bma_Vec_getData_impl_D(pSelf, #type))
#else
	#define bma_Vec_at(pSelf, index, type) \
		((type*)(((char*)(pSelf)->pData) + (index * (pSelf)->elemSz)))
	#define bma_Vec_getData(pSelf, type) \
		((type*)(pSelf)->pData)
#endif

#define bma_Vec_getSz(pSelf) ((const size_t)((pSelf)->size))
#define bma_Vec_getCap(pSelf) ((const size_t)((pSelf)->cap))
#define bma_Vec_getAlloc(pSelf) ((bma_IMemAlloc* const)((pSelf)->pAlloc))

BMA_DEF void bma_Vec_each(bma_Vec *pSelf, bma_each_t pFunc, void *pOptUsrDta);

BMA_DEF bma_bool_t BMA_DBG_SFFX(bma_Vec_rmv_impl)(bma_Vec *pSelf,
                                                  size_t index,
                                                  void *pOptRmvdElem
                                                  BMA_DBG_OPT_PARAM(const char* pType));
#define bma_Vec_rmv(pSelf, index, pOptRmvdElem, type) \
	BMA_DBG_SFFX(bma_Vec_rmv_impl)(pSelf, index, pOptRmvdElem BMA_DBG_OPT_PARAM(#type))

BMA_DEF void bma_Vec_clear(bma_Vec *pSelf);

typedef struct bma_HshMpBcktHdr_ bma_HshMpBcktHdr_;

typedef struct bma_HshMp {
	bma_HshMpBcktHdr_   **pBckts;
	size_t              numBckts;
	size_t              size;
	size_t              keySz;
	size_t              valueSz;
	size_t              keyOffs;
	size_t              valueOffs;
	size_t              bcktSz;
	bma_dtor_t          pKeyDtor;
	bma_dtor_t          pValueDtor;
	bma_hash_t          pKeyHash;
	bma_eq_t            pKeyEq;
	bma_IMemAlloc       *pAlloc;
#if BMA_DBG
	const char          *pKeyType;
	const char          *pValueType;
#endif
} bma_HshMp;

BMA_DEF void BMA_DBG_SFFX(bma_HshMp_ctor_impl)(
	bma_HshMp *pSelf,
	size_t keySz,
	size_t valueSz,
	bma_dtor_t pKeyDtor,
	bma_dtor_t pValueDtor,
	bma_hash_t pKeyHsh,
	bma_eq_t pKeyEq,
	bma_IMemAlloc *pAlloc
	BMA_DBG_OPT_PARAM(const char* pKeyType)
	BMA_DBG_OPT_PARAM(const char* pValueType)
);

#define bma_HshMp_ctor_ext(pSelf, keyType, valueType, pKeyDtor, pValueDtor, pKeyHsh, pKeyEq, pAlloc) \
	BMA_DBG_SFFX(bma_HshMp_ctor_impl)( \
		pSelf, \
		sizeof(keyType), \
		sizeof(valueType), \
		pKeyDtor, \
		pValueDtor, \
		pKeyHsh, \
		pKeyEq, \
		pAlloc \
		BMA_DBG_OPT_PARAM(#keyType) \
		BMA_DBG_OPT_PARAM(#valueType) \
	)

#define bma_HshMp_ctor(pSelf, keyType, valueType, pKeyDtor, pValueDtor, pKeyHsh, pKeyEq) \
	BMA_DBG_SFFX(bma_HshMp_ctor_impl)( \
		pSelf, \
		sizeof(keyType), \
		sizeof(valueType), \
		pKeyDtor, \
		pValueDtor, \
		pKeyHsh, \
		pKeyEq, \
		bma_getDfltMemAlloc() \
		BMA_DBG_OPT_PARAM(#keyType) \
		BMA_DBG_OPT_PARAM(#valueType) \
	)

BMA_DEF void bma_HshMp_dtor(bma_HshMp *pSelf, bma_IMemAlloc *pAlloc);

#define bma_HshMp_getSz(pSelf) ((const size_t)((pSelf)->size))

BMA_DEF bma_bool_t BMA_DBG_SFFX(bma_HshMp_put_impl)(
	bma_HshMp *pSelf,
	void *pKey,
	void *pValue
	BMA_DBG_OPT_PARAM(const char* pKeyType)
	BMA_DBG_OPT_PARAM(const char* pValueType)
);

#define bma_HshMp_put(pSelf, pKey, pValue, keyType, valueType) \
	BMA_DBG_SFFX(bma_HshMp_put_impl)(pSelf, \
						   pKey, \
						   pValue \
						   BMA_DBG_OPT_PARAM(#keyType) \
						   BMA_DBG_OPT_PARAM(#valueType))

BMA_DEF void* BMA_DBG_SFFX(bma_HshMp_get_impl)(
	bma_HshMp *pSelf,
	const void *pKey
	BMA_DBG_OPT_PARAM(const char* pKeyType)
	BMA_DBG_OPT_PARAM(const char* pValueType)
);

#define bma_HshMp_get(pSelf, pKey, keyType, valueType) \
	((valueType*)BMA_DBG_SFFX(bma_HshMp_get_impl)(pSelf, \
	                                                        pKey \
	                                                        BMA_DBG_OPT_PARAM(#keyType) \
	                                                        BMA_DBG_OPT_PARAM(#valueType)))

BMA_DEF void bma_HshMp_clear(bma_HshMp* pSelf);

typedef struct bma_StrBldr {
	char   *pStr;
	size_t size; /* not counting the null terminator */
	size_t cap;
	bma_IMemAlloc *pAlloc;
} bma_StrBldr;

BMA_DEF void bma_StrBldr_ctor_ext(bma_StrBldr *pSelf, bma_IMemAlloc *pAlloc);
#define bma_StrBldr_ctor(pSelf) bma_StrBldr_ctor_ext((pSelf), bma_getDfltMemAlloc())
BMA_DEF void bma_StrBldr_dtor(bma_StrBldr *pSelf, bma_IMemAlloc *pAlloc);
#define bma_StrBldr_getStr(pSelf) ((const char* const)((pSelf)->pStr))
#define bma_StrBldr_getSz(pSelf) ((const size_t)((pSelf)->size))
BMA_DEF char *bma_StrBldr_rlse(bma_StrBldr *pSelf);
BMA_DEF void bma_StrBldr_rsrv(bma_StrBldr *pSelf, size_t num);
BMA_DEF void bma_StrBldr_appndGnrc(bma_StrBldr *pSelf, size_t capIncrease, const char *pFmt, ...);
BMA_DEF void bma_StrBldr_appndStr(bma_StrBldr *pSelf, const char *p);
BMA_DEF void bma_StrBldr_appndStrN(bma_StrBldr *pSelf, const char *p, size_t len);
BMA_DEF void bma_StrBldr_appndChr(bma_StrBldr *pSelf, int c, size_t count);
BMA_DEF void bma_StrBldr_appndSgnd(bma_StrBldr *pSelf, bma_intmax_t i);
BMA_DEF void bma_StrBldr_appndUnsgnd(bma_StrBldr *pSelf, bma_uintmax_t i);
BMA_DEF void bma_StrBldr_appndDbl(bma_StrBldr *pSelf, double d);
BMA_DEF void bma_StrBldr_appndBool(bma_StrBldr *pSelf, bma_bool_t b);
BMA_DEF void bma_StrBldr_appndCdPntUtf8(bma_StrBldr *pSelf, unsigned long cdPnt);
BMA_DEF void bma_StrBldr_clear(bma_StrBldr *pSelf);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_CCAL_IMPL

#include <string.h> /* for all kind of memory and string things */
#include <stdarg.h> /* for variadic functions */
#ifdef bma_ccal_cstm_assert
	#define bma_assert(x) bma_ccal_cstm_assert(x)
#else
	#include <assert.h>
	#define bma_assert(x) assert(x)
#endif
#ifndef bma_cstm_memhndlr
	#include <stdlib.h> /* for malloc(), free(), exit() etc. */
#endif
#ifndef bma_cstm_exit_err
	#include <stdio.h> /* to print to stderr before leaving */
	#include <stdlib.h> /* for malloc(), free(), exit() etc. */
#endif

#ifdef __cplusplus
	extern "C" {
#endif

BMA_DEF void *bma_realloc_impl(bma_IMemAlloc *pAlloc, void *pOld, size_t typeSize, size_t newCnt, size_t *pOptRealNewCnt) {
	size_t realSize = 0;
	void *pNew;
	bma_assert(pAlloc != NULL);
	pNew = (*pAlloc->pRllc)(pAlloc, pOld, typeSize * newCnt, &realSize);
	if (pOptRealNewCnt != NULL) *pOptRealNewCnt = realSize / typeSize;
	return pNew;
}

typedef struct bma_AlgnFindr_ {
	char c;
	bma_max_align_t a;
} bma_AlgnFindr_;

#define BMA_MAX_ALGNMT offsetof(bma_AlgnFindr_, a)

BMA_DEF bma_bool_t bma_ispow2(size_t value) {
	return (bma_bool_t)((value > 0) && !(value & (value - 1)));
}

BMA_DEF size_t bma_alignof(size_t typeSz) {
	bma_assert(bma_ispow2(BMA_MAX_ALGNMT));
	bma_assert(typeSz > 0);
	if (typeSz < BMA_MAX_ALGNMT) {
		/* next-power-of-two bit twiddling */
		--typeSz;
		typeSz |= typeSz >> 1;
		typeSz |= typeSz >> 2;
		typeSz |= typeSz >> 4;
		typeSz |= typeSz >> 8;
		typeSz |= typeSz >> 16;
#if SIZE_MAX > 0xFFFFFFFF
		typeSz |= typeSz >> 32;
#endif
#if SIZE_MAX > 0xFFFFFFFFFFFFFFFF
		typeSz |= typeSz >> 64;
#endif
		++typeSz;		
		bma_assert(bma_ispow2(typeSz));
		return typeSz;
	}
	return BMA_MAX_ALGNMT;
}

BMA_DEF size_t bma_memgap(size_t offset, size_t followingTypeSize) {
	/* This is a power of two: */
	size_t alignOfFollowing = bma_alignof(followingTypeSize);
	--alignOfFollowing; /* make bit field out of it */
	offset += alignOfFollowing;
	offset &= ~alignOfFollowing;
	return offset;
}

BMA_DEF void bma_exit_err(const char *msg) {
#ifdef bma_cstm_exit_err
	bma_cstm_exit_err(msg);
#else
	fprintf(stderr, "Error: %s. Terminating.\n", msg);
	fflush(stderr);
	abort();
#endif
}

BMA_DEF void *bma_dfltMemAllocRllc(void *pSelf,
                                   void *pOld,
                                   size_t newSz,
                                   size_t *pOptRealSz) {
#ifdef bma_cstm_memhndlr
	(void)pSelf;
	bma_cstm_memhndlr(pOld, newSz, pOptRealSz);
#else
	void *p;
	(void)pSelf;
	if (pOptRealSz != NULL) {
		*pOptRealSz = 0;
	}
	if (pOld == NULL) {
		if (newSz == 0) {
			return NULL; /* Null pointer is a legitimate memory block of size zero */
		}
		p = malloc(newSz);
		if (p == NULL) {
			bma_exit_err("Out of memory");
		}
		if (pOptRealSz != NULL) {
			*pOptRealSz = newSz;
		}
		return p;
	}
	if (newSz == 0) {
		free(pOld);
		return NULL;
	}
	p = realloc(pOld, newSz);
	if (p == NULL) {
		bma_exit_err("Out of memory");
	}
	if (pOptRealSz != NULL) {
		*pOptRealSz = newSz;
	}
	return p;
#endif
}

const bma_IMemAlloc bma_dfltMemAlloc = {
	 &bma_dfltMemAllocRllc
};

BMA_DEF void bma_memblck_dtor(void* ppMemBlock, bma_IMemAlloc *pAlloc) {
	bma_assert(ppMemBlock != NULL);
	bma_assert(pAlloc != NULL);
	void* pMemBlock = *(void**)ppMemBlock;
	bma_free_ext(pAlloc, pMemBlock);
}

BMA_DEF size_t bma_strhash(const void *ppStr) {
	const char* p;
	size_t result = 0;
	bma_assert(ppStr != NULL);
	p = *(const char**)ppStr;
	bma_assert(p != 0);
	while(*p) {
		result = (31u * result) + (size_t)((unsigned char)*p);
		++p;
	}
	return result;
}

BMA_DEF bma_bool_t bma_streq(const void* ppLhs, const void *ppRhs) {
	const char* pLhs;
	const char* pRhs;
	bma_assert(ppLhs != NULL);
	bma_assert(ppRhs != NULL);
	pLhs = *(const char**)ppLhs;
	pRhs = *(const char**)ppRhs;
	return (strcmp(pLhs, pRhs) == 0);
}

BMA_DEF char* bma_strdup_ext(const char* p, bma_IMemAlloc *pAlloc) {
	size_t l;
	char *c;
	bma_assert((p != NULL) && "String must not be NULL");
	bma_assert((pAlloc != NULL) && "Allocator must not be NULL");
	l = strlen(p) + 1u; /* +1 to include the null terminator */
	c = (char*)((pAlloc->pRllc)(pAlloc, NULL, l, NULL));
	memcpy(c, p, l);
	return c;
}

BMA_DEF char* bma_strndup_ext(const char* p, size_t n, bma_IMemAlloc *pAlloc) {
	char *c;
	bma_assert((p != NULL) && "String must not be NULL");
	bma_assert((pAlloc != NULL) && "Allocator must not be NULL");
	c = (char*)((*pAlloc->pRllc)(pAlloc, NULL, n + 1u, NULL));
	memcpy(c, p, n);
	c[n] = '\0';
	return c;
}

BMA_DEF void *bma_memmem(const void *pHayStack, size_t hayStackSize, const void *pNeedle, size_t needleSize) {
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

BMA_DEF size_t bma_StrN_hash(const bma_StrN *pSelf) {
	size_t result = 0;
	size_t i;
	size_t len;
	const char* p;
	bma_assert(pSelf != NULL);
	p = pSelf->p;
	len = pSelf->len;
	bma_assert(p != 0);
	for (i=0; i<len; ++i) {
		result = (31u * result) + (size_t)((unsigned char)*p);
		++p;
	}
	return result;
}

BMA_DEF bma_bool_t bma_StrN_eq(const bma_StrN *pLhs, const bma_StrN *pRhs) {
	bma_assert(pLhs != NULL);
	bma_assert(pRhs != NULL);
	return (pLhs->len == pRhs->len) && (memcmp(pLhs->p, pRhs->p, pLhs->len) == 0);
}

/*
  Pre-requisite for using this: member "p" is heap-allocad with same pAlloc
*/
BMA_DEF void bma_StrN_dtor(bma_StrN *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_assert(pAlloc != NULL);
	bma_free_ext(pAlloc, pSelf->p);
}
		
BMA_DEF void BMA_DBG_SFFX(bma_Vec_ctor_impl)(
	bma_Vec *pSelf,
	size_t elemSize, 
	bma_dtor_t pElemDtor,
	bma_IMemAlloc *pAlloc
	BMA_DBG_OPT_PARAM(const char* pType)
) {
	bma_assert(pSelf != NULL);
	bma_assert(elemSize > 0);
	bma_assert(pAlloc != NULL);
	pSelf->pData = NULL;
	pSelf->elemSz = elemSize;
	pSelf->size = 0;
	pSelf->cap = 0;
	pSelf->pElemDtor = pElemDtor;
	pSelf->pAlloc = pAlloc;
#if BMA_DBG
	bma_assert(pType != NULL);
	pSelf->pType = pType;
#endif
}

BMA_DEF void bma_Vec_dtor(bma_Vec *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	(void)pAlloc; /* vec has its own */
	bma_Vec_clear(pSelf);
	bma_free_ext(pSelf->pAlloc, pSelf->pData);
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF void bma_Vec_rsrv(bma_Vec *pSelf, size_t num) {
	size_t newCap;
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->pAlloc != NULL);
	if ((pSelf->size + num) <= (pSelf->cap)) {
		return;
	}
	if (pSelf->cap == 0) {
		newCap = BMA_INIT_CAP;
	}
	else {
		newCap = pSelf->cap;
	}
	while((pSelf->size + num) > newCap) {
		newCap = bma_cap_incr(newCap);
	}
	pSelf->pData = bma_realloc_impl(pSelf->pAlloc, pSelf->pData, pSelf->elemSz, newCap, &pSelf->cap);
}

BMA_DEF void BMA_DBG_SFFX(bma_Vec_appnd_impl)(
	bma_Vec *pSelf,
	void *pElem
	BMA_DBG_OPT_PARAM(const char* pType)
) {
	char* p;
	bma_assert(pSelf != NULL);
	bma_assert(strcmp(pType, pSelf->pType) == 0);
	bma_Vec_rsrv(pSelf, 1u);
	p = ((char*)pSelf->pData) + (pSelf->size * pSelf->elemSz);
	memcpy(p, pElem, pSelf->elemSz);
	++pSelf->size;
}

#if BMA_DBG

BMA_DEF void *bma_Vec_at_impl_D(bma_Vec *pSelf, size_t index, size_t elemSz, const char *pType) {
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->pType != NULL);
	bma_assert(pSelf->elemSz == elemSz);
	bma_assert(strcmp(pType, pSelf->pType) == 0);
	bma_assert(index < pSelf->size);
	return ((char*)pSelf->pData) + (index * elemSz);
}

BMA_DEF void *bma_Vec_getData_impl_D(bma_Vec *pSelf, const char *pType) {
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->pType != NULL);
	bma_assert(strcmp(pType, pSelf->pType) == 0);
	return pSelf->pData;
}

#endif

BMA_DEF void bma_Vec_each(bma_Vec *pSelf, bma_each_t pFunc, void *pOptUsrDta) {
	size_t i;
	char *p;
	bma_assert(pSelf != NULL);
	bma_assert(pFunc != NULL);
	p = (char*)pSelf->pData;
	for (i=0; i<pSelf->size; ++i) {
		(*pFunc)(p, pOptUsrDta);
		p += pSelf->elemSz;
	}
}

BMA_DEF bma_bool_t BMA_DBG_SFFX(bma_Vec_rmv_impl)(bma_Vec *pSelf,
                                                  size_t index,
                                                  void *pOptRmvdElem
                                                  BMA_DBG_OPT_PARAM(const char* pType)) {
	bma_assert(pSelf != NULL);
	bma_assert(strcmp(pType, pSelf->pType) == 0);
	bma_assert(index < pSelf->size);
	if (index >= pSelf->size) {
		return BMA_FALSE;
	}
	char *pRmvd = ((char*)pSelf->pData) + (index * pSelf->elemSz);
	if (pOptRmvdElem != NULL) {
		memcpy(pOptRmvdElem, pRmvd, pSelf->elemSz);
	}
	else if (pSelf->pElemDtor != NULL) {
		(*pSelf->pElemDtor)(pRmvd, pSelf->pAlloc);
	}
	if (index < (pSelf->size - 1)) {
		char *pNext = pRmvd + pSelf->elemSz;
		size_t bytesToMove = ((pSelf->size - 1 - index) * pSelf->elemSz);
		memmove(pRmvd, pNext, bytesToMove);
	}
	--pSelf->size;
	return BMA_TRUE;
}

BMA_DEF void bma_Vec_clear(bma_Vec *pSelf) {
	bma_assert(pSelf != NULL);
	if (pSelf->pElemDtor) {
		char *p = ((char*)pSelf->pData) + (pSelf->size * pSelf->elemSz);
		size_t i = pSelf->size;
		while(i-- > 0) {
			p -= pSelf->elemSz;
			(*pSelf->pElemDtor)(p, pSelf->pAlloc);
		}
	}
	pSelf->size = 0;
}


struct bma_HshMpBcktHdr_ {
	size_t size;
	size_t cap;
	/* followed by dynamically allocated array of [hashCode, key, value], [hashCode, key, value], ... */
};

typedef union bma_HshMpBcktHrdSz_ {
	bma_HshMpBcktHdr_ h;
	bma_max_align_t a;
} bma_HshMpBcktHrdSz_;

#define BMA_HSHMP_BCKT_HDR_SZ sizeof(bma_HshMpBcktHrdSz_)

BMA_DEF void BMA_DBG_SFFX(bma_HshMp_ctor_impl)(
	bma_HshMp *pSelf,
	size_t keySz,
	size_t valueSz,
	bma_dtor_t pKeyDtor,
	bma_dtor_t pValueDtor,
	bma_hash_t pKeyHsh,
	bma_eq_t pKeyEq,
	bma_IMemAlloc *pAlloc
	BMA_DBG_OPT_PARAM(const char* pKeyType)
	BMA_DBG_OPT_PARAM(const char* pValueType)
) {
	bma_assert(pSelf != NULL);
	bma_assert(keySz > 0);
	bma_assert(valueSz > 0);
	bma_assert(pKeyHsh != NULL);
	bma_assert(pKeyEq != NULL);
	bma_assert(pAlloc != NULL);
	pSelf->pBckts = NULL;
	pSelf->numBckts = 0;
	pSelf->size = 0;
	pSelf->keySz = keySz;
	pSelf->valueSz = valueSz;
	/* data structure stored in the buckets: dynamic array of [hashCode, key, value] */
	pSelf->keyOffs = bma_memgap(sizeof(size_t) /* hash code */, pSelf->keySz);
	pSelf->valueOffs = bma_memgap(pSelf->keyOffs + pSelf->keySz, pSelf->valueSz);
	pSelf->bcktSz = bma_memgap(pSelf->valueOffs + pSelf->valueSz, sizeof(size_t) /* next hashCode */);
	pSelf->pKeyDtor = pKeyDtor;
	pSelf->pValueDtor = pValueDtor;
	pSelf->pKeyHash = pKeyHsh;
	pSelf->pKeyEq = pKeyEq;
	pSelf->pAlloc = pAlloc;
#if BMA_DBG
	pSelf->pKeyType = pKeyType;
	pSelf->pValueType = pValueType;
#endif
}

BMA_DEF void bma_HshMp_dtor(bma_HshMp *pSelf, bma_IMemAlloc *pAlloc) {
	size_t i;
	bma_assert(pSelf != NULL);
	(void)pAlloc; /* map has its own */
	bma_HshMp_clear(pSelf);
	for(i=0; i<pSelf->numBckts; ++i) {
		bma_HshMpBcktHdr_* pHdr;
		pHdr = pSelf->pBckts[i];
		if (pHdr == NULL) continue;
		(*pSelf->pAlloc->pRllc)(pSelf->pAlloc, pHdr, 0, NULL);
	}
	bma_free_ext(pSelf->pAlloc, pSelf->pBckts);
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF void bma_HshMp_clear(bma_HshMp* pSelf) {
	size_t i;
	bma_assert(pSelf != NULL);
	for(i=0; i<pSelf->numBckts; ++i) {
		bma_HshMpBcktHdr_* pHdr;
		pHdr = pSelf->pBckts[i];
		if (pHdr == NULL) continue;
		if ((pSelf->pKeyDtor != NULL) || (pSelf->pValueDtor != NULL)) {
			char* p;
			size_t j;
			p = ((char*)pHdr) + BMA_HSHMP_BCKT_HDR_SZ;
			for (j=0; j<pHdr->size; ++j) {
				if (pSelf->pKeyDtor != NULL) {
					(*pSelf->pKeyDtor)(p + pSelf->keyOffs, pSelf->pAlloc);
				}
				if (pSelf->pValueDtor != NULL) {
					(*pSelf->pValueDtor)(p + pSelf->valueOffs, pSelf->pAlloc);
				}
				p += pSelf->bcktSz;
			}
		}
		pHdr->size = 0;
	}
	pSelf->size = 0;
}

#define BMA_HSHMP_BCKT_CNT 8u

BMA_DEF void bma_HshMp_reserve(bma_HshMp *pSelf, size_t num) {
	size_t newNumOfBuckets;
	size_t realSize;
	size_t i;
	bma_HshMpBcktHdr_ **pNewBuckets;
	bma_assert(pSelf != NULL);
	if ((pSelf->size + num) <= (pSelf->numBckts * BMA_HSHMP_BCKT_CNT)) return;
	newNumOfBuckets = (pSelf->numBckts == 0) ? BMA_INIT_CAP : pSelf->numBckts;
	while((pSelf->size + num) > (newNumOfBuckets * BMA_HSHMP_BCKT_CNT)) {
		newNumOfBuckets = bma_cap_incr(newNumOfBuckets);
	}
	pNewBuckets = (bma_HshMpBcktHdr_**)(*pSelf->pAlloc->pRllc)(pSelf->pAlloc, NULL, newNumOfBuckets * sizeof(bma_HshMpBcktHdr_*), &realSize);
	newNumOfBuckets = realSize / sizeof(bma_HshMpBcktHdr_*);
	memset(pNewBuckets, 0, newNumOfBuckets * sizeof(bma_HshMpBcktHdr_*));
	for (i=0; i<pSelf->numBckts; ++i) {
		bma_HshMpBcktHdr_ *pBucket;
		char *p;
		size_t j;
		pBucket = pSelf->pBckts[i];
		if (pBucket == NULL) continue;
		p = ((char*)pBucket) + BMA_HSHMP_BCKT_HDR_SZ;
		for (j=0; j<pBucket->size; ++j) {
			size_t bucketIdx;
			bma_HshMpBcktHdr_ *pNewBucket;
			size_t hashCode = *(size_t*)p;
			char *q;
			bucketIdx = hashCode % newNumOfBuckets;
			pNewBucket = pNewBuckets[bucketIdx];
			if ((pNewBucket == NULL) || (pNewBucket->size >= pNewBucket->cap)) {
				bma_bool_t wasNull = (pNewBucket == NULL);
				size_t newBucketCapacity = wasNull ? BMA_HSHMP_BCKT_CNT : bma_cap_incr(pNewBucket->cap);
				pNewBucket = (bma_HshMpBcktHdr_*)(pSelf->pAlloc->pRllc)(pSelf->pAlloc, pNewBucket, BMA_HSHMP_BCKT_HDR_SZ + (newBucketCapacity * pSelf->bcktSz), &realSize);
				realSize -= BMA_HSHMP_BCKT_HDR_SZ;
				realSize /= pSelf->bcktSz;
				if (wasNull) pNewBucket->size = 0;
				pNewBucket->cap = realSize;
				pNewBuckets[bucketIdx] = pNewBucket;
			}
			q = ((char*)pNewBucket) + BMA_HSHMP_BCKT_HDR_SZ + (pNewBucket->size * pSelf->bcktSz);
			memcpy(q, p, pSelf->bcktSz);
			++pNewBucket->size;
			p += pSelf->bcktSz;
		}
		bma_free_ext(pSelf->pAlloc, pBucket);
	}
	bma_free_ext(pSelf->pAlloc, pSelf->pBckts);
	pSelf->pBckts = pNewBuckets;
	pSelf->numBckts = newNumOfBuckets;
}

BMA_DEF bma_bool_t BMA_DBG_SFFX(bma_HshMp_put_impl)(
	bma_HshMp *pSelf,
	void *pKey,
	void *pValue
	BMA_DBG_OPT_PARAM(const char* pKeyType)
	BMA_DBG_OPT_PARAM(const char* pValueType)
) {
	size_t hashCode;
	size_t bucketIdx;
	bma_HshMpBcktHdr_ *pBucket;
	char *p;
	size_t i;
	size_t realSize;
	bma_assert(pSelf != NULL);
	bma_assert(pKey != NULL);
	bma_assert(pValue != NULL);
#if BMA_DBG
	bma_assert(strcmp(pKeyType, pSelf->pKeyType) == 0);
	bma_assert(strcmp(pValueType, pSelf->pValueType) == 0);
#endif
	bma_HshMp_reserve(pSelf, 1u);
	hashCode = (*pSelf->pKeyHash)(pKey);
	bucketIdx = hashCode % pSelf->numBckts;
	pBucket = pSelf->pBckts[bucketIdx];
	if (pBucket == NULL) {
		pBucket = (bma_HshMpBcktHdr_*)(pSelf->pAlloc->pRllc)(pSelf->pAlloc, NULL, BMA_HSHMP_BCKT_HDR_SZ + (BMA_HSHMP_BCKT_CNT * pSelf->bcktSz), &realSize);
		realSize -= BMA_HSHMP_BCKT_HDR_SZ;
		realSize /= pSelf->bcktSz;
		pBucket->size = 0;
		pBucket->cap = realSize;
		pSelf->pBckts[bucketIdx] = pBucket;
	}
	p = ((char*)pBucket) + BMA_HSHMP_BCKT_HDR_SZ;
	for (i = 0; i < pBucket->size; ++i) {
		if ((*(size_t*)p == hashCode) &&
		    (*pSelf->pKeyEq)(p + pSelf->keyOffs, pKey)) {
			/* Key already exists, update value */
			if (pSelf->pKeyDtor != NULL) {
				(*pSelf->pKeyDtor)(pKey, pSelf->pAlloc);
			}
			if (pSelf->pValueDtor != NULL) {
				(*pSelf->pValueDtor)(p + pSelf->valueOffs, pSelf->pAlloc);
			}
			memcpy(p + pSelf->valueOffs, pValue, pSelf->valueSz);
			return BMA_FALSE;
		}
		p += pSelf->bcktSz;
	}
	if (pBucket->size >= pBucket->cap) {
		size_t newCapacity = bma_cap_incr(pBucket->cap);
		pBucket = (bma_HshMpBcktHdr_*)(pSelf->pAlloc->pRllc)(pSelf->pAlloc, pBucket, BMA_HSHMP_BCKT_HDR_SZ + (newCapacity * pSelf->bcktSz), &realSize);
		realSize -= BMA_HSHMP_BCKT_HDR_SZ;
		realSize /= pSelf->bcktSz;
		pBucket->cap = realSize;
		pSelf->pBckts[bucketIdx] = pBucket;
	}
	p = ((char*)pBucket) + BMA_HSHMP_BCKT_HDR_SZ + (pBucket->size * pSelf->bcktSz);
	*(size_t*)p = hashCode;
	memcpy(p + pSelf->keyOffs, pKey, pSelf->keySz);
	memcpy(p + pSelf->valueOffs, pValue, pSelf->valueSz);
	++pBucket->size;
	++pSelf->size;
	return BMA_TRUE;
}
		
BMA_DEF void* BMA_DBG_SFFX(bma_HshMp_get_impl)(
	bma_HshMp *pSelf,
	const void *pKey
	BMA_DBG_OPT_PARAM(const char* pKeyType)
	BMA_DBG_OPT_PARAM(const char* pValueType)
) {
	size_t hashCode;
	bma_HshMpBcktHdr_ *pBucket;
	char *p;
	size_t i;
	bma_assert(pSelf != NULL);
#if BMA_DBG
	bma_assert(strcmp(pKeyType, pSelf->pKeyType) == 0);
	bma_assert(strcmp(pValueType, pSelf->pValueType) == 0);
#endif
	if (pSelf->numBckts == 0) return NULL;
	hashCode = (*pSelf->pKeyHash)(pKey);
	pBucket = pSelf->pBckts[hashCode % pSelf->numBckts];
	if (pBucket == NULL) return NULL;
	p = ((char*)pBucket) + BMA_HSHMP_BCKT_HDR_SZ;
	for (i=0; i<pBucket->size; ++i) {
		
		if ((*(size_t*)p == hashCode) &&
		    (*pSelf->pKeyEq)(p + pSelf->keyOffs, pKey)) {
			return p + pSelf->valueOffs;
		}
		p += pSelf->bcktSz;
	}
	return NULL;
}

/*
  An empty StringBuilder shouldn't need to allocate memory; on the other hand,
  the contained string always should point to something valid and
  null-terminated.
  BMA_NULLSTR is used for the case that nothing has been appended yet,
  so that bma_StrBldr_getStr() always returns a valid string.
*/
static const char BMA_NULLSTR_[1] = {'\0'};
#define BMA_NULLSTR ((char*)BMA_NULLSTR_)

BMA_DEF void bma_StrBldr_ctor_ext(bma_StrBldr *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	bma_assert(pAlloc != NULL);
	pSelf->pStr = BMA_NULLSTR;
	pSelf->size = 0;
	pSelf->cap = 0;
	pSelf->pAlloc = pAlloc;
}

BMA_DEF void bma_StrBldr_dtor(bma_StrBldr *pSelf, bma_IMemAlloc *pAlloc) {
	bma_assert(pSelf != NULL);
	(void)pAlloc; /* strbldr has its own */
	bma_assert(pSelf->pAlloc != NULL);
	if (pSelf->pStr != BMA_NULLSTR) {
		bma_free_ext(pSelf->pAlloc, pSelf->pStr);
	}
#if BMA_DBG
	memset(pSelf, 0xFF, sizeof(*pSelf));
#endif
}

BMA_DEF char *bma_StrBldr_rlse(bma_StrBldr *pSelf) {
	char *p;
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->pStr != NULL);
	bma_assert(pSelf->pStr[pSelf->size] == '\0');
	bma_assert(pSelf->pAlloc != NULL);
	if (pSelf->pStr == BMA_NULLSTR) {
		/* Give client always something on the heap */
		p = (*pSelf->pAlloc->pRllc)(pSelf->pAlloc, NULL, 1u, NULL);
		p[0] = '\0';
	}
	else {
		p = pSelf->pStr;
		pSelf->pStr = BMA_NULLSTR;
	}
	pSelf->size = 0;
	pSelf->cap = 0;
	return p;
}

BMA_DEF void bma_StrBldr_rsrv(bma_StrBldr *pSelf, size_t num) {
	size_t newCap;
	size_t realNewCap;
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->pAlloc != NULL);
	if ((pSelf->size + num) <= (pSelf->cap)) {
		return;
	}
	if (pSelf->cap == 0) {
		newCap = BMA_INIT_CAP;
	}
	else {
		newCap = pSelf->cap;
	}
	while((pSelf->size + num) > newCap) {
		newCap = bma_cap_incr(newCap);
	}
	bma_assert(pSelf->pAlloc != NULL);
	if (pSelf->pStr == BMA_NULLSTR) {
		pSelf->pStr = (*pSelf->pAlloc->pRllc)(pSelf->pAlloc, NULL, newCap + 1u, &realNewCap);
		pSelf->pStr[0] = '\0';
	}
	else {
		pSelf->pStr = (*pSelf->pAlloc->pRllc)(pSelf->pAlloc, pSelf->pStr, newCap + 1u, &realNewCap);
	}
	pSelf->cap = realNewCap - 1u;
}

BMA_DEF void bma_StrBldr_appndGnrc(bma_StrBldr *pSelf, size_t capIncrease, const char *pFmt, ...) {
	va_list ap;
	int numPrinted;
	bma_assert(pSelf != NULL);
	bma_assert(capIncrease > 0);
	bma_StrBldr_rsrv(pSelf, capIncrease);
	++capIncrease; /* reserve aways allocates one byte more */
	va_start(ap, pFmt);
#if BMA_MDRN_C
	numPrinted = vsnprintf(pSelf->pStr + pSelf->size, capIncrease, pFmt, ap);
#else
	numPrinted = vsprintf(pSelf->pStr + pSelf->size, pFmt, ap);
#endif
	va_end(ap);
	if (numPrinted < 0) {
		bma_exit_err("vs(n)printf returned negative value in bma_StrBldr_appndGnrc");
	}
	bma_assert((size_t)numPrinted < capIncrease);
	if ((size_t)numPrinted >= capIncrease) {
		bma_exit_err("Too small capIncrease in bma_StrBldr_appndGnrc, memory may be corrupted");
	}
	pSelf->size += (size_t)numPrinted;
}

BMA_DEF void bma_StrBldr_appndStr(bma_StrBldr *pSelf, const char *p) {
	size_t l;
	bma_assert(pSelf != NULL);
	bma_assert(p != NULL);
	l = strlen(p);
	bma_StrBldr_rsrv(pSelf, l);
	memcpy(pSelf->pStr + pSelf->size, p, l + 1u);
	pSelf->size += l;
}

BMA_DEF void bma_StrBldr_appndStrN(bma_StrBldr *pSelf, const char *p, size_t len) {
	bma_assert(pSelf != NULL);
	bma_assert(p != NULL);
	bma_StrBldr_rsrv(pSelf, len);
	memcpy(pSelf->pStr + pSelf->size, p, len);
	pSelf->size += len;
	pSelf->pStr[pSelf->size] = '\0';
}

BMA_DEF void bma_StrBldr_appndChr(bma_StrBldr *pSelf, int c, size_t count) {
	size_t i;
	char* p;
	bma_assert(pSelf != NULL);
	bma_StrBldr_rsrv(pSelf, count);
	p = pSelf->pStr + pSelf->size;
	for (i=0; i<count; ++i) {
		*p++ = (char)((unsigned char)c);
	}
	pSelf->size += count;
	pSelf->pStr[pSelf->size] = '\0';
}

BMA_DEF void bma_StrBldr_appndSgnd(bma_StrBldr *pSelf, bma_intmax_t i) {
	bma_StrBldr_appndGnrc(pSelf, BMA_UINTMAX_MAX_DEC_LEN, "%" BMA_INTMAX_PRFX "d", i);
}

BMA_DEF void bma_StrBldr_appndUnsgnd(bma_StrBldr *pSelf, bma_uintmax_t i) {
	bma_StrBldr_appndGnrc(pSelf, BMA_UINTMAX_MAX_DEC_LEN, "%" BMA_INTMAX_PRFX "u", i);
}

BMA_DEF void bma_StrBldr_appndDbl(bma_StrBldr *pSelf, double d) {
	bma_StrBldr_appndGnrc(pSelf, BMA_DBL_PRNT_NUM, "%." BMA_DBL_DEC_PRECSN_STR "g" , d);
}

BMA_DEF void bma_StrBldr_appndBool(bma_StrBldr *pSelf, bma_bool_t b) {
	if (b) {
		bma_StrBldr_appndStrN(pSelf, "true", 4u);
	}
	else {
		bma_StrBldr_appndStrN(pSelf, "false", 5u);
	}
}

BMA_DEF void bma_StrBldr_appndCdPntUtf8(bma_StrBldr *pSelf, unsigned long cdPnt) {
	unsigned char c[4];
	size_t l;
	bma_assert(cdPnt <= 0x10FFFF);
	if (cdPnt <= 0x7F) {
		c[0] = (unsigned char)(cdPnt);
		l = 1u;
	}
	else if (cdPnt <= 0x7FF) {
		c[0] = (unsigned char)(0xC0 | (cdPnt >> 6));
		c[1] = (unsigned char)(0x80 | (cdPnt & 0x3F));
		l = 2u;
	}
	else if (cdPnt <= 0xFFFF) {
		c[0] = (unsigned char)(0xE0 | (cdPnt >> 12));
		c[1] = (unsigned char)(0x80 | ((cdPnt >> 6) & 0x3F));
		c[2] = (unsigned char)(0x80 | (cdPnt & 0x3F));
		l = 3u;
	}
	else if (cdPnt <= 0x10FFFF) {
		c[0] = (unsigned char)(0xF0 | (cdPnt >> 18));
		c[1] = (unsigned char)(0x80 | ((cdPnt >> 12) & 0x3F));
		c[2] = (unsigned char)(0x80 | ((cdPnt >> 6) & 0x3F));
		c[3] = (unsigned char)(0x80 | (cdPnt & 0x3F));
		l = 4;
	}
	else {
		l = 0;
	}
	bma_StrBldr_appndStrN(pSelf, (const char*)&c[0], l);
}

BMA_DEF void bma_StrBldr_clear(bma_StrBldr *pSelf) {
	bma_assert(pSelf != NULL);
	if (pSelf->cap > 0) {
		pSelf->pStr[0] = '\0';
		pSelf->size = 0;
	}

}

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_CCAL_IMPL */

#ifdef BMA_CCAL_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

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

void test_strdup(void) {
	char* p;
	p = bma_strdup("Hello");
	BMA_EXPECT(p != NULL);
	BMA_EXPECT(strcmp(p, "Hello") == 0);
	bma_free(p);
}

typedef struct Ints {
	int *items;
	size_t size;
	size_t cap;
} Ints;

void test_dummy_int_destruct(int* p) {
	(void)p;
}

void test_vector_construct(void) {
	bma_Vec v;
	bma_Vec_ctor(&v, int, (bma_dtor_t)&test_dummy_int_destruct);
	BMA_EXPECT(v.pData == NULL);
	BMA_EXPECT(v.elemSz == sizeof(int));
	BMA_EXPECT(v.size == 0);
	BMA_EXPECT(v.cap == 0);
	BMA_EXPECT(v.pElemDtor == (bma_dtor_t)&test_dummy_int_destruct);
	BMA_EXPECT(v.pAlloc == bma_getDfltMemAlloc());
#if BMA_DBG
	BMA_EXPECT(strcmp(v.pType, "int") == 0);
#endif
	bma_Vec_dtor(&v, NULL);
}

void test_vector_append(void) {
	bma_Vec v;
	int x = 42;
	bma_Vec_ctor(&v, int, NULL);
	bma_Vec_appnd(&v, int, &x);
	BMA_EXPECT(v.size == 1u);
	BMA_EXPECT(v.cap == BMA_INIT_CAP);
	BMA_EXPECT(*(int*)v.pData == 42);
	bma_Vec_dtor(&v, NULL);
}

void test_vector_at(void) {
	bma_Vec v;
	int x = 42;
	bma_Vec_ctor(&v, int, NULL);
	bma_Vec_appnd(&v, int, &x);
	x = 43;
	bma_Vec_appnd(&v, int, &x);
	BMA_EXPECT(v.size == 2u);
	BMA_EXPECT(*bma_Vec_at(&v, 0, int) == 42);
	BMA_EXPECT(*bma_Vec_at(&v, 1, int) == 43);
	bma_Vec_dtor(&v, NULL);
}

void test_vector_of_strings(void) {
	bma_Vec v;
	char *p;
	bma_Vec_ctor(&v, char*, &bma_memblck_dtor);
	p = bma_strdup("one");
	bma_Vec_appnd(&v, char*, &p);
	p = bma_strdup("two");
	bma_Vec_appnd(&v, char*, &p);
	p = bma_strdup("three");
	bma_Vec_appnd(&v, char*, &p);
	BMA_EXPECT(strcmp(*bma_Vec_at(&v, 0, char*), "one") == 0);
	BMA_EXPECT(strcmp(*bma_Vec_at(&v, 1, char*), "two") == 0);
	BMA_EXPECT(strcmp(*bma_Vec_at(&v, 2, char*), "three") == 0);
	bma_Vec_dtor(&v, NULL);
}

void test_vector_rmv(void) {
	bma_Vec v;
	char *p;
	bma_Vec_ctor(&v, char*, &bma_memblck_dtor);
	p = bma_strdup("one");
	bma_Vec_appnd(&v, char*, &p);
	p = bma_strdup("two");
	bma_Vec_appnd(&v, char*, &p);
	p = bma_strdup("three");
	bma_Vec_appnd(&v, char*, &p);
	BMA_EXPECT(bma_Vec_getSz(&v) == 3u);
	BMA_EXPECT(bma_Vec_rmv(&v, 0u, &p, char*));
	BMA_EXPECT(strcmp(p, "one") == 0);
	bma_free(p);
	BMA_EXPECT(bma_Vec_getSz(&v) == 2u);
	BMA_EXPECT(bma_Vec_rmv(&v, 1u, &p, char*));
	BMA_EXPECT(strcmp(p, "three") == 0);
	BMA_EXPECT(bma_Vec_getSz(&v) == 1u);
	BMA_EXPECT(bma_Vec_rmv(&v, 0u, &p, char*));
	BMA_EXPECT(strcmp(p, "two") == 0);
	BMA_EXPECT(bma_Vec_getSz(&v) == 0u);
	bma_Vec_dtor(&v, NULL);
}

void test_dummy_forEachCallBack(int* p, bma_Vec* pUserData) {
	bma_Vec_appnd(pUserData, int, p);
}

void test_vector_callForEach(void) {
	bma_Vec v;
	bma_Vec v2;
	int x;
	bma_Vec_ctor(&v, int, NULL);
	bma_Vec_ctor(&v2, int, NULL);
	x = 0;
	bma_Vec_appnd(&v, int, &x);
	x = 1;
	bma_Vec_appnd(&v, int, &x);
	x = 2;
	bma_Vec_appnd(&v, int, &x);
	bma_Vec_each(&v, (bma_each_t)&test_dummy_forEachCallBack, &v2);
	BMA_EXPECT(v2.size == 3);
	BMA_EXPECT(*bma_Vec_at(&v2, 0, int) == 0);
	BMA_EXPECT(*bma_Vec_at(&v2, 1, int) == 1);
	BMA_EXPECT(*bma_Vec_at(&v2, 2, int) == 2);
	bma_Vec_dtor(&v2, NULL);
	bma_Vec_dtor(&v, NULL);
}

void test_stringBuilder_construct(void) {
	bma_StrBldr sb;
	bma_StrBldr_ctor(&sb);
	BMA_EXPECT(sb.pStr == BMA_NULLSTR);
	BMA_EXPECT(sb.size == 0);
	BMA_EXPECT(sb.cap == 0);
	BMA_EXPECT(sb.pAlloc = bma_getDfltMemAlloc());
	bma_StrBldr_dtor(&sb, NULL);
}

void test_stringBuilder_release(void) {
	bma_StrBldr sb;
	char *p;
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndStr(&sb, "Hello");
	p = bma_StrBldr_rlse(&sb);
	BMA_EXPECT(strcmp(p, "Hello") == 0);
	bma_free(p);
	/* State after release() should be same as after construction */
	BMA_EXPECT(sb.pStr == BMA_NULLSTR);
	BMA_EXPECT(sb.size == 0);
	BMA_EXPECT(sb.cap == 0);
	BMA_EXPECT(sb.pAlloc = bma_getDfltMemAlloc());
	bma_StrBldr_dtor(&sb, NULL);
	/* Release of empty string should heap-allocate: */
	bma_StrBldr_ctor(&sb);
	p = bma_StrBldr_rlse(&sb);
	bma_StrBldr_dtor(&sb, NULL);
	BMA_EXPECT(strcmp(p, "") == 0);
	bma_free(p);
}

void test_stringBuilder_reserve(void) {
	bma_StrBldr sb;
	bma_StrBldr_ctor(&sb);
	BMA_EXPECT(sb.pStr == BMA_NULLSTR);
	BMA_EXPECT(sb.size == 0);
	BMA_EXPECT(sb.cap == 0);
	bma_StrBldr_rsrv(&sb, 1u);
	BMA_EXPECT(sb.pStr != BMA_NULLSTR);
	BMA_EXPECT(sb.pStr[0] == '\0');
	BMA_EXPECT(sb.size == 0);
	BMA_EXPECT(sb.cap == BMA_INIT_CAP);
	bma_StrBldr_dtor(&sb, NULL);
}

void test_stringBuilder_appendGeneric(void) {
	bma_StrBldr sb;
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndGnrc(&sb, 3u, "%d", 123);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), "123") == 0);
	bma_StrBldr_dtor(&sb, NULL);
}

void test_stringBuilder_appendStr(void) {
	bma_StrBldr sb;
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndStr(&sb, "Hello");
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), "Hello") == 0);
	bma_StrBldr_dtor(&sb, NULL);
}

void test_stringBuilder_appendStrWithLen(void) {
	bma_StrBldr sb;
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndStrN(&sb, "Hello", 3u);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), "Hel") == 0);
	bma_StrBldr_dtor(&sb, NULL);
}

void test_stringBuilder_appendChar(void) {
	bma_StrBldr sb;
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndChr(&sb, 'A', 3u);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), "AAA") == 0);
	bma_StrBldr_dtor(&sb, NULL);
}

void test_stringBuilder_appendSigned(void) {
	bma_StrBldr sb;
	char cmp[BMA_UINTMAX_MAX_DEC_LEN + 1u] = {0};
	/* min */
	sprintf(cmp, "%" BMA_INTMAX_PRFX "d", BMA_INTMAX_MIN);
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndSgnd(&sb, BMA_INTMAX_MIN);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), cmp) == 0);
	bma_StrBldr_dtor(&sb, NULL);
	/* 0 */
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndSgnd(&sb, 0);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), "0") == 0);
	bma_StrBldr_dtor(&sb, NULL);
	/* max */
	memset(cmp, 0, sizeof(cmp));
	sprintf(cmp, "%" BMA_INTMAX_PRFX "d", BMA_INTMAX_MAX);
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndSgnd(&sb, BMA_INTMAX_MAX);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), cmp) == 0);
	bma_StrBldr_dtor(&sb, NULL);
}

void test_stringBuilder_appendUnsigned(void) {
	bma_StrBldr sb;
	char cmp[BMA_UINTMAX_MAX_DEC_LEN + 1u] = {0};
	/* 0 */
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndUnsgnd(&sb, 0);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), "0") == 0);
	bma_StrBldr_dtor(&sb, NULL);
	/* max */
	sprintf(cmp, "%" BMA_INTMAX_PRFX "u", BMA_UINTMAX_MAX);
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndUnsgnd(&sb, BMA_UINTMAX_MAX);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), cmp) == 0);
	bma_StrBldr_dtor(&sb, NULL);
}

void test_stringBuilder_appendDouble(void) {
	bma_StrBldr sb;
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndDbl(&sb, -DBL_MAX);
	bma_StrBldr_appndDbl(&sb, 0.0);
	bma_StrBldr_appndDbl(&sb, DBL_MAX);
	bma_StrBldr_appndDbl(&sb, -DBL_MIN);
	bma_StrBldr_appndDbl(&sb, DBL_MIN);
	bma_StrBldr_dtor(&sb, NULL);
}

void test_stringBuilder_appendBool(void) {
	bma_StrBldr sb;
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndBool(&sb, BMA_TRUE);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), "true") == 0);
	bma_StrBldr_dtor(&sb, NULL);
	bma_StrBldr_ctor(&sb);
	bma_StrBldr_appndBool(&sb, BMA_FALSE);
	BMA_EXPECT(strcmp(bma_StrBldr_getStr(&sb), "false") == 0);
	bma_StrBldr_dtor(&sb, NULL);
}

void test_hashMap_constructDestruct(void) {
	bma_HshMp h;
	bma_HshMp_ctor(&h, char*, char*, &bma_memblck_dtor, &bma_memblck_dtor, &bma_strhash, &bma_streq);
	bma_HshMp_dtor(&h, NULL);
}

void test_hashMap_putGet(void) {
	bma_HshMp h;
	char *pKey;
	char *pValue;
	char** ppFoundValue;
	bma_bool_t isNewValue;
	bma_HshMp_ctor(&h, char*, char*, &bma_memblck_dtor, &bma_memblck_dtor, &bma_strhash, &bma_streq);
	pKey = bma_strdup("Hello");
	pValue = bma_strdup("Hallo");
	isNewValue = bma_HshMp_put(&h, &pKey, &pValue, char*, char*);
	BMA_EXPECT(isNewValue);
	pKey = bma_strdup("Goodbye");
	pValue = bma_strdup("Auf Wiedersehen");
	isNewValue = bma_HshMp_put(&h, &pKey, &pValue, char*, char*);
	BMA_EXPECT(isNewValue);
	pKey = bma_strdup("Hello");
	pValue = bma_strdup("NewValue");
	isNewValue = bma_HshMp_put(&h, &pKey, &pValue, char*, char*);
	BMA_EXPECT(!isNewValue);
	pKey = (char*)"Hello";
	ppFoundValue = bma_HshMp_get(&h, &pKey, char*, char*);
	BMA_EXPECT(ppFoundValue != NULL);
	BMA_EXPECT(strcmp(*ppFoundValue, "NewValue") == 0);
	pKey = (char*)"Goodbye";
	ppFoundValue = bma_HshMp_get(&h, &pKey, char*, char*);
	BMA_EXPECT(ppFoundValue != NULL);
	BMA_EXPECT(strcmp(*ppFoundValue, "Auf Wiedersehen") == 0);
	pKey = (char*)"Something else";
	ppFoundValue = bma_HshMp_get(&h, &pKey, char*, char*);
	BMA_EXPECT(ppFoundValue == NULL);
	bma_HshMp_dtor(&h, NULL);
}

static size_t test_intHasher(const void* p) {
	return (size_t)(*(const int*)p);
}

static bma_bool_t test_intEquals(const void* pLhs, const void* pRhs) {
	return (*(const int*)pLhs) == (*(const int*)pRhs);
}

void test_hashMap_manyElements(void) {
	bma_HshMp h;
	int key, value, *pFoundValue;
	size_t i;
	size_t num = 10000;
	bma_HshMp_ctor(&h, int, int, NULL, NULL, &test_intHasher, &test_intEquals);
	for (i=0; i<num; ++i) {
		key = (int)i;
		value = (int)(i);
		bma_HshMp_put(&h, &key, &value, int, int);
	}
	BMA_EXPECT(bma_HshMp_getSz(&h) == num);
	for (i=0; i<num; ++i) {
		key = (int)i;
		pFoundValue = bma_HshMp_get(&h, &key, int, int);
		BMA_EXPECT(pFoundValue != NULL);
		BMA_EXPECT(*pFoundValue == (int)i);
	}
	bma_HshMp_dtor(&h, NULL);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_strdup);
	BMA_TEST(test_vector_construct);
	BMA_TEST(test_vector_append);
	BMA_TEST(test_vector_at);
	BMA_TEST(test_vector_of_strings);
	BMA_TEST(test_vector_rmv);
	BMA_TEST(test_vector_callForEach);
	BMA_TEST(test_stringBuilder_construct);
	BMA_TEST(test_stringBuilder_release);
	BMA_TEST(test_stringBuilder_reserve);
	BMA_TEST(test_stringBuilder_appendGeneric);
	BMA_TEST(test_stringBuilder_appendStr);
	BMA_TEST(test_stringBuilder_appendStrWithLen);
	BMA_TEST(test_stringBuilder_appendChar);
	BMA_TEST(test_stringBuilder_appendSigned);
	BMA_TEST(test_stringBuilder_appendUnsigned);
	BMA_TEST(test_stringBuilder_appendDouble);
	BMA_TEST(test_stringBuilder_appendBool);
	BMA_TEST(test_hashMap_constructDestruct);
	BMA_TEST(test_hashMap_putGet);
	BMA_TEST(test_hashMap_manyElements);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_CCAL_TEST */

#endif /* BMA_CCAL_H_INCLUDED */
