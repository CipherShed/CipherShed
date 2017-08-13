#include "MessageBox.h"
#include "NULL.h"

int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	fauxMessageBox* fmb=(fauxMessageBox*)hWnd;
	if (hWnd==NULL) return 0;
	
	return fmb->retval;
}
int MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	fauxMessageBox* fmb=(fauxMessageBox*)hWnd;
	if (hWnd==NULL) return 0;
	
	return fmb->retval;
}


