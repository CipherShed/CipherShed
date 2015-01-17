#ifndef _faux_windows_SetFilePointer_h_
#define _faux_windows_SetFilePointer_h_

#include "BOOL.h"
#include "HANDLE.h"
#include "DWORD.h"
#include "LARGE_INTEGER.h"

BOOL SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);

#endif