#define BMA_CCAL_IMPL
#define BMA_CMDSL_IMPL
#define BMA_FS_IMPL
#define BMA_NSON_IMPL
#define BMA_THRD_IMPL
#define BMA_LOG_IMPL

#include <bma/ccal.h>
#include <bma/cmdsl.h>
#include <bma/fs.h>
#include <bma/nson.h>
#include <bma/thrd.h>
#include <bma/log.h>

int main(int argc, char *argv[]) {
	bma_Log *pLog;
	bma_StrBldr bldr;
	(void)argc;
	(void)argv;
	bma_Log_init();
	bma_Log_addCnslSink("", BMA_LOG_LEVEL_DEBUG);
	pLog = bma_Log_get("main");
	bma_StrBldr_ctor(&bldr);
	bma_getHomeDir(&bldr);
	bma_entrPth(&bldr, ".bauma" BMA_FS_SEP "profiles" BMA_FS_SEP "default.nson");
	if (bma_isFile(bma_StrBldr_getStr(&bldr))) {
		bma_StrBldr content;
		bma_NsonNode *pNode;
		bma_StrBldr_ctor(&content);
		if (!bma_rdFile(bma_StrBldr_getStr(&bldr), &content)) bma_StrBldr_clear(&content);
		pNode = bma_nsonParse(bma_StrBldr_getStr(&content));
		bma_StrBldr_clear(&content);
		if (!bma_nsonGetString(&content, pNode, "target/vendor")) bma_StrBldr_clear(&content);
		bma_Log_debug_1(pLog, "Target vendor is '%s'", bma_StrBldr_getStr(&content));
		bma_StrBldr_dtor(&content, NULL);
		bma_nsonDelete(pNode);
	}
	bma_Log_debug_1(pLog, "Home dir is '%s'", bma_StrBldr_getStr(&bldr));
	bma_StrBldr_dtor(&bldr, NULL);
	bma_Log_clnup();
	return 0;
}
