#ifndef CS_unittesting_faux_windows_h_
#define CS_unittesting_faux_windows_h_

#include "HMODULE.h"
#include "LPCSTR.h"
#include "LPCWSTR.h"

#ifdef __cplusplus
extern "C" {
#endif

HMODULE GetModuleHandleA(LPCSTR lpModuleName);
HMODULE GetModuleHandleW(LPCWSTR lpModuleName);

#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define GetModuleHandle  GetModuleHandleW
#else
#define GetModuleHandle  GetModuleHandleA
#endif // !UNICODE


#endif