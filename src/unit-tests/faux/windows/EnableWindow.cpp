#include "EnableWindow.h"

BOOL EnableWindow(HWND hWnd, BOOL bEnable)
{
	BOOL res=false;
	if (hWnd)
	{
		fauxEnablableWidget* few=(fauxEnablableWidget*)hWnd;
		if (few->enabled) 
		{
			res=true;
		}
		else
		{
			res=false;
		}
		few->enabled=bEnable;
	}
	return res;
}