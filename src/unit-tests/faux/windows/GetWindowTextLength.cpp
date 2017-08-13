#include "GetWindowTextLength.h"

#include "NULL.h"
#include "WCHAR.h"

/**
per: http://msdn.microsoft.com/en-us/library/windows/desktop/ms633521(v=vs.85).aspx

Parameters
hWnd [in]
Type: HWND

A handle to the window or control.

Return value
Type: 
Type: int


If the function succeeds, the return value is the length, in characters, of the 
text. Under certain conditions, this value may actually be greater than the 
length of the text. For more information, see the following Remarks section.

If the window has no text, the return value is zero. To get extended error 
information, call GetLastError. 

Remarks
If the target window is owned by the current process, GetWindowTextLength causes 
a WM_GETTEXTLENGTH message to be sent to the specified window or control. 

Under certain conditions, the GetWindowTextLength function may return a value that 
is larger than the actual length of the text. This occurs with certain mixtures of 
ANSI and Unicode, and is due to the system allowing for the possible existence of 
double-byte character set (DBCS) characters within the text. The return value, 
however, will always be at least as large as the actual length of the text; 
you can thus always use it to guide buffer allocation. This behavior can occur when 
an application uses both ANSI functions and common dialogs, which use Unicode. It 
can also occur when an application uses the ANSI version of GetWindowTextLength with 
a window whose window procedure is Unicode, or the Unicode version of GetWindowTextLength 
with a window whose window procedure is ANSI. For more information on ANSI and 
ANSI functions, see Conventions for Function Prototypes. 

To obtain the exact length of the text, use the WM_GETTEXT, LB_GETTEXT, or 
CB_GETLBTEXT messages, or the GetWindowText function.



*/

int GetWindowTextLengthA(HWND hWnd)
{
	if (hWnd==NULL)
	{
		return 0;
	}
	fauxWindowText* fwt=(fauxWindowText*)hWnd;
	char* src=fwt->buf.a;
	int cnt=0;
	while(*src++)
	{
			++cnt;
	}
	return cnt;
}

int GetWindowTextLengthW(HWND hWnd)
{
	if (hWnd==NULL)
	{
		return 0;
	}
	fauxWindowText* fwt=(fauxWindowText*)hWnd;
	WCHAR* src=fwt->buf.w;
	int cnt=0;
	while(*src++)
	{
			++cnt;
	}
	return cnt;
}
