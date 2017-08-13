#ifndef CS_unittesting_faux_windows_loadcursor_h_
#define CS_unittesting_faux_windows_loadcursor_h_

#include "HCURSOR.h"
#include "HINSTANCE.h"
#include "LPCSTR.h"
#include "LPCWSTR.h"

HCURSOR
LoadCursorA(
    HINSTANCE hInstance,
    LPCSTR lpCursorName);

HCURSOR
LoadCursorW(
    HINSTANCE hInstance,
    LPCWSTR lpCursorName);

#ifdef UNICODE
#define LoadCursor  LoadCursorW
#else
#define LoadCursor  LoadCursorA
#endif // !UNICODE


#endif
