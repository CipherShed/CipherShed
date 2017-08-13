#ifndef _faux_windows_VOID_h_
#define _faux_windows_VOID_h_

#include "FAR.h"
#include "CONST.h"

#ifndef VOID
#define VOID void
#endif

typedef void *PVOID;
typedef void far            *LPVOID;
typedef CONST void far      *LPCVOID;

#endif