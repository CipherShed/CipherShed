#ifndef _CS_unittesting_faux_windows_UnregisterClass_h_
#define _CS_unittesting_faux_windows_UnregisterClass_h_

#include "BOOL.h"
#include "LPCSTR.h"
#include "HINSTANCE.h"
#include "LPCWSTR.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL UnregisterClassA(LPCSTR lpClassName, HINSTANCE hInstance);

BOOL UnregisterClassW(LPCWSTR lpClassName, HINSTANCE hInstance);

#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define UnregisterClass  UnregisterClassW
#else
#define UnregisterClass  UnregisterClassA
#endif // !UNICODE


#endif