#ifndef _CS_unittesting_faux_windows_QueryDosDevice_h_
#define _CS_unittesting_faux_windows_QueryDosDevice_h_

#include "DWORD.h"
#include "LPCSTR.h"
#include "LPSTR.h"
#include "LPCWSTR.h"
#include "LPWSTR.h"

DWORD QueryDosDeviceA(LPCSTR lpDeviceName, LPSTR lpTargetPath, DWORD ucchMax);

DWORD QueryDosDeviceW(LPCWSTR lpDeviceName, LPWSTR lpTargetPath, DWORD ucchMax);

#ifdef UNICODE
#define QueryDosDevice  QueryDosDeviceW
#else
#define QueryDosDevice  QueryDosDeviceA
#endif // !UNICODE


#endif