#ifndef CS_unittesting_faux_windows_getprocaddress_h_
#define CS_unittesting_faux_windows_getprocaddress_h_

#include "NEAR.h"
#include "FAR.h"
#include "HINSTANCE.h"
#include "LPCSTR.h"

typedef int (FAR *FARPROC)();
typedef int (NEAR *NEARPROC)();
typedef int (*PROC)();

#ifdef __cplusplus
extern "C" {
#endif

FARPROC GetProcAddress (HMODULE hModule, LPCSTR lpProcName);

#ifdef __cplusplus
}
#endif

#endif
