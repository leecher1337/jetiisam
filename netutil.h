#include <wininet.h>

enum eProtoType
{
	Proto_Other = 0,
	Proto_HTTP  = 1,
	Proto_FTP   = 2
};

typedef struct 
{
	HINTERNET hInternet;
	HINTERNET hFind;
} NET_FIND_HANDLE;

/* Missing error codes from esent98.h */
/* I don't know their original name, so I made up some names */
#define JET_errNetTimeout          -20157
#define JET_errNetResolveHost      -20158
#define JET_errNetLoginFailed      -20159
#define JET_errNetInvalidProtocol  -20160
#define JET_errNetStatusUnknown    -20161
#define JET_errNetInitialize       -20163


/* Some errorcodes for netutil, I made the names up as there is no official documentations for them */

BOOL NetInitializeInternetServices();
BOOL NetTerminateInternetServices();
LPWSTR NetCreateLocalDirectory(LPWSTR lpDirectory);
short NetDestroyLocalDirectory(LPCWSTR lpDirectory);
JET_ERR NetDirectoryExists(LPCWSTR lpDirectory, JET_ERR *pRet);
JET_ERR NetDownloadToLocal(LPCWSTR lpURL, LPCWSTR *alpWildcards, LPCWSTR lpDownloadPath, LPWSTR lpLocalFile);
JET_ERR NetFileExists(LPCWSTR lpURL);
JET_ERR NetFileInfo(LPCWSTR lpURL, WIN32_FIND_DATAW *pInfo);
JET_ERR NetFindFirstMatchingFile(LPCWSTR lpURL, LPWSTR lpFileName, NET_FIND_HANDLE *pConn, JET_ERR *pRet);
JET_ERR NetFindNextMatchingFile(LPWSTR lpFileName, NET_FIND_HANDLE *pConn, JET_ERR *pRet);
enum eProtoType NetProtocolType(LPCWSTR lpURL);
JET_ERR NetLocalCleanup(LPCWSTR lpFile, LPCWSTR *alpWildcards);
JET_ERR NetDeleteFile(LPCWSTR lpFile);
JET_ERR NetUploadToNet(LPCWSTR lpLocalFile, LPCWSTR *alpWildcards, LPCWSTR lpURL);


