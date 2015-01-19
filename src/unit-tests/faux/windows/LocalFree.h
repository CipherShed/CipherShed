#ifndef CS_unittests_faux_windows_localfree_h_
#define CS_unittests_faux_windows_localfree_h_


#include "HANDLE.h"

#ifdef __cplusplus
extern "C"
{
#endif

HLOCAL LocalFree(HLOCAL hMem);

#ifdef __cplusplus
}
#endif

#endif
