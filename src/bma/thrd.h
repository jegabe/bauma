#ifndef BMA_THRD_H_INCLUDED
#define BMA_THRD_H_INCLUDED
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

Bau 'ma's basic multithreading support

*/
#include <bma/ccal.h>

#ifdef _WIN32
	#define BMA_HAS_THRDS 1
	#define BMA_WIN_THRDS 1
	#define BMA_POSIX_THRDS 0
#elif defined(__linux__) || defined(__gnu_linux__) || \
      defined(__APPLE__) || defined(__MACH__) || \
      defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
	/* POSIX */
	#define BMA_HAS_THRDS 1
	#define BMA_WIN_THRDS 0
	#define BMA_POSIX_THRDS 1
#else
	#define BMA_HAS_THRDS 0
	#define BMA_WIN_THRDS 0
	#define BMA_POSIX_THRDS 0
#endif

/* OS headers needed */
#if BMA_WIN_THRDS
	/* Windows.h is tremendously huge. Trying to get it down a bit. */
	#ifndef BMA_FULL_WINDOWS_H
		#ifndef WIN32_LEAN_AND_MEAN
			#define WIN32_LEAN_AND_MEAN
			#define BMA_UNDEF_WIN32_LEAN_AND_MEAN
		#endif
		#ifndef NOMINMAX
			#define NOMINMAX
			#define BMA_UNDEF_NOMINMAX
		#endif
	#endif
	#include <Windows.h>
	/* Clean-up of temporary macro definitions */
	#ifdef BMA_UNDEF_WIN32_LEAN_AND_MEAN
		#undef WIN32_LEAN_AND_MEAN
		#undef BMA_UNDEF_WIN32_LEAN_AND_MEAN
	#endif
	#ifdef BMA_UNDEF_NOMINMAX
		#undef NOMINMAX
		#undef BMA_UNDEF_NOMINMAX
	#endif
#elif BMA_POSIX_THRDS
	#include <unistd.h>
	#include <time.h>
	#include <pthread.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif

/* Atomic counters */
#if BMA_WIN_THRDS /* Independent of compiler, there's an API */
	typedef LONG bma_atmc_t;
	#define bma_atmc_inc(p) InterlockedIncrement((volatile LONG*)(p))
	#define bma_atmc_dec(p) InterlockedDecrement((volatile LONG*)(p))
	#define bma_atmc_cas(p, comp, exchg) InterlockedCompareExchange((volatile LONG*)(p), (exchg), (comp))
	#define bma_atmc_exchg(p, val) InterlockedExchange((volatile LONG*)(p), val)
#elif defined (__GNUC__) || defined (__clang__) /* Independent of OS, theres a compiler intrinsic */
	typedef int bma_atmc_t;
	#define bma_atmc_inc(p) __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST)
	#define bma_atmc_dec(p) __atomic_sub_fetch(p, 1, __ATOMIC_SEQ_CST)
	bma_inline bma_atmc_t bma_atmc_cas_impl_(bma_atmc_t *p, bma_atmc_t comp, bma_atmc_t exchg) {
		(void)__atomic_compare_exchange_n(p, &comp, exchg, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
		return comp;
	}
	#define bma_atmc_cas(p, comp, exchg) bma_atmc_cas_impl_(p, comp, exchg)
	#define bma_atmc_exchg(p, val) __atomic_exchange_n(p, val, __ATOMIC_SEQ_CST)
#else
	#undef BMA_HAS_THRDS
	#undef BMA_WIN_THRDS
	#undef BMA_POSIX_THRDS
	#define BMA_HAS_THRDS 0
	#define BMA_WIN_THRDS 0
	#define BMA_POSIX_THRDS 0
	typedef int bma_atmc_t;
	#define bma_atmc_inc(p) (++(*(p)))
	#define bma_atmc_dec(p) (--(*(p)))
	bma_inline bma_atmc_t bma_atmc_cas_impl_(bma_atmc_t *p, bma_atmc_t comp, bma_atmc_t exchg) {
		bma_atmc_t old = *p;
		if (old == comp) {
			*p = exchg;
		}	
		return old;
	}
	#define bma_atmc_cas(p, comp, exchg) bma_atmc_cas_impl_(p, comp, exchg)
#endif


/* Shared ownership of data on heap */
typedef struct bma_Shrd {
	bma_atmc_t strongRefs;
	bma_atmc_t weakRefs;
	bma_atmc_t alive;
	bma_dtor_t pDtor;
	bma_IMemAlloc *pAlloc;
#if BMA_DBG
	const char *pType;
#endif
	bma_max_align_t aligner_;
} bma_Shrd;


#define BMA_SHRD_HDR_SZ (sizeof(bma_Shrd) - sizeof(bma_max_align_t))

BMA_DEF bma_Shrd *BMA_DBG_SFFX(bma_Shrd_new_impl)(
	size_t sz,
	bma_dtor_t pDtor,
	bma_IMemAlloc *pAlloc
	BMA_DBG_OPT_PARAM(const char* pType)
);

#define bma_Shrd_new_ext(type, pDtor, pAlloc) \
	BMA_DBG_SFFX(bma_Shrd_new_impl)(sizeof(type), pDtor, pAlloc BMA_DBG_OPT_PARAM(#type))

#define bma_Shrd_new(type, pDtor) \
	BMA_DBG_SFFX(bma_Shrd_new_impl)(sizeof(type), pDtor, bma_getDfltMemAlloc() BMA_DBG_OPT_PARAM(#type))

#if BMA_DBG
	BMA_DEF void *bma_Shrd_get_impl_D(bma_Shrd *pSelf, size_t typeSz, const char *pType);
	#define bma_Shrd_get(pSelf, type) \
		((type*)bma_Shrd_get_impl_D(pSelf, sizeof(type), #type))
#else
	#define bma_Shrd_get(pSelf, type) \
		bma_lndr_cast(type, ((char*)pSelf) + BMA_SHRD_HDR_SZ)
#endif

BMA_DEF void bma_Shrd_inc(bma_Shrd *pSelf);
BMA_DEF void bma_Shrd_dec(bma_Shrd *pSelf);

typedef struct bma_Wk {
	bma_Shrd target;
} bma_Wk;

BMA_DEF bma_Wk *bma_Wk_new(bma_Shrd *pShrd);
BMA_DEF bma_Shrd *bma_Wk_lock(bma_Wk *pSelf);
BMA_DEF void bma_Wk_inc(bma_Wk *pSelf);
BMA_DEF void bma_Wk_dec(bma_Wk *pSelf);

#define BMA_DEF_SHRD(name_, namewk_, datatype_, pDtor_) \
	typedef struct name_ { \
		bma_Shrd impl; \
	} name_; \
	\
	bma_inline name_ *name_ ## _new_ext(bma_IMemAlloc *pAlloc) { \
		return bma_lndr_cast(name_, bma_Shrd_new_ext(datatype_, pDtor_, pAlloc)); \
	} \
	\
	bma_inline name_ *name_ ## _new() { \
		return bma_lndr_cast(name_, bma_Shrd_new(datatype_, pDtor_)); \
	} \
	\
	bma_inline datatype_ *name_ ## _get(name_ *pSelf) { \
		return (datatype_*)bma_Shrd_get(&pSelf->impl, datatype_); \
	} \
	\
	bma_inline void name_ ## _inc(name_ *pSelf) { \
		bma_Shrd_inc(&pSelf->impl); \
	} \
	\
	bma_inline void name_ ## _dec(name_ *pSelf) { \
		bma_Shrd_dec(&pSelf->impl); \
	} \
	\
	typedef struct namewk_ { \
		bma_Wk impl; \
	} namewk_; \
	\
	bma_inline namewk_ *namewk_ ## _new(name_ *pShrd) { \
		return bma_lndr_cast(namewk_, bma_Wk_new(&pShrd->impl)); \
	} \
	\
	bma_inline name_ *namewk_ ## _lock(namewk_ *pSelf) { \
		return bma_lndr_cast(name_, bma_Wk_lock(&pSelf->impl)); \
	} \
	\
	bma_inline void namewk_ ## _inc(namewk_ *pSelf) { \
		bma_Wk_inc(&pSelf->impl); \
	} \
	\
	bma_inline void namewk_ ## _dec(namewk_ *pSelf) { \
		bma_Wk_dec(&pSelf->impl); \
	}

/* Mutex, ConditionVariable, Thread... */
#if BMA_WIN_THRDS
	typedef CRITICAL_SECTION bma_Mtx;
	typedef CONDITION_VARIABLE bma_Cnd;
	typedef SRWLOCK bma_Rw;
	typedef HANDLE bma_Thrd;
#elif BMA_POSIX_THRDS
	typedef pthread_mutex_t bma_Mtx;
	typedef pthread_rwlock_t bma_Rw;
	typedef pthread_cond_t bma_Cnd;
	typedef pthread_t bma_Thrd;
#else
	#undef BMA_HAS_THRDS
	#undef BMA_WIN_THRDS
	#undef BMA_POSIX_THRDS
	#define BMA_HAS_THRDS 0
	#define BMA_WIN_THRDS 0
	#define BMA_POSIX_THRDS 0
	typedef char bma_Mtx;
	typedef char bma_Cnd;
	typedef char bma_Rw;
	typedef char bma_Thrd;
#endif

BMA_DEF void bma_Mtx_ctor(bma_Mtx *pSelf);
BMA_DEF void bma_Mtx_dtor(bma_Mtx* pSelf, bma_IMemAlloc *pAlloc);
BMA_DEF void bma_Mtx_lck(bma_Mtx* pSelf);
BMA_DEF void bma_Mtx_unlck(bma_Mtx* pSelf);

BMA_DEF void bma_Rw_ctor(bma_Rw *pSelf);
BMA_DEF void bma_Rw_dtor(bma_Rw *pSelf, bma_IMemAlloc *pAlloc);
BMA_DEF void bma_Rw_lckRd(bma_Rw* pSelf);
BMA_DEF void bma_Rw_unlckRd(bma_Rw* pSelf);
BMA_DEF void bma_Rw_lckWrt(bma_Rw* pSelf);
BMA_DEF void bma_Rw_unlckWrt(bma_Rw* pSelf);

BMA_DEF void bma_Cnd_ctor(bma_Cnd *pSelf);
BMA_DEF void bma_Cnd_dtor(bma_Cnd *pSelf);
BMA_DEF void bma_Cnd_wait(bma_Cnd *pSelf, bma_Mtx *pMtx);
#define bma_Cnd_waitUntil(pSelf, pMtx, cond) \
	do { \
		while (!(cond)) { \
			bma_Cnd_wait(pSelf, pMtx); \
		} \
	} while (0)
BMA_DEF void bma_Cnd_sgnlOne(bma_Cnd *pSelf);
BMA_DEF void bma_Cnd_sgnlAll(bma_Cnd *pSelf);

typedef void (*bm_thrdfnc_t)(void* pUsrDta);

BMA_DEF void bma_Thrd_ctor(bma_Thrd* pSelf, bm_thrdfnc_t pFunc, void *pUsrDta);
BMA_DEF void bma_Thrd_dtor(bma_Thrd* pSelf, bma_IMemAlloc *pAlloc);

BMA_DEF void bma_sleepMs(unsigned int ms);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_THRD_IMPL

#ifdef __cplusplus
	extern "C" {
#endif

BMA_DEF bma_Shrd *BMA_DBG_SFFX(bma_Shrd_new_impl)(
	size_t sz,
	bma_dtor_t pDtor,
	bma_IMemAlloc *pAlloc
	BMA_DBG_OPT_PARAM(const char* pType)
) {
	bma_assert(sz > 0);
	bma_assert(pAlloc != NULL);
	bma_Shrd *p = (bma_Shrd*)(*pAlloc->pRllc)(pAlloc, NULL, BMA_SHRD_HDR_SZ + sz, NULL);
	p->strongRefs = 1;
	p->weakRefs = 1;
	p->alive = 1;
	p->pDtor = pDtor;
	p->pAlloc = pAlloc;
#if BMA_DBG
	p->pType = pType;
#endif
	return p;
}

#if BMA_DBG

BMA_DEF void *bma_Shrd_get_impl_D(bma_Shrd *pSelf, size_t typeSz, const char *pType) {
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->pType != NULL);
	bma_assert(typeSz > 0);
	bma_assert(strcmp(pType, pSelf->pType) == 0);
	bma_assert(pSelf->weakRefs > 0);
	bma_assert(pSelf->strongRefs > 0);
	bma_assert(pSelf->alive > 0);
	return ((char*)pSelf) + BMA_SHRD_HDR_SZ;
}

#endif

BMA_DEF void bma_Shrd_inc(bma_Shrd *pSelf) {
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->weakRefs > 0);
	bma_assert(pSelf->strongRefs > 0);
	(void)bma_atmc_inc(&pSelf->weakRefs);
	(void)bma_atmc_inc(&pSelf->strongRefs);
}

BMA_DEF void bma_Shrd_dec(bma_Shrd *pSelf) {
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->pAlloc != NULL);
	bma_assert(pSelf->weakRefs > 0);
	bma_assert(pSelf->strongRefs > 0);
	if (bma_atmc_dec(&pSelf->strongRefs) == 0) {
		if (bma_atmc_cas(&pSelf->alive, 1, 0) == 1) {
			if (pSelf->pDtor != NULL) {
				pSelf->pDtor(((char*)pSelf) + BMA_SHRD_HDR_SZ, pSelf->pAlloc);
				pSelf->pDtor = NULL;
			}
		}
	}
	if (bma_atmc_dec(&pSelf->weakRefs) == 0) {
		bma_IMemAlloc *pAlloc = pSelf->pAlloc;
#if BMA_DBG
		memset(pSelf, 0xFF, BMA_SHRD_HDR_SZ);
#endif
		(*pAlloc->pRllc)(pAlloc, pSelf, 0, NULL);
	}
}

BMA_DEF bma_Wk *bma_Wk_new(bma_Shrd *pShrd) {
	bma_assert(pShrd != NULL);
	bma_Wk *pWk = bma_lndr_cast(bma_Wk, pShrd);
	(void)bma_atmc_inc(&pWk->target.weakRefs);
	return pWk;
}

BMA_DEF bma_Shrd *bma_Wk_lock(bma_Wk *pSelf) {
	bma_atmc_t newStrngRefs;
	bma_assert(pSelf != NULL);
	(void)bma_atmc_inc(&pSelf->target.weakRefs); /* prevent object from being freed while we are locking it */
	newStrngRefs = bma_atmc_inc(&pSelf->target.strongRefs);
	if (newStrngRefs == 1) { /* was 0 before */
		bma_Shrd_dec(&pSelf->target);
		return NULL;
	}
	if (bma_atmc_cas(&pSelf->target.alive, 0, 0) == 0) {
		bma_Shrd_dec(&pSelf->target);
		return NULL;
	}
	return &pSelf->target;
}

BMA_DEF void bma_Wk_inc(bma_Wk *pSelf) {
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->target.weakRefs > 0);
	(void)bma_atmc_inc(&pSelf->target.weakRefs);
}

BMA_DEF void bma_Wk_dec(bma_Wk *pSelf) {
	bma_assert(pSelf != NULL);
	bma_assert(pSelf->target.pAlloc != NULL);
	bma_assert(pSelf->target.weakRefs > 0);
	if (bma_atmc_dec(&pSelf->target.weakRefs) == 0) {
		bma_IMemAlloc *pAlloc = pSelf->target.pAlloc;
#if BMA_DBG
		memset(pSelf, 0xFF, BMA_SHRD_HDR_SZ);
#endif
		(*pAlloc->pRllc)(pAlloc, pSelf, 0, NULL);
	}
}

BMA_DEF void bma_Mtx_ctor(bma_Mtx *pSelf) {
	#if BMA_WIN_THRDS
		InitializeCriticalSection(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_mutex_init(pSelf, NULL) != 0) {
			bma_exit_err("pthread_mutex_init() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_ctor()"
	#endif
}

BMA_DEF void bma_Mtx_dtor(bma_Mtx* pSelf, bma_IMemAlloc *pAlloc) {
	(void)pAlloc;
	#if BMA_WIN_THRDS
		DeleteCriticalSection(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_mutex_destroy(pSelf) != 0) {
			bma_exit_err("pthread_mutex_destroy() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_dtor()"
	#endif
}

BMA_DEF void bma_Mtx_lck(bma_Mtx* pSelf) {
	#if BMA_WIN_THRDS
		EnterCriticalSection(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_mutex_lock(pSelf) != 0) {
			bma_exit_err("pthread_mutex_lock() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_lck()"
	#endif
}

BMA_DEF void bma_Mtx_unlck(bma_Mtx* pSelf) {
	#if BMA_WIN_THRDS
		LeaveCriticalSection(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_mutex_unlock(pSelf) != 0) {
			bma_exit_err("pthread_mutex_unlock() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_unlck()"
	#endif
}

BMA_DEF void bma_Rw_ctor(bma_Rw *pSelf) {
	#if BMA_WIN_THRDS
		InitializeSRWLock(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_rwlock_init(pSelf, NULL) != 0) {
			bma_exit_err("pthread_rwlock_init() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Rw_ctor()"
	#endif
}

BMA_DEF void bma_Rw_dtor(bma_Rw *pSelf, bma_IMemAlloc *pAlloc) {
	(void)pAlloc;
	#if BMA_WIN_THRDS
	(void)pSelf; /* WinAPI doesn't need SRWLocks to be destroyed funny enough */
	#elif BMA_POSIX_THRDS
		if (pthread_rwlock_destroy(pSelf) != 0) {
			bma_exit_err("pthread_rwlock_destroy() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Rw_dtor()"
	#endif
}

BMA_DEF void bma_Rw_lckRd(bma_Rw* pSelf) {
	#if BMA_WIN_THRDS
		AcquireSRWLockShared(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_rwlock_rdlock(pSelf) != 0) {
			bma_exit_err("pthread_rwlock_rdlock() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Rw_lckRd()"
	#endif
}

BMA_DEF void bma_Rw_unlckRd(bma_Rw* pSelf) {
	#if BMA_WIN_THRDS
		ReleaseSRWLockShared(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_rwlock_unlock(pSelf) != 0) {
			bma_exit_err("pthread_rwlock_unlock() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Rw_unlckRd()"
	#endif
}

BMA_DEF void bma_Rw_lckWrt(bma_Rw* pSelf) {
	#if BMA_WIN_THRDS
		AcquireSRWLockExclusive(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_rwlock_wrlock(pSelf) != 0) {
			bma_exit_err("pthread_rwlock_wrlock() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Rw_lckWrt()"
	#endif
}

BMA_DEF void bma_Rw_unlckWrt(bma_Rw* pSelf) {
	#if BMA_WIN_THRDS
		ReleaseSRWLockExclusive(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_rwlock_unlock(pSelf) != 0) {
			bma_exit_err("pthread_rwlock_unlock() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Rw_unlckWrt()"
	#endif
}

BMA_DEF void bma_Cnd_ctor(bma_Cnd *pSelf) {
	#if BMA_WIN_THRDS
		InitializeConditionVariable(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_cond_init(pSelf, NULL) != 0) {
			bma_exit_err("pthread_cond_init() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_dtor()"
	#endif
}

BMA_DEF void bma_Cnd_dtor(bma_Cnd *pSelf) {
	#if BMA_WIN_THRDS
	(void)pSelf; /* WinAPI doesn't need condvars to be destroyed funny enough */
	#elif BMA_POSIX_THRDS
		if (pthread_cond_destroy(pSelf) != 0) {
			bma_exit_err("pthread_cond_destroy() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_dtor()"
	#endif
}

BMA_DEF void bma_Cnd_wait(bma_Cnd *pSelf, bma_Mtx *pMtx) {
	#if BMA_WIN_THRDS
		if (!SleepConditionVariableCS(pSelf, pMtx, INFINITE)) {
			bma_exit_err("SleepConditionVariableCS() failed");
		}
	#elif BMA_POSIX_THRDS
		if (pthread_cond_wait(pSelf, pMtx) != 0) {
			bma_exit_err("pthread_cond_wait() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
		(void)pMtx;
	#else
		#error "Missing impl. for bma_Mtx_dtor()"
	#endif
}

BMA_DEF void bma_Cnd_sgnlOne(bma_Cnd *pSelf) {
	#if BMA_WIN_THRDS
		WakeConditionVariable(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_cond_signal(pSelf) != 0) {
			bma_exit_err("pthread_cond_signal() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_dtor()"
	#endif
}

BMA_DEF void bma_Cnd_sgnlAll(bma_Cnd *pSelf) {
	#if BMA_WIN_THRDS
		WakeAllConditionVariable(pSelf);
	#elif BMA_POSIX_THRDS
		if (pthread_cond_broadcast(pSelf) != 0) {
			bma_exit_err("pthread_cond_broadcast() failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_dtor()"
	#endif
}

#if BMA_WIN_THRDS

typedef struct bma_WinThrdParam {
	bm_thrdfnc_t pFunc;
	void *pUsrDta;
} bma_WinThrdParam;

static DWORD WINAPI bma_WinThrdFunc(LPVOID lpParam) {
	bm_thrdfnc_t pFunc;
	void *pUsrDta;
	bma_WinThrdParam *pParam = (bma_WinThrdParam*)lpParam;
	bma_assert(pParam != NULL);
	pFunc = pParam->pFunc;
	pUsrDta = pParam->pUsrDta;
	bma_assert(pFunc != NULL);
	bma_free(pParam);
	(*pFunc)(pUsrDta);
	return 0;
}

#elif BMA_POSIX_THRDS

typedef struct bma_PosixThrdParam {
	bm_thrdfnc_t pFunc;
	void *pUsrDta;
} bma_PosixThrdParam;

static void *bma_PosixThrdFunc(void *pParam_) {
	bm_thrdfnc_t pFunc;
	void *pUsrDta;
	bma_PosixThrdParam *pParam = (bma_PosixThrdParam*)pParam_;
	bma_assert(pParam != NULL);
	pFunc = pParam->pFunc;
	pUsrDta = pParam->pUsrDta;
	bma_assert(pFunc != NULL);
	bma_free(pParam);
	(*pFunc)(pUsrDta);
	return NULL;
}

#endif

BMA_DEF void bma_Thrd_ctor(bma_Thrd* pSelf, bm_thrdfnc_t pFunc, void *pUsrDta) {
	#if BMA_WIN_THRDS
		HANDLE hThread;
		bma_WinThrdParam *pParam;
		bma_assert(pSelf != NULL);
		bma_assert(pFunc != NULL);
		pParam = bma_malloc(bma_WinThrdParam);
		pParam->pFunc = pFunc;
		pParam->pUsrDta = pUsrDta;
		hThread = CreateThread(NULL, 0, &bma_WinThrdFunc, pParam, 0, NULL);
		if (hThread == NULL) {
			bma_free(pParam);
			bma_exit_err("CreateThread() failed");
		}
		*pSelf = hThread;
	#elif BMA_POSIX_THRDS
		pthread_t thread;
		bma_PosixThrdParam *pParam;
		bma_assert(pSelf != NULL);
		bma_assert(pFunc != NULL);
		pParam = bma_malloc(bma_PosixThrdParam);
		pParam->pFunc = pFunc;
		pParam->pUsrDta = pUsrDta;
		if (pthread_create(&thread, NULL, &bma_PosixThrdFunc, pParam) != 0) {
			bma_free(pParam);
			bma_exit_err("pthread_create() failed");
		}
		*pSelf = thread;
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_dtor()"
	#endif
}

BMA_DEF void bma_Thrd_dtor(bma_Thrd* pSelf, bma_IMemAlloc *pAlloc) {
	#if BMA_WIN_THRDS
		DWORD waitResult;
		(void)pAlloc;
		assert(pSelf != NULL);
		waitResult = WaitForSingleObject(*pSelf, INFINITE);
		if (waitResult == WAIT_FAILED) {
			bma_exit_err("WaitForSingleObject() on thread failed");
		}
		(void)CloseHandle(*pSelf);
	#elif BMA_POSIX_THRDS
		(void)pAlloc;
		assert(pSelf != NULL);
		if (pthread_join(*pSelf, NULL) != 0) {
			bma_exit_err("pthread_join() on thread failed");
		}
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_dtor()"
	#endif
}

BMA_DEF void bma_sleepMs(unsigned int ms) {
	if (ms == 0) ms = 1;
	#if BMA_WIN_THRDS
		#if (UINT_MAX >= 0xFFFFFFFF) /* at least size of DWORD */
			if (ms == INFINITE) --ms; /* bma_sleep has no infinite support */
		#endif
		Sleep((DWORD)ms);
	#elif BMA_POSIX_THRDS
		struct timespec ts;
		ts.tv_sec = (time_t)(ms / 1000);
		ts.tv_nsec = (long)((ms % 1000) * 1000000);
		clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
	#elif (BMA_HAS_THRDS == 0)
		(void)pSelf;
	#else
		#error "Missing impl. for bma_Mtx_dtor()"
	#endif
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_LOG_IMPL */

#ifdef BMA_THRD_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

bma_bool_t g_shrdDtorRan = BMA_FALSE;

void test_ShrdInt_dtor(void* p, bma_IMemAlloc *pAlloc) {
	(void)pAlloc;
	BMA_EXPECT(*(int*)p == 42);
	g_shrdDtorRan = BMA_TRUE;
}

BMA_DEF_SHRD(bma_TestShrdInt, bma_TestWkInt, int, &test_ShrdInt_dtor)

void test_shrd_snglThrd(void) {
	bma_TestShrdInt *p, *p3;
	bma_TestWkInt *p2;
	g_shrdDtorRan = BMA_FALSE;
	p = bma_TestShrdInt_new();
	BMA_EXPECT(p != NULL);
	BMA_EXPECT(p->impl.strongRefs == 1);
	BMA_EXPECT(p->impl.weakRefs == 1);
	BMA_EXPECT(p->impl.alive == 1);
	BMA_EXPECT(p->impl.pDtor == &test_ShrdInt_dtor);
	BMA_EXPECT(p->impl.pAlloc == bma_getDfltMemAlloc());
	*bma_TestShrdInt_get(p) = 42;
	BMA_EXPECT(*bma_TestShrdInt_get(p) == 42);
	bma_TestShrdInt_dec(p);
	BMA_EXPECT(g_shrdDtorRan);
	g_shrdDtorRan = BMA_FALSE;
	p = bma_TestShrdInt_new();
	*bma_TestShrdInt_get(p) = 42;
	bma_TestShrdInt_inc(p);
	BMA_EXPECT(p->impl.strongRefs == 2);
	BMA_EXPECT(p->impl.weakRefs == 2);
	BMA_EXPECT(p->impl.alive == 1);
	bma_TestShrdInt_dec(p);
	BMA_EXPECT(p->impl.strongRefs == 1);
	BMA_EXPECT(p->impl.weakRefs == 1);
	BMA_EXPECT(p->impl.alive == 1);
	BMA_EXPECT(!g_shrdDtorRan);
	bma_TestShrdInt_dec(p);
	BMA_EXPECT(g_shrdDtorRan);
	g_shrdDtorRan = BMA_FALSE;
	p = bma_TestShrdInt_new();
	*bma_TestShrdInt_get(p) = 42;
	p2 = bma_TestWkInt_new(p);
	BMA_EXPECT(p->impl.strongRefs == 1);
	BMA_EXPECT(p->impl.weakRefs == 2);
	bma_TestShrdInt_dec(p);
	BMA_EXPECT(g_shrdDtorRan);
	g_shrdDtorRan = BMA_FALSE;
	BMA_EXPECT(p->impl.strongRefs == 0);
	BMA_EXPECT(p->impl.weakRefs == 1);
	BMA_EXPECT(p->impl.alive == 0);
	bma_TestWkInt_dec(p2);
	p = bma_TestShrdInt_new();
	*bma_TestShrdInt_get(p) = 42;
	p2 = bma_TestWkInt_new(p);
	BMA_EXPECT(p->impl.strongRefs == 1);
	BMA_EXPECT(p->impl.weakRefs == 2);
	p3 = bma_TestWkInt_lock(p2);
	BMA_EXPECT(p3 != NULL);
	BMA_EXPECT(p3 == p);
	BMA_EXPECT(p->impl.strongRefs == 2);
	BMA_EXPECT(p->impl.weakRefs == 3);
	bma_TestWkInt_dec(p2);
	BMA_EXPECT(p3->impl.strongRefs == 2);
	BMA_EXPECT(p3->impl.weakRefs == 2);
	bma_TestShrdInt_dec(p);
	BMA_EXPECT(p3->impl.strongRefs == 1);
	BMA_EXPECT(p3->impl.weakRefs == 1);
	bma_TestWkInt_inc(p2);
	BMA_EXPECT(p3->impl.strongRefs == 1);
	BMA_EXPECT(p3->impl.weakRefs == 2);
	bma_TestShrdInt_dec(p);
	BMA_EXPECT(p3->impl.strongRefs == 0);
	BMA_EXPECT(p3->impl.weakRefs == 1);
	BMA_EXPECT(bma_TestWkInt_lock(p2) == NULL);
	bma_TestWkInt_dec(p2);
}

#if BMA_HAS_THRDS

static void test_mtx_thrdFnc(void* pMtx_) {
	size_t i;
	bma_Mtx *pMtx = (bma_Mtx*)pMtx_;
	for (i=0; i<10u; ++i) {
		bma_Mtx_lck(pMtx);
		bma_sleepMs(1u);
		bma_Mtx_unlck(pMtx);
		bma_sleepMs(1u);
	}
}

#endif

void test_mtx(void) {
	#if BMA_HAS_THRDS
		bma_Mtx mtx;
		bma_Thrd thrd[10u];
		size_t i;
		bma_Mtx_ctor(&mtx);
		for (i=0; i<bma_ary_sz(thrd); ++i) {
			bma_Thrd_ctor(&thrd[i], &test_mtx_thrdFnc, &mtx);
		}
		i = bma_ary_sz(thrd);
		while (i-- > 0) {
			bma_Thrd_dtor(&thrd[i], NULL);
		}
		bma_Mtx_dtor(&mtx, NULL);
	#endif
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_shrd_snglThrd);
	BMA_TEST(test_mtx);
	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_THRD_TEST */

#endif /* BMA_THRD_H_INCLUDED */
