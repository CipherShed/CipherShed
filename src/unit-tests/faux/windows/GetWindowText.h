#ifndef _faux_windows_GetWindowText_h_
#define _faux_windows_GetWindowText_h_

#include "HWND.h"
#include "CHAR.h"
#include "WCHAR.h"

#ifdef __cplusplus
extern "C" {
#endif

int GetWindowText(HWND hWnd, LPSTR lpString, int nMaxCount);
int GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount);
int GetWindowTextW(HWND hWnd, LPWSTR lpString, int nMaxCount);

#ifdef __cplusplus
}
#endif

#endif