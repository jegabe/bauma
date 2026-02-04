#ifndef BAUMA_CCAL_H_INCLUDED
#define BAUMA_CCAL_H_INCLUDED

/*

Bau 'ma's general purpose C container and algorithms library (CCAL).
This is to be some kind of STL-like library, but in the spirit of C.
This means that containers treat their data as opaque (void* plus size_t pair)
and move memory around, just like qsort(), memcpy() and memmove() does.
Fun fact: C had "move semantics" from the beginning *grin*.

CCAL has no dependencies except for some common C98 header such as <string.h>,
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
containg the function definitions, which can be linked to afterwards.

*/

#include <stddef.h> /* for size_t, NULL and stuff */
#include <string.h> /* for all kind of memory and string things */
#include <stdlib.h> /* for malloc(), free(), exit() etc. */
#ifndef bauma_exit_err
    #include <stdio.h> /* to print to stderr before leaving */
#endif
#ifndef bauma_assert
    #include <assert.h>
#endif

/* Define this before including when something else is needed */
#ifndef BAUMA_DEBUG
    #ifdef NDEBUG /* Standard way of detecting releas build */
        #define BAUMA_DEBUG 0 /* release build */
    #else
        #define BAUMA_DEBUG 1 /* debug build */
    #endif
#endif

/* Define this before including when something else is needed */
#ifndef bauma_assert
    #define bauma_assert(x) assert(x)
#endif

/*
   Define this before including when something else is needed,
   such as "static inline" or "declspec(dllimport)"
*/
#ifndef BAUMA_DEF
    #define BAUMA_DEF
#endif

#ifdef __cplusplus
    extern "C" {
#endif

/*! \brief Signature for memory allocators for customizing containers */
typedef void*(*bauma_memalloc_function)(void*, size_t, size_t*);

#ifndef bauma_exit_err

/*!
  \brief Prints error message to stderr and calls exit(1) to terminate the application
*/
BAUMA_DEF void bauma_exit_err(const char *msg);

#endif

/*!
  \brief To allocate, re-allocate and free memory according to bauma_memalloc_function.

  This is designed to behave more well-defined than plain realloc(): When going out of memory,
  the application is terminated instead of returning NULL pointer; when size is zero, NULL
  is guaranteed to be returned since that is a legitimate pointer to a memory block of size zero.
  The realsize parameter is optional and can be used when the memory allocator is smart enough to
  know the real memory block sizes which might be greater than size. This info can be used
  to make smarter reallocations, but the default implementation based on malloc/realloc/free can
  of course not know that.
  \param old Old memory block or NULL to allocate a new memory block
  \param size Wanted memory block size or 0 to deallocate
  \param realsize Output parameter receiving the number of allocated bytes
*/
BAUMA_DEF void *bauma_default_memalloc(void *old, size_t size, size_t *realsize);

/*
  \brief Allocates a copy of the passed null-terminated string and returns it
  \param p A null-terminated string
  \param alloc Memory allocator to be used
*/
BAUMA_DEF char* bauma_strdup_ext(const char* p, bauma_memalloc_function alloc);

/*
  \brief Short-hand of bauma_strdup_ext() using the default allocator
  \param p A null-terminated string
*/
#define bauma_strdup(p) bauma_strdup_ext(p, &bauma_default_memalloc)

#if 0 /* TODO */
#define bauma_dynarray_append(d, e) \
    do { \
      if ((d)->count <= (d)->capacity) {
        void* p_;
        p = bauma_default_memalloc((d)->items, )
      }
    } while(0)
#endif

#ifdef __cplusplus
    } /* extern "C" */
#endif

#ifdef BAUMA_CCAL_IMPLEMENTATION

#ifdef __cplusplus
    extern "C" {
#endif

#ifndef bauma_exit_err
BAUMA_DEF void bauma_exit_err(const char *msg) {
    fprintf(stderr, "Error: %s. Terminating.\n", msg);
    fflush(stderr);
    exit(1);
}
#endif /* bauma_exit_err */

BAUMA_DEF void *bauma_default_memalloc(void *old, size_t size, size_t *realsize) {
    void *p;
    if (realsize != NULL) {
        *realsize = 0;
    }
    if (old == NULL) {
        if (size == 0) {
            return NULL; /* Null pointer is a legitimate memory block of size zero */
        }
        p = malloc(size);
        if (p == NULL) {
            bauma_exit_err("Out of memory");
        }
        if (realsize != NULL) {
            *realsize = size;
        }
        return p;
    }
    if (size == 0) {
        free(old);
        return NULL;
    }
    p = realloc(old, size);
    if (p == NULL) {
        bauma_exit_err("Out of memory");
    }
    if (realsize != NULL) {
        *realsize = size;
    }
    return p;
}

BAUMA_DEF char* bauma_strdup_ext(const char* p, bauma_memalloc_function alloc) {
    size_t l;
    char *c;
    bauma_assert((alloc != NULL) && "Allocator must not be NULL");
    l = strlen(p) + 1u; /* +1 to include the null terminator */
    c = (char*)((*alloc)(NULL, l, NULL));
    memcpy(c, p, l);
    return c;
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
}

#ifdef __cplusplus
    } /* extern "C" */
#endif

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    BAUMA_TEST(test_strdup);
    printf("All tests passed.\n");
    fflush(stdout);
    return 0;
}

#endif /* BAUMA_CCAL_TEST */

#endif /* BAUMA_CCAL_H_INCLUDED */
