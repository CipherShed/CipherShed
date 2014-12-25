#ifndef _faux_windows_LARGE_INTEGER_h_
#define _faux_windows_LARGE_INTEGER_h_

#include "DWORD.h"
#include "LONG.h"

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef LARGE_INTEGER *PLARGE_INTEGER;

#endif