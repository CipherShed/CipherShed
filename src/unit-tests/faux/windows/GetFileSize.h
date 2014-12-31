#ifndef _faux_windows_GetFileSize_h_
#define _faux_windows_GetFileSize_h_

#include "HANDLE.h"
#include "LARGE_INTEGER.h"
#include "BOOL.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);

#ifdef __cplusplus
}
#endif

#endif