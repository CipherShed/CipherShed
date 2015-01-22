#ifndef _CS_unittests_faux_windows_MessageBox_h_
#define _CS_unittests_faux_windows_MessageBox_h_

#include "HWND.h"
#include "LPCSTR.h"
#include "UINT.h"
#include "LPCWSTR.h"

typedef struct fauxMessageBox
{
	int retval;
	union
	{
		char* a;
		WCHAR* w;
	} text;
	union
	{
		char* a;
		WCHAR* w;
	} caption;
	UINT type;
} fauxMessageBox;

#ifdef __cplusplus
extern "C" {
#endif

int MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
int MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
#define MessageBox  MessageBoxA

#ifdef __cplusplus
}
#endif


#endif
