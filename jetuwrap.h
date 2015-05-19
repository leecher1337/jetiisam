void IsUnicodeOS();

/**************************************************
 * Jet registry functions                         *
 **************************************************/
LONG JetRegOpenKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
LONG JetRegCreateKeyW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
LONG JetRegDeleteKeyW(HKEY hKey, LPCWSTR lpSubKey);
LONG JetRegDeleteValueW(HKEY hKey, LPCWSTR lpValueName);
LONG JetRegQueryInfoKeyW(HKEY hKey, LPWSTR lpClass, LPDWORD lpcbClass, LPDWORD lpReserved, LPDWORD lpcSubKeys,
						 LPDWORD lpcbMaxSubKeyLen, LPDWORD lpcbMaxClassLen, LPDWORD lpcValues, 
						 LPDWORD lpcbMaxValueNameLen, LPDWORD lpcbMaxValueLen, LPDWORD lpcbSecurityDescriptor, 
						 PFILETIME lpftLastWriteTime);
LONG JetRegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType,
						LPBYTE lpData, LPDWORD lpcbData);
LONG JetRegSetValueExW(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType,
					   LPBYTE lpData, DWORD cbData);

/**************************************************
 * Jet filesystem functions                       *
 **************************************************/
BOOL JetSetCurrentDirectoryW(LPCWSTR lpPathName);
HANDLE JetCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
					  LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
					  DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
BOOL JetCreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
BOOL JetDeleteFileW(LPCWSTR lpFileName);
HANDLE JetFindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
BOOL JetFindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);
DWORD JetGetCurrentDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer);
DWORD JetGetFileAttributesW(LPCWSTR lpFileName);
DWORD JetGetTempPathW(DWORD nBufferLength, LPWSTR lpBuffer);
BOOL JetMoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
DWORD JetRemoveDirectoryW(LPCWSTR lpPathName);
WCHAR *Jetwfullpath(LPWSTR absPath, LPCWSTR relPath, int maxLength);

/**************************************************
 * Jet character class functions                  *
 **************************************************/
LPWSTR JetCharUpperW(LPWSTR lpwsz);

/**************************************************
 * Jet base system functions                      *
 **************************************************/
HMODULE JetLoadLibraryW(LPCWSTR lpLibFileName);
