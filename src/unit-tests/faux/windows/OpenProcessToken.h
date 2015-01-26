#ifndef _CS_unittesting_faux_windows_OpenProcessToken_h_
#define _CS_unittesting_faux_windows_OpenProcessToken_h_

#include "BOOL.h"
#include "HANDLE.h"
#include "DWORD.h"
#include "PHANDLE.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL OpenProcessToken (HANDLE ProcessHandle, DWORD DesiredAccess, PHANDLE TokenHandle);

#ifdef __cplusplus
}
#endif

#define TOKEN_QUERY             (0x0008)


#endif 