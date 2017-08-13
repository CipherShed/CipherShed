#ifndef _faux_windows_FlushFileBuffers_h_
#define _faux_windows_FlushFileBuffers_h_

#include "BOOL.h"
#include "HANDLE.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL FlushFileBuffers(HANDLE hFile);

#ifdef __cplusplus
}
#endif

#endif