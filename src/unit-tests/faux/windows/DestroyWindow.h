#ifndef _CS_unittesting_faux_windows_DestroyWindow_h_
#define _CS_unittesting_faux_windows_DestroyWindow_h_

#include "BOOL.h"
#include "HWND.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL DestroyWindow(HWND hWnd);

#ifdef __cplusplus
}
#endif

#endif