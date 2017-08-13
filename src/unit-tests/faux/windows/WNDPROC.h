#ifndef _CS_unittesting_faux_windows_wndproc_h_
#define _CS_unittesting_faux_windows_wndproc_h_

#include "LRESULT.h"
#include "CALLBACK.h"
#include "HWND.h"
#include "UINT.h"
#include "WPARAM.h"
#include "LPARAM.h"

typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

#endif