#ifndef _faux_windows_GetLastError_h_
#define _faux_windows_GetLastError_h_

#include "DWORD.h"
#include "VOID.h"

#ifdef __cplusplus
extern "C" {
#endif

DWORD GetLastError(VOID);

#ifdef __cplusplus
}
#endif

#endif