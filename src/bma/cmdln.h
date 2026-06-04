#ifndef BMA_CMDLN_H_INCLUDED
#define BMA_CMDLN_H_INCLUDED

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

Bau 'ma's command line parser
*/
#include <bma/ccal.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct bma_CmdLnOptDef {
	const char *pShort;
	const char *pLong;
	const char *pDesc;
	bma_bool_t required;
} bma_CmdLnOptDef;

typedef struct bma_CmdLnDef {
	size_t minNumStdArgs;
	size_t maxNumStdArgs;
	size_t numOptns;
	const bma_CmdLnOptDef *pOptns;
} bma_CmdLnDef;

BMA_DEF_VEC(bma_CmdLnStrVec, bma_StrN, (bma_dtor_t)&bma_StrN_dtor)
BMA_DEF_HSHMP(bma_CmdLnStrMap, bma_StrN, bma_StrN, (bma_dtor_t)&bma_StrN_dtor, (bma_dtor_t)&bma_StrN_dtor, (bma_hash_t)&bma_StrN_hash, (bma_eq_t)&bma_StrN_eq)

BMA_DEF bma_bool_t bma_CmdLn_parse(int argc, char **argv, const bma_CmdLnDef *pDef, bma_CmdLnStrVec *pOptOutStdArgs, bma_CmdLnStrMap *pOptOutOpts);
BMA_DEF void bma_CmdLn_parseOrExit(int argc, char **argv, const bma_CmdLnDef *pDef, bma_CmdLnStrVec *pOptOutStdArgs, bma_CmdLnStrMap *pOptOutOpts);

#ifdef __cplusplus
	} /* extern "C" */
#endif

#ifdef BMA_CMDLN_IMPL

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
	extern "C" {
#endif

BMA_DEF bma_bool_t bma_CmdLn_parse(int argc, char **argv, const bma_CmdLnDef *pDef, bma_CmdLnStrVec *pOptOutStdArgs, bma_CmdLnStrMap *pOptOutOpts) {
	int i;
	const char *pArg;
	size_t numStdArgs = 0;
	bma_assert(argc > 0);
	bma_assert(argv != NULL);
	bma_assert(pDef != NULL);
	for (i=1; i<argc; ++i) {
		pArg = argv[i];
		if (bma_strtsWth(pArg, "--")) {
		}
		else if (bma_strtsWth(pArg, "-")) {
		}
		else {
		}
	}
	return BMA_TRUE;
}

BMA_DEF void bma_CmdLn_parseOrExit(int argc, char **argv, const bma_CmdLnDef *pDef, bma_CmdLnStrVec *pOptOutStdArgs, bma_CmdLnStrMap *pOptOutOpts) {
	if (!bma_CmdLn_parse(argc, argv, pDef, pOptOutStdArgs, pOptOutOpts)) {
		exit(1);
	}
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

#endif /* BMA_CMDLN_IMPL */

#ifdef BMA_CMDLN_TEST

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

void test_construct(void) {
}

#ifdef __cplusplus
	} /* extern "C" */
#endif

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	BMA_TEST(test_construct);

	printf("All tests passed.\n");
	fflush(stdout);
	return 0;
}

#endif /* BMA_CMDSL_TEST */

#endif /* BMA_CMDSL_H_INCLUDED */
