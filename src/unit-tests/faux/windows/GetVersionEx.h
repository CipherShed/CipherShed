#ifndef _CS_unittesting_faux_windows_GetVersionEx_h_
#define _CS_unittesting_faux_windows_GetVersionEx_h_

#include "BOOL.h"
#include "OSVERSIONINFO.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL GetVersionExA(LPOSVERSIONINFOA lpVersionInformation);

BOOL GetVersionExW(LPOSVERSIONINFOW lpVersionInformation);

#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define GetVersionEx  GetVersionExW
#else
#define GetVersionEx  GetVersionExA
#endif // !UNICODE

#define VER_NT_WORKSTATION              0x0000001
#define VER_NT_DOMAIN_CONTROLLER        0x0000002
#define VER_NT_SERVER                   0x0000003


#endif
