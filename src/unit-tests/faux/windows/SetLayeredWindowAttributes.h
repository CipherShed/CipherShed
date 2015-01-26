#ifndef _CS_unittesting_faux_windows_SetLayeredWindowAttributes_h_
#define _CS_unittesting_faux_windows_SetLayeredWindowAttributes_h_

#include "BOOL.h"
#include "HWND.h"
#include "COLORREF.h"
#include "BYTE.h"
#include "DWORD.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL SetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002



#endif