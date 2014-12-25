#ifndef _faux_windows_GetFileSize_h_
#define _faux_windows_GetFileSize_h_

#include "HANDLE.h"
#include "LARGE_INTEGER.h"

BOOL GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize);

#endif