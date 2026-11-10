#include <stdio.h>
#include <string.h>
#define BMA_IMPL
#include <bma/ccal.h>
#include <bma/cmdln.h>
#include <bma/fs.h>
#include <bma/nj.h>
#include <bma/utils.h>
#include <bma/log.h>

#define BMA_CMD_TYPE_BAUMA   1
#define BMA_CMD_TYPE_GET_INC 2

#define BMA_HME_SUB_DIR ".bauma"
#define BMA_DFLT_PRFL_NAME "dflt_dbg_prfl.nj"

typedef struct bma_CmdLnArgs {
	unsigned int cmd;
	const char *pFilePath;
} BmaCmdLnArgs;

bma_NjNode* bma_rdNjFile(const char *pPath) {
	bma_NjNode* pRet = NULL;
	bma_StrBldr cntnt;
	bma_StrBldr errFmt;
	bma_NjParseSrc parseSrc;
	bma_StrBldr_ctor(&cntnt);
	bma_StrBldr_ctor(&errFmt);
	if (!bma_isFile(pPath)) {
		fprintf(stderr, "File '%s' not found.\n", pPath);
		fflush(stderr);
		goto out;
	}
	if (!bma_rdFile(pPath, &cntnt)) {
		fprintf(stderr, "Could not read file '%s'.\n", pPath);
		fflush(stderr);
		goto out;
	}
	parseSrc.pStr = bma_StrBldr_getStr(&cntnt);
	parseSrc.pBegin = parseSrc.pStr;
	parseSrc.len = bma_StrBldr_getSz(&cntnt);
	pRet = bma_njParseExt(&parseSrc, &errFmt, bma_getDfltMemAlloc());
	if (pRet == NULL) {
		fprintf(stderr, "Error parsing file '%s': %s\n", pPath, bma_StrBldr_getStr(&errFmt));
		fflush(stderr);
		goto out;
	}
out:
	bma_StrBldr_dtor(&errFmt, NULL);
	bma_StrBldr_dtor(&cntnt, NULL);
	return pRet;
}

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
		,/* enumOffs   */ offsetof(BmaCmdLnArgs, cmd)
		,/* enumVal    */ BMA_CMD_TYPE_BAUMA
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

bma_bool_t build(void) {
	bma_DirEntVec v;
	bma_DirEntVec_ctor(&v);
	bma_scnFrFles("C:\\projects\\bauma", &v, BMA_TRUE, NULL, NULL);
	bma_DirEntVec_dtor(&v, NULL);

	bma_bool_t result = BMA_FALSE;
	bma_StrBldr path;
	bma_NjNode *pRoot = NULL;
	bma_StrBldr_ctor(&path);
	if (!bma_getHomeDir(&path)) {
		fprintf(stderr, "Could not determine home directory.\n");
		fflush(stderr);
		goto out;	
	}
	bma_entrPth(&path, BMA_HME_SUB_DIR);
	bma_entrPth(&path, BMA_DFLT_PRFL_NAME);
	if (!bma_isFile(bma_StrBldr_getStr(&path))) {
		fprintf(stderr, "Default profile file '%s' not found.\n", bma_StrBldr_getStr(&path));
		fflush(stderr);
		goto out;
	}
	pRoot = bma_rdNjFile(bma_StrBldr_getStr(&path));
	bma_StrBldr_clear(&path);
	if (!bma_getCwd(&path)) {
		fprintf(stderr, "Could not determine current working directory.\n");
		fflush(stderr);
		goto out;
	}
	result = BMA_TRUE;
out:
	bma_njDelete(pRoot);
	bma_StrBldr_dtor(&path, NULL);
	return result;
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
	case BMA_CMD_TYPE_BAUMA: {
		resultCode = (build()) ? 0 : 1;
		goto out;
	}
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
