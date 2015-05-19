#include "iisam.h"
#include "netutil.h"
#include "spec.h"
#include "windos.h"
/* This module contains all our settings */

BOOL InitializeSpec(IISAM_SETTINGS *pSettings)
{
	memset(pSettings, 0, sizeof(IISAM_SETTINGS));
	return TRUE;
}

IISAM_CFG *ConfigInit(IISAM_SETTINGS *pSettings)
{
	IISAM_CFG *pCfg = ConfigAllocate();
	static WCHAR szWinDir[MAX_PATH];

	if (pCfg)
	{
		ConfigAddBoolParameter(pCfg, L"Trace", &pSettings->bTrace, FALSE);
		DOSFindLocalDirectory(szWinDir);
		wcscat (szWinDir, L"\\iisam.log");
		ConfigAddStrParameter(pCfg, L"TraceFile", pSettings->szTraceFile, szWinDir);
	}
	return pCfg;
}

