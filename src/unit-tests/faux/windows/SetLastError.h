#ifndef _faux_windows_SetLastError_h_
#define _faux_windows_SetLastError_h_

#include "DWORD.h"
#include "VOID.h"

#ifdef __cplusplus
extern "C" {
#endif

VOID SetLastError(DWORD dwErrCode);

#ifdef __cplusplus
}
#endif

#endif