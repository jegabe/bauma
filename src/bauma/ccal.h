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

- bauma_dynarray_append(): Minimalist macro to have dynamic arrays without bauma_Vector overhead
  but less powerful
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

#define BAUMA_TRUE 	((bauma_bool_t)1)
#define BAUMA_FALSE ((bauma_bool_t)0)

#define bauma_bool2str(x) ((x) ? "true" : "false")

/* Find fitting (u)intmax_t data type */
#if BAUMA_MODERN_C
	#include <stdint.h>
	typedef intmax_t bauma_intmax_t;
	typedef uintmax_t bauma_uintmax_t;
	#if UINTMAX_MAX == UINT_MAX
		#define BAUMA_INTMAX_PREFIX ""
	#elif UINTMAX_MAX == ULONG_MAX
		#define BAUMA_INTMAX_PREFIX "l"
	#elif UINTMAX_MAX == ULLONG_MAX
		#define BAUMA_INTMAX_PREFIX "ll"
	#else
		#error "Can't detect printf prefix for bauma_(u)intmax_t"
	#endif
#else /* older compiler */
	/* This is a guess but usually those types reflect
		the maximum */
	typedef ptrdiff_t bauma_intmax_t;
	typedef size_t bauma_uintmax_t;
	#if SIZE_MAX == UINT_MAX
		#define BAUMA_INTMAX_PREFIX ""
	#elif SIZE_MAX == ULONG_MAX
		#define BAUMA_INTMAX_PREFIX "l"
	#elif defined(ULLONG_MAX) && (SIZE_MAX == ULLONG_MAX)
		#define BAUMA_INTMAX_PREFIX "ll"
	#else
		#error "Can't detect printf prefix for bauma_(u)intmax_t"
	#endif
#endif


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

/*! Initial dynamic array capacity for bauma_dynarray_append */
#define BAUMA_INITIAL_CAPACITY 16u

/*! Capacity growth factor for bauma_dynarray_append */
#define BAUMA_CAPACITY_GROWTH 2u

#define bauma_min(a, b) ((a) < (b) ? (a) : (b))
#define bauma_max(a, b) ((a) < (b) ? (a) : (b))

/*! \brief Signature for memory allocators for customizing containers */
typedef void*(*bauma_pDynmem_handler)(void*, size_t, size_t*);

typedef void (*bauma_pDestructor)(void* pCurrentElement);

typedef void (*bauma_pForEachHandler)(void *pCurrentElement, void *pOptUserData);

/*!
	\brief Prints error message to stderr and calls exit(1) to terminate the application
*/
BAUMA_DEF void bauma_exit_err(const char *msg);

/*!
  \brief To allocate, re-allocate and free memory according to bauma_memalloc_function.

  This is designed to behave more well-defined than plain realloc(): When going out of memory,
  the application is terminated instead of returning NULL pointer; when size is zero, NULL
  is guaranteed to be returned since that is a legitimate pointer to a memory block of size zero.
  The realsize parameter is optional and can be used when the memory allocator is smart enough to
  know the real memory block sizes which might be greater than size. This info can be used
  to make smarter reallocations, but the default implementation based on malloc/realloc/free can
  of course not know that.
  \param pOld Old memory block or NULL to allocate a new memory block
  \param newSize Wanted memory block size or 0 to deallocate
  \param pOptRealSize Output parameter receiving the number of allocated bytes
*/
BAUMA_DEF void *bauma_default_dynmem_handler(void *pOld, size_t newSsize, size_t *pOptRealSize);

/*! Short-hand for just allocating memory */
#define bauma_default_malloc(s) bauma_default_dynmem_handler(NULL, s, NULL)

/*! Short-hand for just freeing memoy*/
#define bauma_default_free(p) (void)bauma_default_dynmem_handler(p, 0, NULL)

/*! Short-hand for just reallicating freeing memoy, ignoring the real block size */
#define bauma_default_realloc(p, n) bauma_default_dynmem_handler(p, n, NULL)

/*! Can be used as element destructor (e.g. for vectors) when the element
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
BAUMA_DEF char* bauma_strdup_ext(const char* p, bauma_pDynmem_handler alloc);

/*!
	\brief Short-hand of bauma_strdup_ext() using the default allocator
	\param p A null-terminated string
*/
#define bauma_strdup(p) bauma_strdup_ext(p, &bauma_default_dynmem_handler)

/*!
	\brief Minimalistic approach for dynamically growing array.

	To use it, define a struct having 3 elements: items, size and capacity,
	just like this:
	\code
	typedef struct Ints
	{
		int *items;
		size_t size;
		size_t capacity;
	} Ints;
	\endcode
	and intialize that with "{0}". After that, bauma_dynarray_append can be used to dynamically
	append items, doing reallocations where needed. Full example:
	\code
	typedef struct Ints
	{
		int *items;
		size_t size;
		size_t capacity;
	} Ints;

	Ints ints = {0};
	bauma_dynarray_append(&ints, int, 0);
	bauma_dynarray_append(&ints, int, 0);
	bauma_dynarray_append(&ints, int, 0);
	bauma_default_free(ints.items);
	\endcode
*/
#define bauma_dynarray_append(d, t, e) \
	do { \
		if ((d)->size >= (d)->capacity) { \
			size_t newcap_; \
			size_t realNewCap_; \
			if ((d)->capacity == 0) { \
				newcap_ = BAUMA_INITIAL_CAPACITY; \
			} \
			else { \
				newcap_ = (d)->capacity * BAUMA_CAPACITY_GROWTH; \
			} \
			(d)->items = (t*)bauma_default_dynmem_handler((d)->items, newcap_ * sizeof(t), &realNewCap_); \
			(d)->capacity = realNewCap_ / sizeof(t); \
		} \
		(d)->items[(d)->size++] = e; \
	} while(0)

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
	bauma_VectorDataPtr_  d;
	size_t                elementSize;
	size_t                size;
	size_t                capacity;
	bauma_pDestructor     pDestructor;
	bauma_pDynmem_handler pMemHandler;
#if BAUMA_DEBUG
	const char            *pDataType;
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
	bauma_pDynmem_handler pMemHandler
	BAUMA_DEBUG_OPT_PARAM(const char* pDataType)
);

#define bauma_Vector_construct(pSelf, dataType, pElementDestructor) \
	BAUMA_DEBUG_SUFFIX(bauma_Vector_construct_impl)( \
		pSelf, \
		sizeof(dataType), \
		pElementDestructor, \
		&bauma_default_dynmem_handler \
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

typedef struct bauma_StringBuilder {
	char* pStr;
	size_t size; /* not counting the null terminator */
	size_t capacity;
	bauma_pDynmem_handler pMemHandler
} bauma_StringBuilder;

#ifdef __cplusplus
	} /* extern "C" */
#endif

BAUMA_DEF void bauma_Vector_callForEach(bauma_Vector *pSelf, bauma_pForEachHandler pFunc, void *pOptUserData);

#ifdef BAUMA_CCAL_IMPLEMENTATION

#include <string.h> /* for all kind of memory and string things */
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
	exit(1);
#endif
}

BAUMA_DEF void *bauma_default_dynmem_handler(void *pOld, size_t newSize, size_t *pOptRealSize) {
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

BAUMA_DEF void bauma_memblock_destructor(void* ppMemBlock) {
	void* pMemBlock = *(void**)ppMemBlock;
	bauma_default_dynmem_handler(pMemBlock, 0, NULL);
}


BAUMA_DEF char* bauma_strdup_ext(const char* p, bauma_pDynmem_handler alloc) {
	size_t l;
	char *c;
	bauma_assert((alloc != NULL) && "Allocator must not be NULL");
	l = strlen(p) + 1u; /* +1 to include the null terminator */
	c = (char*)((*alloc)(NULL, l, NULL));
	memcpy(c, p, l);
	return c;
}

BAUMA_DEF void BAUMA_DEBUG_SUFFIX(bauma_Vector_construct_impl)(
	bauma_Vector *pSelf,
	size_t elemSize, 
	bauma_pDestructor pElementDestructor,
	bauma_pDynmem_handler pMemHandler
	BAUMA_DEBUG_OPT_PARAM(const char* pDataType)
) {
	bauma_assert(pSelf != NULL);
	bauma_assert(elemSize > 0);
	bauma_assert(pMemHandler != NULL);
	pSelf->d.pData = NULL;
	pSelf->elementSize = elemSize;
	pSelf->size = 0;
	pSelf->capacity = 0;
	pSelf->pDestructor = pElementDestructor;
	pSelf->pMemHandler = pMemHandler;
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
	(*pSelf->pMemHandler)(pSelf->d.pData, 0, NULL);
}

BAUMA_DEF void bauma_Vector_reserve(bauma_Vector *pSelf, size_t num) {
	size_t newCap;
	size_t realNewCap;
	bauma_assert(pSelf != NULL);
	bauma_assert(pSelf->pMemHandler != NULL);
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
	bauma_assert(pSelf->pMemHandler != NULL);
	pSelf->d.pData = (*pSelf->pMemHandler)(pSelf->d.pData, newCap * pSelf->elementSize, &realNewCap);
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

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BAUMA_CCAL_IMPLEMENTATION */

#ifdef BAUMA_CCAL_TEST

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

void test_strdup(void) {
	char* p;
	p = bauma_strdup("Hello");
	BAUMA_EXPECT(p != NULL);
	BAUMA_EXPECT(strcmp(p, "Hello") == 0);
	bauma_default_free(p);
}

typedef struct Ints {
	int *items;
	size_t size;
	size_t capacity;
} Ints;

void test_dynarray_append(void) {
	Ints ints = {0};
	size_t i;
	bauma_dynarray_append(&ints, int, 0);
	bauma_dynarray_append(&ints, int, 1);
	bauma_dynarray_append(&ints, int, 2);
	BAUMA_EXPECT(ints.size == 3);
	BAUMA_EXPECT(ints.capacity == BAUMA_INITIAL_CAPACITY);
	for (i=3; i<BAUMA_INITIAL_CAPACITY; ++i)
	{
		bauma_dynarray_append(&ints, int, (int)i);
	}
	BAUMA_EXPECT(ints.size == BAUMA_INITIAL_CAPACITY);
	BAUMA_EXPECT(ints.capacity == BAUMA_INITIAL_CAPACITY);
	bauma_dynarray_append(&ints, int, BAUMA_INITIAL_CAPACITY);
	BAUMA_EXPECT(ints.capacity == (BAUMA_INITIAL_CAPACITY * BAUMA_CAPACITY_GROWTH));

	bauma_default_free(ints.items);
}

void test_dummy_int_destruct(int*) {
}

void test_vector_construct(void) {
	bauma_Vector v;
	bauma_Vector_construct(&v, int, (bauma_pDestructor)&test_dummy_int_destruct);
	BAUMA_EXPECT(v.d.pData == NULL);
	BAUMA_EXPECT(v.elementSize == sizeof(int));
	BAUMA_EXPECT(v.size == 0);
	BAUMA_EXPECT(v.capacity == 0);
	BAUMA_EXPECT(v.pDestructor == (bauma_pDestructor)&test_dummy_int_destruct);
	BAUMA_EXPECT(v.pMemHandler == &bauma_default_dynmem_handler);
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

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BAUMA_TEST(test_strdup);
	BAUMA_TEST(test_dynarray_append);
	BAUMA_TEST(test_vector_construct);
	BAUMA_TEST(test_vector_append);
	BAUMA_TEST(test_vector_at);
	BAUMA_TEST(test_vector_forEach);
	BAUMA_TEST(test_vector_of_strings);
	BAUMA_TEST(test_vector_callForEach);
	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BAUMA_CCAL_TEST */

#endif /* BAUMA_CCAL_H_INCLUDED */
