#ifndef _faux_windows_CloseHandle_h_
#define _faux_windows_CloseHandle_h_

#include "BOOL.h"
#include "HANDLE.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL CloseHandle(HANDLE hObject);

#ifdef __cplusplus
}
#endif

#endif