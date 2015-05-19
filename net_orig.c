#if 0
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "dos.h"
#include "jetuwrap.h"

/* These are part of the original functions used in Net-Library in ODBC IISAM drivers.
 * hewever as we already have InternetCrackURL, in my optinion, it doesn't make
 * sense to rewrite all this code, therefore it's just archived here for reference
 * but not actively used...
 * InternetCrackUrlA is also there in Win95, so I'm not sure why they did it that way.
 */

static struct
{
	LPCWSTR lpProto;
	DWORD dwLen;
} m_aProtos[] = {
	{L"http:",     5},
	{L"ftp:",      4},
	{L"gopher:",   7},
	{L"wais:",     5},
	{L"file:",     5},
	{L"https:",    6},
	{L"mailto:",   7},
	{L"news:",     5},
	{L"msn:",      4},
	{L"nntp:",     5},
	{L"mid:",      4},
	{L"cid:",      4},
	{L"prospero:", 9},
	{L"telnet:",   7},
	{L"rlogin:",   7},
	{L"tn3270:",   7}
};
enum {
	Proto_HTTP = 0,
	Proto_FTP,
	Proto_Gopher,
	Proto_Wais,
	Proto_File,
	Proto_HTTPS,
	Proto_MailTo,
	Proto_News,
	Proto_MSN,
	Proto_NNTP,
	Proto_MID,
	Proto_Cid,
	Proto_Prospero,
	Proto_Telnet,
	Proto_Rlogin,
	Proto_Tn3270
};

int ProtocolPrefix(LPWSTR lpURL)
{
	int i;

	for (i=0; i<sizeof(m_aProtos)/sizeof(m_aProtos[0]) && wcsnicmp(m_aProtos[i].lpProto, lpURL, m_aProtos[i].dwLen); i++);
	if (i==sizeof(m_aProtos)/sizeof(m_aProtos[0])) return -1;
	return i;
}

JET_ERR NetDirectoryExists(LPCWSTR lpDirectory, JET_ERR *pRet)
{
	int pfx;
	WCHAR wszDirectory[MAX_PATH], *pw;

	*pRet = JET_errSuccess;
	if ((pfx = ProtocolPrefix(lpDirectory)) != Proto_FTP && pfx != Proto_HTTP)
		return JET_errObjectNotFound;
	if (!m_hInternet && !NetInitializeInternetServices())
		return -20163;
	wcscpy(wszDirectory, &lpDirectory[m_aProtos[pfx].dwLen+1]);
	for (pw=wszDirectory; *pw; pw++)
		if (*pw==L'/') *pw=L'\\';
	for (pw=wszDirectory; *pw && *pw!=L'\\'; pw++);
	if (*pw) {*pw=0; pw++;}
	// ...
}

#endif