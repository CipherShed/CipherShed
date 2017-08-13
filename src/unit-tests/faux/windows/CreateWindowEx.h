#ifndef _CS_unittesting_faux_windows_CreateWindowEx_h_
#define _CS_unittesting_faux_windows_CreateWindowEx_h_

#include "HWND.h"
#include "DWORD.h"
#include "LPCSTR.h"
#include "HMENU.h"
#include "HINSTANCE.h"
#include "LPVOID.h"
#include "LPCWSTR.h"

#ifdef __cplusplus
extern "C" {
#endif

HWND CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

HWND CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);

#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define CreateWindowEx  CreateWindowExW
#else
#define CreateWindowEx  CreateWindowExA
#endif // !UNICODE

#define WS_EX_LAYERED           0x00080000

#define WS_EX_MDICHILD          0x00000040L
#define WS_EX_TOOLWINDOW        0x00000080L
#define WS_EX_WINDOWEDGE        0x00000100L
#define WS_EX_CLIENTEDGE        0x00000200L
#define WS_EX_CONTEXTHELP       0x00000400L

#define SM_CXSCREEN             0
#define SM_CYSCREEN             1


#endif