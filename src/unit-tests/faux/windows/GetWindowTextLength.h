#ifndef _faux_windows_GetWindowTextLength_h_
#define _faux_windows_GetWindowTextLength_h_

#include "HWND.h"

#ifdef __cplusplus
extern "C" {
#endif

int GetWindowTextLengthA(HWND hWnd);
int GetWindowTextLengthW(HWND hWnd);

#ifdef UNICODE
#define GetWindowTextLength  GetWindowTextLengthW
#else
#define GetWindowTextLength  GetWindowTextLengthA
#endif // !UNICODE

#ifdef __cplusplus
}
#endif

#endif