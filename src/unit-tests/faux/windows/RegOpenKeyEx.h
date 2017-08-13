#ifndef CS_unittests_faux_windows_RegOpenKeyEx_h_
#define CS_unittests_faux_windows_RegOpenKeyEx_h_

#include "HKEY.h"
#include "CHAR.h"
#include "DWORD.h"
#include "WCHAR.h"
#include "BYTE.h"

#ifdef __cplusplus
extern "C" {
#endif


LSTATUS RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);

LSTATUS RegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);

#ifdef UNICODE
#define RegOpenKeyEx  RegOpenKeyExW
#else
#define RegOpenKeyEx  RegOpenKeyExA
#endif // !UNICODE

LSTATUS RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

LSTATUS RegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

#ifdef UNICODE
#define RegQueryValueEx  RegQueryValueExW
#else
#define RegQueryValueEx  RegQueryValueExA
#endif // !UNICODE

LSTATUS RegCloseKey(HKEY hKey);

#ifdef __cplusplus
}
#endif


#endif
