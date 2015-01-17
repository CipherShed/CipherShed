#ifndef _faux_windows_filetime_h_
#define _faux_windows_filetime_h_

#include "DWORD.h"
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;
#define _FILETIME_

#endif