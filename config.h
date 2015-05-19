#ifndef __CONFIG_H__
#define __CONFIG_H__

struct tag_cfg;
typedef struct tag_cfg IISAM_CFG;
/* The UINTs in this struct describe a Resource ID in msjint40.dll for the string */
typedef struct
{
	LPCWSTR Format;				/* Format name */
	LPCWSTR Engine;				/* Name of engine */
	LPCWSTR ExportFilter;		/* Filter string for dialog box for file export */
	LPCWSTR ImportFilter;		/* Filter string for dialog box for file import */
	UINT DecodedExportFilter;
	UINT DecodedImportFilter;
	BOOL CanLink;				/* TRUE = JET can link to this format */
	BOOL OneTablePerFile;		/* TRUE = ISAM driver has one table per file and is required by client applications that need to propt for file name */
	DWORD IsamType;				/* See JET_isamType* constants */
	BOOL IndexDialog;			/* TRUE: Driver requires seperate dialog for index files. If yes, IndexFilter mus be provided: */
	UINT IndexFilter;			/*   Filter string for dialog box for index files */
	BOOL CreateDBOnExport;		/* TRUE: File name specified need not exist for export. */
	UINT ResultTextImport;		/* Common dialog result for importing */
	UINT ResultTextLink;		/* Common dialog result for linking */
	UINT ResultTextExport;		/* Common dialog result for exporting */
	BOOL SupportsLongNames;		/* Supports long file names */
} IISAM_CFG_DEF;

IISAM_CFG *ConfigAllocate();
void  ConfigRelease(IISAM_CFG *pCfg);

/* ConfigAddStrParameter
 *
 * Adds a string parameter to be used.
 *
 * Parameters:  pCfg         - The config object
 *              pwszName     - The name of the parameter in the registry
 *              pwszParamOut - The WCHAR* that receives the parsed string from 
 *                             the registry.
 *              pwszDefault  - The default value to be used if nothing is found
 *                             in the registry.
 * Returns: 0 on success, -1 on error (OOM)
 */
int ConfigAddStrParameter(IISAM_CFG *pCfg, WCHAR *pwszName, WCHAR *pwszParamOut, WCHAR *pwszDefault);

/* ConfigAddIntParameter
 *
 * Adds an integer parameter to be used.
 *
 * Parameters:  pCfg         - The config object
 *              pwszName     - The name of the parameter in the registry
 *              pdwParamOut  - The DWORD* that receives the parsed DWORD from 
 *                             the registry.
 *              dwDefault    - The default value to be used if nothing is found
 *                             in the registry.
 * Returns: 0 on success, -1 on error (OOM)
 */
int ConfigAddIntParameter(IISAM_CFG *pCfg, WCHAR *pwszName, DWORD *pdwParamOut, DWORD dwDefault);

/* ConfigAddBoolParameter
 *
 * Adds an boolean parameter to be used.
 *
 * Parameters:  pCfg         - The config object
 *              pwszName     - The name of the parameter in the registry
 *              pbParamOut   - The BOOL* that receives the parsed BOOL from 
 *                             the registry.
 *              dwDefault    - The default value to be used if nothing is found
 *                             in the registry.
 * Returns: 0 on success, -1 on error (OOM)
 */
int ConfigAddBoolParameter(IISAM_CFG *pCfg, WCHAR *pwszName, BOOL *pbParamOut, BOOL bDefault);

/* ConfigAddEnumParameter
 *
 * Adds an enumeration parameter to be used (3 values are possible).
 *
 * Parameters:  pCfg         - The config object
 *              pwszName     - The name of the parameter in the registry
 *              pdwParamOut  - The DWORD* that receives the number of the selected
 *                             value (0,1 or 2) from the registry.
 *              pEnum1       - Name of the first value of the enumeration
 *              pEnum2       - Name of the second value of the enumeration
 *              pEnum3       - Name of the third value of the enumeration
 *              dwDefault    - The default value to be used if nothing is found
 *                             in the registry (0, 1 or 2).
 * Returns: 0 on success, -1 on error (OOM)
 */
int ConfigAddEnumParameter(IISAM_CFG *pCfg, WCHAR *pwszName, DWORD  *pdwParamOut, 
						   WCHAR *pEnum1, WCHAR *pEnum2, WCHAR *pEnum3, DWORD dwDefault);


BOOL ConfigReadRegSpec(WCHAR *pwszDriver, WCHAR *pwszRegKey, HKEY hKey, WCHAR *pwszRegPath, IISAM_CFG *pCfg);
int ConfigRegister(LPCWSTR lpKey, IISAM_CFG_DEF *pCfgDef, int nDefs, IISAM_CFG *pCfg, LPCWSTR lpModule);
int ConfigUnregister(LPCWSTR lpKey, IISAM_CFG_DEF *pCfgDef, int nDefs, IISAM_CFG *pCfg);

#endif