#ifndef _faux_windows_handle_h_
#define _faux_windows_handle_h_

#include "FAR.h"
#include "NEAR.h"

typedef void *HANDLE;
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

typedef HANDLE NEAR         *SPHANDLE;
typedef HANDLE FAR          *LPHANDLE;
typedef HANDLE              HGLOBAL;
typedef HANDLE              HLOCAL;
typedef HANDLE              GLOBALHANDLE;
typedef HANDLE              LOCALHANDLE;


#endif