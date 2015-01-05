#ifndef _faux_windows_EnableWindow_h_
#define _faux_windows_EnableWindow_h_

#include "HWND.h"
#include "BOOL.h"

typedef struct fauxEnablableWidget
{
	BOOL enabled;
} fauxEnablableWidget;

#ifdef __cplusplus
extern "C" {
#endif

BOOL EnableWindow(HWND hWnd, BOOL bEnable);

#ifdef __cplusplus
}
#endif

#endif