#define BMA_IMPL
#include <stdio.h>
#include <string.h>
#include <bma/cmdln.h>
#include <bma/fs.h>
#include <bma/utils.h>
#include <bma/log.h>

#define BMA_CMD_TYPE_GET_INC 1

typedef struct bma_CmdLnArgs {
	unsigned int cmd;
	const char *pFilePath;
} BmaCmdLnArgs;

static const bma_CmdLnArgDef kGetIncArgs[] = {
	{
		 /* type    */ BMA_CMDLN_ARG_TYPE_STR
		,/* pShort  */ "f"
		,/* pLong   */ "file"
		,/* pDesc   */ "What file to check for include statements"
		,/* reqred  */ BMA_TRUE
		,/* memOffs */ offsetof(BmaCmdLnArgs, pFilePath)
	}
};

static const bma_CmdLnCmdDef kMainSubCmds[] = {
	{
		 /* pName      */ "getinc"
		,/* pDesc      */ "Scan a C or C++ source/include file and print the file names contained in the #include statements"
		,/* enumOffs   */ offsetof(BmaCmdLnArgs, cmd)
		,/* enumVal    */ BMA_CMD_TYPE_GET_INC
		,/* pSubCmds   */ NULL /* no sub-commands*/
		,/* numSubCmds */ 0
		,/* pArgs      */ kGetIncArgs
		,/* numArgs    */ bma_ary_sz(kGetIncArgs)
	}
};

static const bma_CmdLnCmdDef kMainCmd = {
		 /* pName      */ ""
		,/* pDesc      */ "Builds the current project"
		,/* enumOffs   */ BMA_SIZE_MAX
		,/* enumVal    */ 0
		,/* pSubCmds   */ kMainSubCmds
		,/* numSubCmds */ bma_ary_sz(kMainSubCmds)
		,/* pArgs      */ NULL
		,/* numArgs    */ 0
};

BMA_DEF void BmaCmdLnArgs_dtor(BmaCmdLnArgs *pSelf, bma_IMemAlloc *pAlloc) {
	if (pSelf->pFilePath != NULL) {
		bma_free_ext(pAlloc, (void*)pSelf->pFilePath);
		pSelf->pFilePath = NULL;
	}
}

typedef struct MainClnUpUsrData {
	bma_bool_t dummy; /* no real usr data needed yet */
} MainClnUpUsrData;

void mainClnUp(void *pClnUpUsrData) {
	MainClnUpUsrData* pMainClnUpUsrData = (MainClnUpUsrData*)pClnUpUsrData;
	(void)pMainClnUpUsrData;
	bma_Log_clnup();
}

bma_bool_t getinc(const char *pSourceFile) {
	bma_ScnndIncldStmtVec scnndIncldStmnts;
	bma_StrBldr fileContent;
	bma_StrBldr errFmt;
	bma_bool_t result = BMA_FALSE;
	bma_assert(pSourceFile != NULL);
	bma_StrBldr_ctor(&fileContent);
	bma_StrBldr_ctor(&errFmt);
	bma_ScnndIncldStmtVec_ctor(&scnndIncldStmnts);
	if (!bma_rdFile(pSourceFile, &fileContent)) {
		fprintf(stderr, "File '%s' not found or can't be opened.", pSourceFile);
		fflush(stderr);
		goto out;
	}
	result = bma_scnFrInclds_ext(bma_StrBldr_getStr(&fileContent),
	                             bma_StrBldr_getSz(&fileContent),
	                             &scnndIncldStmnts,
	                             &errFmt);
	if (result) {
		size_t i;
		if (bma_StrBldr_getSz(&errFmt) > 0) {
			fprintf(stderr, "WARNING: %s\n", bma_StrBldr_getStr(&errFmt));
			fflush(stderr);
		}
		printf("Include statements of file '%s':\n", pSourceFile);
		if (bma_ScnndIncldStmtVec_getSz(&scnndIncldStmnts) == 0) {
			printf("  None.\n");
		}
		for (i=0; i<bma_ScnndIncldStmtVec_getSz(&scnndIncldStmnts); ++i) {
			const bma_ScnndIncldStmt* pIncStmnt = bma_ScnndIncldStmtVec_at(&scnndIncldStmnts, i);
			bma_assert(pIncStmnt != NULL);
			printf("  Type: '%s', path: '%s', conditional: '%s'\n",
			       (pIncStmnt->type == BMA_SCNND_INCLD_STMT_TYPE_SYS_FIRST) ? "system" : "local",
			       pIncStmnt->pPath,
			       bma_bool2str(pIncStmnt->isCndtnl));
		}
	}
	else
	{
		if (bma_StrBldr_getSz(&errFmt) > 0) {
			fprintf(stderr, "ERROR: %s\n", bma_StrBldr_getStr(&errFmt));
			fflush(stderr);
		}
	}
out:
	bma_ScnndIncldStmtVec_dtor(&scnndIncldStmnts, NULL);
	bma_StrBldr_dtor(&errFmt, NULL);
	bma_StrBldr_dtor(&fileContent, NULL);
	return result;
}

int main(int argc, char *argv[]) {
	BmaCmdLnArgs cmdLnArgs;
	int resultCode = 0; /* NOLINT */
	MainClnUpUsrData mainClnUpUsrData;
	memset(&cmdLnArgs, 0, sizeof(cmdLnArgs));
	memset(&mainClnUpUsrData, 0, sizeof(mainClnUpUsrData));
	bma_Log_init();
	bma_CmdLn_prseOrExit(argc, argv,
	                     &kMainCmd,
	                     &cmdLnArgs,
	                     (bma_dtor_t)&BmaCmdLnArgs_dtor,
	                     &mainClnUp,
	                     &mainClnUpUsrData);
	switch(cmdLnArgs.cmd)
	{
	case BMA_CMD_TYPE_GET_INC: {
		bma_assert(cmdLnArgs.pFilePath != NULL); /* because it was marked as mandatory */
		resultCode = (getinc(cmdLnArgs.pFilePath)) ? 0 : 1;
		goto out;
	}
	default: {
		fprintf(stderr, "Something very unpredictable happened... Please investigate!\n");
		fflush(stderr);
		resultCode = 1;
		goto out; /* NOLINT */
	}
	}
out:
	mainClnUp(&mainClnUpUsrData);
	return resultCode;
}
