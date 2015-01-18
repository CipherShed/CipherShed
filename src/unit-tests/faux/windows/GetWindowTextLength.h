#ifndef _faux_windows_GetWindowTextLength_h_
#define _faux_windows_GetWindowTextLength_h_

#include "HWND.h"
#include "WCHAR.h"

#define fauxWindowText_BUFFERSIZE 2048

typedef struct fauxWindowText
{
	int n;
	union
	{
		WCHAR w[fauxWindowText_BUFFERSIZE];
		char a[fauxWindowText_BUFFERSIZE*2];
	} buf;
} fauxWindowText;

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