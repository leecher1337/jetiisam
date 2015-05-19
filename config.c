#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <esent.h>
#include <stdio.h>
#include "winalloc.h"
#include "jetuwrap.h"
#include "spec.h"

struct tag_cfg_param;
typedef struct tag_cfg_param IISAM_CFGPARAM;
struct tag_cfg_param {
	struct tag_cfg_param *next;	// + 0
	BYTE cType;					// + 4
	BYTE iKey;					// + 5
	WCHAR *pwszName;			// + 8
	PVOID pParamOut;			// + 12
	PVOID pDefault;				// + 16
	WCHAR *apwszEnum[3];		// + 20
};
struct tag_cfg {
	IISAM_CFGPARAM *next;
	BYTE nKeys;
};

enum
{
  TypeString,
  TypeInt,
  TypeBool,
  TypeEnum
};

// Prototypes
static void InitDefaults(IISAM_CFG *pCfg);
static BOOL ReadReg(WCHAR *pwszDriver, HKEY hPKey, WCHAR *pwszRegPath, IISAM_CFG *pCfg);


IISAM_CFG *ConfigAllocate()
{
	IISAM_CFG *pCfg;

	if (pCfg = MemAllocate(sizeof(IISAM_CFG)))
		pCfg->nKeys = 1;
	return pCfg;
}

void ConfigRelease(IISAM_CFG *pCfg)
{
	IISAM_CFGPARAM *pStart, *pNext;

	if (pStart = pCfg->next)
	{
		do
		{
			pNext = pStart->next;
			MemFree(pStart);
			pStart = pNext;
		} while (pNext);
	}
	MemFree(pCfg);
}

int ConfigAddStrParameter(IISAM_CFG *pCfg, WCHAR *pwszName, WCHAR *pwszParamOut, WCHAR *pwszDefault)
{
	IISAM_CFGPARAM *pParam;

	if (pParam = MemAllocate(sizeof(IISAM_CFGPARAM)))
	{
		pParam->cType = TypeString;
		pParam->iKey = pCfg->nKeys++;
		pParam->pwszName = pwszName;
		pParam->pParamOut = pwszParamOut;
		pParam->pDefault = pwszDefault;
		pParam->next = pCfg->next;
		pCfg->next = pParam;
		return 0;
	}
	return -1;
}

int ConfigAddIntParameter(IISAM_CFG *pCfg, WCHAR *pwszName, DWORD *pdwParamOut, DWORD dwDefault)
{
	IISAM_CFGPARAM *pParam;

	if (pParam = MemAllocate(sizeof(IISAM_CFGPARAM)))
	{
		pParam->cType = TypeInt;
		pParam->iKey = pCfg->nKeys++;
		pParam->pwszName = pwszName;
		pParam->pParamOut = pdwParamOut;
		pParam->pDefault = (PVOID)dwDefault;
		pParam->next = pCfg->next;
		pCfg->next = pParam;
		return 0;
	}
	return -1;
}

int ConfigAddBoolParameter(IISAM_CFG *pCfg, WCHAR *pwszName, BOOL *pbParamOut, BOOL bDefault)
{
	IISAM_CFGPARAM *pParam;

	if (pParam = MemAllocate(sizeof(IISAM_CFGPARAM)))
	{
		pParam->cType = TypeBool;
		pParam->iKey = pCfg->nKeys++;
		pParam->pwszName = pwszName;
		pParam->pParamOut = pbParamOut;
		pParam->pDefault = (PVOID)bDefault;
		pParam->next = pCfg->next;
		pCfg->next = pParam;
		return 0;
	}
	return -1;
}

int ConfigAddEnumParameter(IISAM_CFG *pCfg, WCHAR *pwszName, DWORD  *pdwParamOut, 
						   WCHAR *pEnum1, WCHAR *pEnum2, WCHAR *pEnum3, DWORD dwDefault)
{
	IISAM_CFGPARAM *pParam;

	if (pParam = MemAllocate(sizeof(IISAM_CFGPARAM)))
	{
		pParam->cType = TypeEnum;
		pParam->iKey = pCfg->nKeys++;
		pParam->pwszName = pwszName;
		pParam->pParamOut = pdwParamOut;
		pParam->pDefault = (PVOID)dwDefault;
		pParam->apwszEnum[0] = pEnum1;
		pParam->apwszEnum[1] = pEnum2;
		pParam->apwszEnum[2] = pEnum3;
		pParam->next = pCfg->next;
		pCfg->next = pParam;
		return 0;
	}
	return -1;
}


BOOL ConfigReadRegSpec(WCHAR *pwszDriver, WCHAR *pwszRegKey, HKEY hKey, WCHAR *pwszRegPath, IISAM_CFG *pCfg)
{
	InitDefaults(pCfg);
	if (!ReadReg(pwszDriver, NULL, pwszRegKey, pCfg))
		return ReadReg(pwszDriver, hKey, pwszRegPath, pCfg);
	return TRUE;
}


// ---- static --- //

static void InitDefaults(IISAM_CFG *pCfg)
{
	IISAM_CFGPARAM *pParam;

	for (pParam = pCfg->next; pParam; pParam=pParam->next)
	{
		switch (pParam->cType)
		{
		case TypeString:
			if (*((WCHAR*)pParam->pDefault))
				wcscpy(((WCHAR*)pParam->pParamOut), ((WCHAR*)pParam->pDefault));
			else
				*((WCHAR*)pParam->pParamOut) = 0;
			break;
		case TypeInt:
		case TypeEnum:
			*((DWORD*)pParam->pParamOut) = (DWORD)pParam->pDefault;
			break;
		case TypeBool:
			*((BOOL*)pParam->pParamOut) = (BOOL)pParam->pDefault;
			break;
		}
	}
}

static BOOL ReadReg(WCHAR *pwszDriver, HKEY hPKey, WCHAR *pwszRegPath, IISAM_CFG *pCfg)
{
	WCHAR wszRegPath[256];
	HKEY hKey;
	IISAM_CFGPARAM *pParam;
	DWORD cbData, dwType, dwVal;
	BYTE bVal;
	WCHAR wszData[1025];

	if (!*pwszRegPath) return FALSE;
	wcscpy (wszRegPath, pwszRegPath);
	wcscat (wszRegPath, L"\\Engines\\");
	wcscat (wszRegPath, pwszDriver);
	if (!hPKey) hPKey = HKEY_LOCAL_MACHINE;
	if (JetRegOpenKeyW(hPKey, wszRegPath, &hKey))
		return FALSE;

	for (pParam = pCfg->next; pParam; pParam=pParam->next)
	{
		switch (pParam->cType)
		{
		case TypeString:
			cbData = 1025;
			if (JetRegQueryValueExW(hKey, pParam->pwszName, NULL, &dwType, (BYTE*)wszData, &cbData) == ERROR_SUCCESS)
			{
				if (cbData > 0 && dwType == REG_SZ)
				{
					wszData[cbData] = 0;
					wcscpy ((WCHAR*)pParam->pParamOut, wszData);
				}
			}
			break;
		case TypeInt:
			cbData = sizeof(dwVal);
			if (JetRegQueryValueExW(hKey, pParam->pwszName, NULL, &dwType, (BYTE*)&dwVal, &cbData) == ERROR_SUCCESS)
			{
				if (cbData > 0 && dwType == REG_DWORD)
					*((DWORD*)pParam->pParamOut) = dwVal;
			}
			break;
		case TypeBool:
			cbData = sizeof(bVal);
			if (JetRegQueryValueExW(hKey, pParam->pwszName, NULL, &dwType, &bVal, &cbData) == ERROR_SUCCESS)
			{
				if (cbData > 0 && dwType == REG_BINARY)
					*((BOOL*)pParam->pParamOut) = bVal!=0;
			}
			break;
		case TypeEnum:
			cbData = sizeof(wszData);
			if (JetRegQueryValueExW(hKey, pParam->pwszName, NULL, &dwType, (BYTE*)wszData, &cbData) == ERROR_SUCCESS)
			{
				if (cbData > 0 && dwType == REG_SZ)
				{
					DWORD i;

					wszData[cbData] = 0;
					for (i=0; i<3; i++)
					{
						if (wcsicmp(wszData, pParam->apwszEnum[i]) == 0)
						{
							*((DWORD*)pParam->pParamOut) = i;
							break;
						}
					}
				}
			}
			break;
		}
	}
	RegCloseKey(hKey);
	return TRUE;
}

typedef UINT ( WINAPI *ft_CchLszOfId2)(UINT, WCHAR*, UINT);
ft_CchLszOfId2 CchLszOfId2;
static HMODULE m_hLibJint;
static BOOL LoadCchLszOfId2()
{
	if (!(m_hLibJint = JetLoadLibraryW(L"MSJINT40.DLL")))
		return FALSE;
	if (CchLszOfId2 = (ft_CchLszOfId2)GetProcAddress(m_hLibJint, "CchLszOfId2"))
		return TRUE;
	FreeLibrary(m_hLibJint);
	return FALSE;
}

static BOOL UnloadCchLszOfId2()
{
	BOOL bRet = FALSE;
	if (m_hLibJint && (bRet = FreeLibrary(m_hLibJint)))
	{
		m_hLibJint = NULL;
		CchLszOfId2 = NULL;
	}
	return bRet;
}

static LONG WriteString(HKEY hKey, LPCWSTR lpValueName, LPCWSTR lpValue)
{
	return JetRegSetValueExW(hKey, lpValueName, 0, REG_SZ, (LPBYTE)lpValue, wcslen(lpValue)*sizeof(WCHAR));
}

static LONG WriteDecodedString(HKEY hKey, LPCWSTR lpValueName, UINT id)
{
	WCHAR szBuf[512];

	CchLszOfId2(id, szBuf, sizeof(szBuf)/sizeof(WCHAR));
	if (!wcslen(szBuf)) return 1;
	return JetRegSetValueExW(hKey, lpValueName, 0, REG_SZ, (LPBYTE)szBuf, wcslen(szBuf)*sizeof(WCHAR));
}

static LONG WriteInt(HKEY hKey, LPCWSTR lpValueName, DWORD lpValue)
{
	return JetRegSetValueExW(hKey, lpValueName, 0, REG_DWORD, (LPBYTE)&lpValue, sizeof(DWORD));
}

static LONG WriteBool(HKEY hKey, LPCWSTR lpValueName, BOOL bValue)
{
	BYTE byte = (BYTE)bValue;
	return JetRegSetValueExW(hKey, lpValueName, 0, REG_BINARY, (LPBYTE)&byte, sizeof(BYTE));
}


int ConfigRegister(LPCWSTR lpKey, IISAM_CFG_DEF *pCfgDef, int nDefs, IISAM_CFG *pCfg, LPCWSTR lpModule)
{
	WCHAR szKey[256], szBuf[1025];
	HKEY hKey;
	IISAM_CFGPARAM *pParam;
	DWORD cbData, Type;
	LONG lret;
	int i;

	swprintf(szKey, L"%s\\Engines\\%s", lpKey, pCfgDef[0].Engine);
	if (JetRegCreateKeyW(HKEY_LOCAL_MACHINE, szKey, &hKey))
		return -1;
	if (WriteString(hKey, L"win32", lpModule))
	{
		RegCloseKey(hKey);
		return -1;
	}

	for (pParam = pCfg->next; pParam; pParam = pParam->next)
	{
		switch (pParam->cType)
		{
		case TypeString:
			if (*((WCHAR*)pParam->pDefault))
			{
				cbData = sizeof(szBuf);
				*szBuf=0;
				if ((lret = JetRegQueryValueExW(hKey, pParam->pwszName, 0, &Type, (BYTE*)szBuf, &cbData)) == ERROR_SUCCESS)
					szBuf[cbData]=0;
				else cbData=0;
				if (lret != ERROR_SUCCESS || !cbData || Type != REG_SZ)
					WriteString(hKey, pParam->pwszName, pParam->pDefault);
			}
			break;
		case TypeInt:
			cbData = sizeof(DWORD);
			if ((lret = JetRegQueryValueExW(hKey, pParam->pwszName, 0, &Type, (BYTE*)szBuf, &cbData)) || !cbData || Type != REG_DWORD)
				lret = WriteInt(hKey, pParam->pwszName, (DWORD)pParam->pDefault);
			break;
		case TypeBool:
			cbData = sizeof(BYTE);
			if ((lret = JetRegQueryValueExW(hKey, pParam->pwszName, 0, &Type, (BYTE*)szBuf, &cbData)) || !cbData || Type != REG_BINARY)
				lret = WriteBool(hKey, pParam->pwszName, (BOOL)pParam->pDefault);
			break;
		case TypeEnum:
			cbData = sizeof(szBuf);
			if ((lret = JetRegQueryValueExW(hKey, pParam->pwszName, 0, &Type, (BYTE*)szBuf, &cbData)) == ERROR_SUCCESS && cbData > 0 && Type == REG_SZ)
			{
				szBuf[cbData]=0;
				if ((pParam->apwszEnum[0] && wcsicmp(szBuf, pParam->apwszEnum[0]) == 0) ||
					(pParam->apwszEnum[1] && wcsicmp(szBuf, pParam->apwszEnum[1]) == 0) ||
					(pParam->apwszEnum[2] && wcsicmp(szBuf, pParam->apwszEnum[2]) == 0))
					break;

			}
			Type = (DWORD)pParam->pDefault;
			if (Type<=2)
				lret = WriteString(hKey, pParam->pwszName, pParam->apwszEnum[Type]);
			break;
		}
		if (lret)
		{
			RegCloseKey(hKey);
			return -1;
		}
	}
	RegCloseKey(hKey);
	hKey = NULL;

	if (nDefs <= 0) return 0;
	if ( !LoadCchLszOfId2() )
		return -1;
	for (i=0; i<nDefs; i++)
	{
		swprintf(szKey, L"%s\\ISAM Formats\\%s", lpKey, pCfgDef[i].Format);
		if (JetRegCreateKeyW(HKEY_LOCAL_MACHINE, szKey, &hKey) || WriteString(hKey, L"Engine", pCfgDef[i].Engine)) break;
		if (pCfgDef[i].DecodedExportFilter)
		{
			if (WriteDecodedString(hKey, L"ExportFilter", pCfgDef[i].DecodedExportFilter))
				break;
		}
		else if (*pCfgDef[i].ExportFilter && WriteString(hKey, L"ExportFilter", pCfgDef[i].ExportFilter)) break;
		if (pCfgDef[i].DecodedImportFilter)
		{
			if (WriteDecodedString(hKey, L"ImportFilter", pCfgDef[i].DecodedImportFilter))
				break;
		}
		else if (*pCfgDef[i].ImportFilter && WriteString(hKey, L"ImportFilter", pCfgDef[i].ImportFilter)) break;
		if (WriteBool(hKey, L"CanLink", pCfgDef[i].CanLink)) break;
		if (WriteBool(hKey, L"OneTablePerFile", pCfgDef[i].OneTablePerFile)) break;
		if (WriteInt(hKey, L"IsamType", pCfgDef[i].IsamType)) break;
		if (WriteBool(hKey, L"IndexDialog", pCfgDef[i].IndexDialog)) break;
		if (pCfgDef[i].IndexDialog && WriteDecodedString(hKey, L"IndexFilter", pCfgDef[i].IndexFilter)) break;
		if (WriteBool(hKey, L"CreateDBOnExport", pCfgDef[i].CreateDBOnExport)) break;
		if (pCfgDef[i].ResultTextImport && WriteDecodedString(hKey, L"ResultTextImport", pCfgDef[i].ResultTextImport)) break;
		if (pCfgDef[i].ResultTextLink && WriteDecodedString(hKey, L"ResultTextLink", pCfgDef[i].ResultTextLink)) break;
		if (pCfgDef[i].ResultTextExport && WriteDecodedString(hKey, L"ResultTextExport", pCfgDef[i].ResultTextExport)) break;
		if (WriteBool(hKey, L"SupportsLongNames", pCfgDef[i].SupportsLongNames)) break;
		RegCloseKey(hKey); hKey = NULL;
	}
	if (hKey) RegCloseKey(hKey);
	UnloadCchLszOfId2();
	return i==nDefs?0:-1;
}

int ConfigUnregister(LPCWSTR lpKey, IISAM_CFG_DEF *pCfgDef, int nDefs, IISAM_CFG *pCfg)
{
	WCHAR szKey[256];
	IISAM_CFGPARAM *pParam;
	LONG lret;
	DWORD cValues;
	HKEY hKey;
	int i;

	swprintf(szKey, L"%s\\Engines\\%s", lpKey, pCfgDef[0].Engine);
	if (JetRegOpenKeyW(HKEY_LOCAL_MACHINE, szKey, &hKey))
		return -1;

	for (pParam = pCfg->next; pParam; pParam = pParam->next)
		JetRegDeleteValueW(hKey, pParam->pwszName);
	JetRegDeleteValueW(hKey, L"win32");
	lret = JetRegQueryInfoKeyW(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, NULL, NULL, NULL, NULL);
	RegCloseKey(hKey);
	if (lret) return -1;
	if (!cValues)
	{
		swprintf(szKey, L"%s\\Engines", lpKey);
		if (JetRegOpenKeyW(HKEY_LOCAL_MACHINE, szKey, &hKey))
			return -1;
		JetRegDeleteKeyW(hKey, pCfgDef[0].Engine);
		RegCloseKey(hKey);
	}
	swprintf(szKey, L"%s\\ISAM Formats", lpKey);
	if (JetRegOpenKeyW(HKEY_LOCAL_MACHINE, szKey, &hKey))
		return -1;
	for (i=0; i<nDefs; i++)
		JetRegDeleteKeyW(hKey, pCfgDef[i].Format);
	RegCloseKey(hKey);
	return 0;
}
