#ifndef __ISAMUTIL_H__
#define __ISAMUTIL_H__

#include "spec.h"
#include <time.h>

struct tag_ISAM_Task;
typedef struct tag_ISAM_Task ISAM_Task;
struct tag_ISAM_Session;
typedef struct tag_ISAM_Session ISAM_Session;
struct tag_ISAM_Database;
typedef struct tag_ISAM_Database ISAM_DB;
struct tag_ISAM_DBUser;
typedef struct tag_ISAM_DBUser ISAM_DBUser;
struct tag_ISAM_DBTable;
typedef struct tag_ISAM_DBTable ISAM_DBTable;
struct tag_ISAM_Cursor;
typedef struct tag_ISAM_Cursor ISAM_Cursor;
struct tag_ISAM_NetFile;
typedef struct tag_ISAM_NetFile ISAM_NetFile;
struct tag_ISAM_VTDef;
typedef struct tag_ISAM_VTDef ISAM_VTDef;
struct tag_ISAM_Column;
typedef struct tag_ISAM_Column ISAM_Column;
struct tag_ISAM_OBJListData;
typedef struct tag_ISAM_OBJListData ISAM_OBJListData;
struct tag_ISAM_ColList;
typedef struct tag_ISAM_ColList ISAM_ColList;
struct tag_ISAM_IDXListData;
typedef struct tag_ISAM_IDXListData ISAM_IDXListData;
struct tag_ISAM_COLListData;
typedef struct tag_ISAM_COLListData ISAM_COLListData;
struct tag_ISAM_Trace;
typedef struct tag_ISAM_Trace ISAM_Trace;

struct tag_ISAM_Task // 0x24 (=36) bytes
{
  struct tag_ISAM_Task *next;	// +0
  ISAM_Session *pSession;		// +4
  ISAM_DB *pDatabase;			// +8
  DWORD dwRefCount;				// +12
  DWORD dwTaskHandle;			// +16
  DWORD dwUsers;				// +20
  DWORD dwObjects;				// +24
  JET_CALLIN *pCaller;			// +28
  JET_INSTANCE JetInst;			// +32
};

struct tag_ISAM_Session // 20 Bytes
{
	struct tag_ISAM_Session *next;	// +0
	ISAM_Task *pTask;				// +4
	ISAM_VTDef *pVTDef;				// +8
	JET_SESID nSession;				// +12
	JET_SESID sesid;				// +16
	ISAM_Trace *pTrace;				// +20 (custom member by us, not part of original)
};

struct tag_ISAM_Database
{
	struct tag_ISAM_Database *next; // +0
	ISAM_Task *pTask;				// +4
	DWORD dwUnk1;					// +8	1 = Virtual memory Database?
	ISAM_NetFile *pNetFiles;		// +20
	JET_GRBIT flags;				// +24
	ISAM_DBUser *pUsers;			// +48
	ISAM_DBTable *pTables;			// +52
	WCHAR szLocalDir[MAX_PATH];		// +56 Download directory for Netfiles
	WCHAR szDBName[1];				// +578
};

struct tag_ISAM_DBUser
{
	struct tag_ISAM_DBUser *next;	// +0
	ISAM_DB *pDB;					// +4
	ISAM_Session *pSession;			// +8
	IISAM_SETTINGS *settings;		// +12 (Normally the struct, not a pointer, but for modular architecture I changed it)
	JET_HANDLE userid;				// +884
	JET_DBID dbid;					// +888
	JET_GRBIT flags;				// +892
	DWORD dwUnk1;					// +904  (-> dwUnk1 from DB)
	WCHAR szConnectStr[1];			// +1026
};

struct tag_ISAM_DBTable
{
	struct tag_ISAM_DBTable *next;	// +0
	ISAM_DB *pDB;					// +4
	PVOID pDrvInst;					// +8   Driver instance of your real driver
	JET_DATESERIAL dtCreate;		// +856
	DWORD dwFileSize;				// +864
	ISAM_Cursor *pCursors;			// +868
	ISAM_Cursor *pCurrCursor;		// +872
	BOOL bReadonly;					// +876
	//BOOL bCreated;				// +880
	DWORD dwUnk1;					// +904 (-> dwUnk1 from DB)
	DWORD nColumns;					// +912
	ISAM_Column *pColumns;			// +916
	WCHAR szTableName[256];			// +920
};

struct tag_ISAM_Column
{
	struct tag_ISAM_Column *next;	// +0
	ISAM_DBTable *pTable;			// +4
	JET_GRBIT grbit;				// +8
	JET_HANDLE columnid;			// +12
	JET_COLTYP coltyp;				// +16
	ULONG cbMax;					// +20
	ULONG wCountry;					// +28
	ULONG langid;					// +32
	WCHAR szColumnName[32];			// +36
};

typedef DWORD rowid_t;	// Set to a real structure that identifies a row

struct tag_ISAM_Cursor
{
	struct tag_ISAM_Cursor *next;	// +0
	ISAM_DBTable *pTable;			// +4
	ISAM_DBUser *pUser;				// +8
	JET_HANDLE cursorid;			// +12
	JET_TABLEID externalTID;		// +16
	JET_GRBIT flags;				// +20
	BOOL bClass8;					// +24
	BYTE bReadonly;					// +28
	BYTE currency;					// +29 CURRENCY_* for current cursor position
	rowid_t rowid;					// +32 Real text driver: 32byte field for current record position as bookmark ID
	ULONG rownum;					// +64 Current row numer
	BOOL bPrepUpdate;				// +76 Cursor prepared to update
	/* Now our custom stuff that is not in the text driver: */
	PVOID BindList;					// +80 Custom attribute for binding columns on update, if you need it
	ULONG curIdx;					// +84 Custom attribute for currently selected Index number, if supported
	PVOID SearchKey;				// +88 Custom attribute for the dataset of IsamMakeKey()
};

struct tag_ISAM_NetFile
{
	struct tag_ISAM_NetFile *next;	// +0
	BOOL bUploadOnExit;				// +4
	LPWSTR pszRemoteFile;			// +8
	LPWSTR pszLocalFile;			// +12
};

/* This is just a list of string, used to enumerate tables */
struct tag_ISAM_ColList
{
	ISAM_ColList *next;				// +0
	WCHAR colname[256];				// +4
};

/* This seems to be the original structur in Text Jet driver. 
 * However it's far more convenient to just use JET_OBJECTINFO
 * within the struct? I don't know why this wasn't done in the
 * original driver 
struct tag_ISAM_OBJListData
{
	ISAM_OBJListData *next;			// +0
	JET_OBJTYP objtyp;				// +4
	JET_DATESERIAL dtCreate;		// +8
	JET_DATESERIAL dtUpdate;		// +16
	ULONG cRecord;					// +24
	ULONG cPage;					// +28
	JET_GRBIT grbit;				// +32
	ULONG flags;					// +36
	WCHAR szObjectName[256];		// +40
};
*/

struct tag_ISAM_OBJListData
{
	ISAM_OBJListData *next;
	JET_OBJECTINFO objInfo;
	WCHAR szObjectName[256];
};

struct tag_ISAM_IDXListData
{
	ISAM_IDXListData *next;
	JET_GRBIT grbit;
	ULONG cKey;
	ULONG cEntry;
	ULONG cPage;
	ULONG cColumn;
	ULONG iColumn;
	JET_COLUMNID columnid;
	JET_COLTYP coltyp;
	LCID Langid;
	JET_GRBIT grbitCol;
	WCHAR szIdxName[65];
	WCHAR szColName[65];
};

struct tag_ISAM_COLListData
{
	ISAM_COLListData *next;
	JET_COLUMNBASE_OLD colDef;
	long presidx;
};

#define VTDEF_TYPE_INDEXINFO	0
#define VTDEF_TYPE_COLUMNINFO	1
#define VTDEF_TYPE_OBJINFO		2

#define CURRENCY_BEGIN		0	/* Beginning has been reached */
#define CURRENCY_CURRENT	2	/* You may seek */
#define CURRENCY_END		1	/* Last record has been reached */
#define CURRENCY_LOST		4	/* Currency has been lost */

/* Virtual Tables are implemented very inefficiently in Text driver.
 * They are all linked list of datasets, therefore seeking back or
 * to a random dataset is very costly. You may want to reimplement
 * them with a memory pointer list, but as the goal of this project
 * is to document the original driver, I've kept it that way, so that
 * you can better understand the code of the original driver. */
typedef struct {
	PVOID pData;					// +560 Pointer to table data linked list
	PVOID pCurData;					// +564 Pointer to current dataset 
} ISAM_DATAPTR;

struct tag_ISAM_VTDef
{
	struct tag_ISAM_VTDef *next;	// +0
	ISAM_Session *pSession;			// +4
	DWORD type;						// +8   Type of VTdef
	JET_HANDLE vtdid;				// +12
	JET_TABLEID externalTID;		// +16
	JET_HANDLE userid;				// +20
	JET_DBID dbid;					// +24
	ULONG InfoLevel;				// +28
	ULONG cRecord;					// +32  Number of rows in this virtual table
	BYTE currency;					// +36  Seek position within data (CURRENCY_*)
	ISAM_DATAPTR data;				// +560 Pointer to table data linked list
									// +564 Pointer to current dataset 
};

/* This structure is not part of the orignal TXT driver, but our own
 * to enable function call tracing */
struct tag_ISAM_Trace
{
	struct tag_ISAM_Trace *next;	// +0
	ISAM_Session *pSession;			// +4
	JET_HANDLE traceid;				// +8
	JET_HANDLE tableid;				// +12
	JET_HANDLE handle;				// +16
	BYTE type;						// +20
	union							// +21
	{
		JET_FNDEFTBL *pTblFN;
		JET_FNDEFDB *pDBFN;
	};
};

typedef struct _tagDATETIME {
    WORD    nYear;
    WORD    nMonth;
    WORD    nDay;
    WORD    nHour;
    WORD    nMinute;
    WORD    nSecond;
} DATETIME;

DWORD CurrentTaskHandle();
JET_SESID ErrorSetSession(JET_SESID sesid);

void ISAMDBInitialize();
ISAM_Task *ISAMDBTaskList();
ISAM_Task *ISAMDBAddTask(DWORD dwTaskHandle);
ISAM_Task *ISAMDBFindTask(DWORD dwTaskHandle);
ISAM_Cursor *ISAMDBFindCursor(JET_SESID sesid, JET_HANDLE cursorid);
ISAM_Cursor *ISAMDBFindCursorFromExternalTID(JET_SESID sesid, JET_TABLEID externalTID);
ISAM_DB *ISAMDBFindDatabase(ISAM_Task *pTask, LPCWSTR lpDBName);
ISAM_DBUser *ISAMDBFindDatabaseUser(JET_SESID sesid, JET_HANDLE userid);
ISAM_NetFile *ISAMDBFindNetFile(ISAM_DB *pDB, LPCWSTR lpRemoteFile);
ISAM_Session *ISAMDBFindSession(JET_SESID sesid);
void ISAMDBDeleteTask(ISAM_Task *pTask);
JET_ERR ISAMDBDeleteCursor(ISAM_DBTable *pTable, ISAM_Cursor *pCursor);
void ISAMDBDeleteDatabase(ISAM_Task *pTask, ISAM_DB *pDatabase);
void ISAMDBDeleteDatabaseUser(ISAM_DB *pDB, ISAM_DBUser *pUser);
void ISAMDBDeleteSession(ISAM_Task *pTask, ISAM_Session *pSession);
JET_ERR ISAMDBDeleteTable(ISAM_DB *pDB, ISAM_DBTable *pTable);
void ISAMDBDeleteTask(ISAM_Task *pTask);
JET_ERR ISAMDBDeleteVTDef(ISAM_Session *pSession, ISAM_VTDef *pVTDef);
JET_ERR ISAMDBDeleteColumn(ISAM_DBTable *pTable, LPCWSTR pszColumn);
JET_ERR ISAMDBDeleteColumns(ISAM_DBTable *pTable);
JET_CALLIN *ISAMCurrentTaskCallbacks();
ISAM_DB *ISAMDBAddDatabase(ISAM_Task *pTask, WCHAR *pwszDBName);
ISAM_DBUser *ISAMDBAddDatabaseUser(ISAM_Session *pSession, ISAM_DB *pDB, LPCWSTR pwszConnect);
ISAM_DBTable *ISAMDBAddTable(ISAM_DB *pDB, LPCWSTR lpTableName);
ISAM_Cursor *ISAMDBAddCursor(ISAM_DBUser *pUser, ISAM_DBTable *pTable);
ISAM_NetFile *ISAMDBAddNetFile(ISAM_DB *pDB, LPCWSTR lpRemote, LPCWSTR lpLocal, BOOL bUnk);
ISAM_VTDef *ISAMDBAddVTDef(ISAM_Session *pSession, DWORD dwUnk);
ISAM_Session *ISAMDBAddSession(ISAM_Task *pTask, JET_SESID sesid);
ISAM_Column *ISAMDBLocateColumn(ISAM_DBTable *pTable, LPCWSTR lpColumn);
ISAM_Column *ISAMDBLocateColumnId(ISAM_DBTable *pTable, JET_HANDLE Id);
ISAM_DBTable *ISAMDBLocateTable(ISAM_DB *pDB, LPCWSTR lpTable);
ISAM_VTDef *ISAMDBLocateVTDef(ISAM_Session *pSession, JET_HANDLE vtdid);
ISAM_VTDef *ISAMDBLocateVTDefExtTID(ISAM_Session *pSession, JET_TABLEID externalTID);

ISAM_Trace *ISAMDBFindTrace(JET_SESID sesid, JET_HANDLE traceid);
ISAM_Trace *ISAMDBFindTraceByTID(BYTE type, JET_HANDLE tableid);
JET_TABLEID ISAMDBFindTraceTableidFromVtid(JET_TABLEID vtid, JET_FNDEFTBL *pfndef);
ISAM_DBUser *ISAMDBFindDatabaseUserId(JET_HANDLE userid);
ISAM_Trace *ISAMDBAddTrace(ISAM_Session *pSession, BYTE type, JET_HANDLE handle, PVOID pvFnDef);
void ISAMDBDeleteTrace(ISAM_Session *pSession, ISAM_Trace *pTrace);

LPCWSTR ErrorSetExtendedInfoSz1(JET_ERR error, LPCWSTR lpError, JET_GRBIT grbit);
LPCWSTR ErrorSetExtendedInfoSz2(JET_ERR error, LPCWSTR lpError, JET_GRBIT grbit);

int MakeValidJetName(LPCWSTR lpOldName, LPWSTR lpNewName, UINT CodePage);
BOOL IsValidJETName(LPCWSTR lpName, UINT CodePage);
int MakeIntoValidFilename(LPCWSTR lpOldName, LPWSTR lpNewName, 
						  int cbFName, int cbExt, BOOL bHasExt, UINT CodePage);
int DBlstrcmpi(LPCWSTR lpString1, LPCWSTR lpString2);
JET_ERR AssignResult(PVOID pSrc, ULONG cbSrc, PVOID pDest, ULONG cbDest);
BYTE SetCursorCurrency(ISAM_Cursor *pCursor, BYTE currency);
JET_ERR EstablishAsCurrentCursor(ISAM_Cursor *pCursor);

void DtfParsedToSerial(DATETIME *dt, double *dtDest);
BOOL TmFromOleDate(double dtSrc, struct tm *tmDest);

PVOID SortList(PVOID arg_list, int (__cdecl *cmp)(const void *elem1, const void *elem2));

#endif
