#ifndef _faux_windows_GetFileTime_h_
#define _faux_windows_GetFileTime_h_

#include "BOOL.h"
#include "HANDLE.h"
#include "FILETIME.h"

BOOL GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime );

#endif