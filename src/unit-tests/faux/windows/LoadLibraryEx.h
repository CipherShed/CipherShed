#ifndef _CS_unittesting_faux_windows_LoadLibraryEx_h_
#define _CS_unittesting_faux_windows_LoadLibraryEx_h_

#include "HMODULE.h"
#include "LPCSTR.h"
#include "HANDLE.h"
#include "DWORD.h"
#include "LPCWSTR.h"

#ifdef __cplusplus
extern "C" {
#endif

HMODULE LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

HMODULE LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

#ifdef UNICODE
#define LoadLibraryEx  LoadLibraryExW
#else
#define LoadLibraryEx  LoadLibraryExA
#endif // !UNICODE

#ifdef __cplusplus
}
#endif

#endif
