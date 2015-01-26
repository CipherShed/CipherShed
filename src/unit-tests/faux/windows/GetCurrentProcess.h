#ifndef CS_unittesting_faux_windows_GetCurrentProcess_h_
#define CS_unittesting_faux_windows_GetCurrentProcess_h_

#include "HANDLE.h"
#include "VOID.h"

#ifdef __cplusplus
extern "C" {
#endif

HANDLE GetCurrentProcess(VOID);

#ifdef __cplusplus
}
#endif

#endif