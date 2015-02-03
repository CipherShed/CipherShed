#ifndef _CS_unittesting_faux_windows_OutputDebugString_h_
#define _CS_unittesting_faux_windows_OutputDebugString_h_

#include "VOID.h"
#include "LPCSTR.h"
#include "LPCWSTR.h"

VOID OutputDebugStringA(LPCSTR lpOutputString);

VOID OutputDebugStringW(LPCWSTR lpOutputString);

#ifdef UNICODE
#define OutputDebugString  OutputDebugStringW
#else
#define OutputDebugString  OutputDebugStringA
#endif // !UNICODE


#endif
