#ifndef _CS_unittesting_faux_windows_DefWindowProc_h_
#define _CS_unittesting_faux_windows_DefWindowProc_h_

#include "LRESULT.h"
#include "HWND.h"
#include "UINT.h"
#include "WPARAM.h"
#include "LPARAM.h"

//see: https://www.winehq.org/pipermail/wine-patches/2006-October/031645.html

#ifdef __cplusplus
extern "C" {
#endif

LRESULT DefWindowProcA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

LRESULT DefWindowProcW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define DefWindowProc  DefWindowProcW
#else
#define DefWindowProc  DefWindowProcA
#endif // !UNICODE


#endif
