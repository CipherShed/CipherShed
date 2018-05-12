#ifndef _CS_unittesting_faux_windows_GetSystemDirectory_h_
#define _CS_unittesting_faux_windows_GetSystemDirectory_h_

#include "UINT.h"
#include "LPSTR.h"
#include "LPWSTR.h"

#ifdef __cplusplus
extern "C" {
#endif

UINT GetSystemDirectoryA(LPSTR lpBuffer, UINT uSize);

UINT GetSystemDirectoryW(LPWSTR lpBuffer, UINT uSize);

#ifdef UNICODE
#define GetSystemDirectory  GetSystemDirectoryW
#else
#define GetSystemDirectory  GetSystemDirectoryA
#endif // !UNICODE

#ifdef __cplusplus
}
#endif

#endif