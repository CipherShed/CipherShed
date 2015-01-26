#ifndef _CS_unittesting_faux_windows_ShowWindow_h_
#define _CS_unittesting_faux_windows_ShowWindow_h_

#include "BOOL.h"
#include "HWND.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL ShowWindow(HWND hWnd, int nCmdShow);

#ifdef __cplusplus
}
#endif

#define SW_SHOWNORMAL       1


#endif
