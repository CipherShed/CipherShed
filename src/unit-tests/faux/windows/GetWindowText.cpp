#include "GetWindowText.h"

#include "NULL.h"

/**
per: http://msdn.microsoft.com/en-us/library/windows/desktop/ms633520(v=vs.85).aspx

Parameters
hWnd [in]
Type: HWND

A handle to the window or control containing the text. 

lpString [out]
Type: LPTSTR

The buffer that will receive the text. If the string is as long or longer than the buffer, 
the string is truncated and terminated with a null character. 

nMaxCount [in]
Type: int

The maximum number of characters to copy to the buffer, including the null character. 
If the text exceeds this limit, it is truncated. 



Return value
Type: 
Type: int


If the function succeeds, the return value is the length, in characters, of the copied string, 
not including the terminating null character. If the window has no title bar or text, if the 
title bar is empty, or if the window or control handle is invalid, the return value is zero. 
To get extended error information, call GetLastError. 

This function cannot retrieve the text of an edit control in another application.

Remarks
If the target window is owned by the current process, GetWindowText causes a WM_GETTEXT message 
to be sent to the specified window or control. If the target window is owned by another process 
and has a caption, GetWindowText retrieves the window caption text. If the window does not have 
a caption, the return value is a null string. This behavior is by design. It allows applications 
to call GetWindowText without becoming unresponsive if the process that owns the target window 
is not responding. However, if the target window is not responding and it belongs to the calling 
application, GetWindowText will cause the calling application to become unresponsive. 

To retrieve the text of a control in another process, send a WM_GETTEXT message directly instead 
of calling GetWindowText. 


*/
int GetWindowTextA(HWND hWnd, LPSTR lpString, int nMaxCount)
{
	if (hWnd==NULL)
	{
		return 0;
	}
	if (lpString==NULL)
	{
		return 0;
	}
	if (nMaxCount==0)
	{
		return 0;
	}
	fauxWindowText* fwt=(fauxWindowText*)hWnd;
	char* src=fwt->buf.a;
    
	int i=nMaxCount;

	do
	{
		if(!(*lpString=*src)) break;
        ++lpString;
        ++src;
	}
	while (--i);

    if (!i)
	{
		*--lpString=0;
		return nMaxCount-1;
	}
	else
	{
		return nMaxCount-i;
	}
}

int GetWindowTextW(HWND hWnd, LPWSTR lpString, int nMaxCount)
{
	if (hWnd==NULL)
	{
		return 0;
	}
	if (lpString==NULL)
	{
		return 0;
	}
	if (nMaxCount==0)
	{
		return 0;
	}
	fauxWindowText* fwt=(fauxWindowText*)hWnd;
	WCHAR* src=(WCHAR*)fwt->buf.w;

	int i=nMaxCount;

	do
	{
		if(!(*lpString=*src)) break;
        ++lpString;
        ++src;
	}
	while (--i);

    if (!i)
	{
		*--lpString=0;
		return nMaxCount-1;
	}
	else
	{
		return nMaxCount-i;
	}
}