#ifndef _faux_windows_SetFileTime_h_
#define _faux_windows_SetFileTime_h_

#include "BOOL.h"
#include "HANDLE.h"
#include "CONST.h"
#include "FILETIME.h"


BOOL SetFileTime(HANDLE hFile, CONST FILETIME *lpCreationTime, CONST FILETIME *lpLastAccessTime, CONST FILETIME *lpLastWriteTime);

#endif