#ifndef _faux_windows_GetWindowText_h_
#define _faux_windows_GetWindowText_h_

#include "HWND.h"
#include "CHAR.h"
#include "WCHAR.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef UNICODE
#define GetWindowText  GetWindowTextW
#else
#define GetWindowText  GetWindowTextA
#endif // !UNICODE

int GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount);
int GetWindowTextW(HWND hWnd, LPWSTR lpString, int nMaxCount);

#ifdef __cplusplus
}
#endif

#endif