#ifndef _faux_windows_GetWindowText_h_
#define _faux_windows_GetWindowText_h_

#include "HWND.h"
#include "CHAR.h"

int GetWindowText(HWND hWnd, LPSTR lpString, int nMaxCount);

#endif