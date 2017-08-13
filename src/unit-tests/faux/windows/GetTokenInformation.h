#ifndef _CS_unittesting_faux_windows_GetTokenInformation_h_
#define _CS_unittesting_faux_windows_GetTokenInformation_h_

#include "BOOL.h"
#include "HANDLE.h"
#include "TOKEN_INFORMATION_CLASS.h"
#include "LPVOID.h"
#include "DWORD.h"
#include "PDWORD.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL GetTokenInformation (HANDLE TokenHandle, TOKEN_INFORMATION_CLASS TokenInformationClass, LPVOID TokenInformation, DWORD TokenInformationLength, PDWORD ReturnLength);

#ifdef __cplusplus
}
#endif

#endif