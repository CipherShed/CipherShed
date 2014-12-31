#include "MessageBox.h"
#include "NULL.h"

int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	if (hWnd==NULL) return 0;
	
	return *((int*)hWnd);
}
int MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	if (hWnd==NULL) return 0;
	
	return *((int*)hWnd);
}


