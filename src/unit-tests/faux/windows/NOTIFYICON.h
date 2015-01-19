#ifndef CS_unittests_faux_windows_notifyicon_h_
#define CS_unittests_faux_windows_notifyicon_h_

#include "DWORD.h"
#include "UINT.h"
#include "HWND.h"
#include "HICON.h"
#include "CHAR.h"
#include "GUID.h"
#include "WCHAR.h"

typedef struct _NOTIFYICONDATAA {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    CHAR   szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    CHAR   szInfo[256];
    union {
        UINT  uTimeout;
        UINT  uVersion;  // used with NIM_SETVERSION, values 0, 3 and 4
    } /*DUMMYUNIONNAME*/;
    CHAR   szInfoTitle[64];
    DWORD dwInfoFlags;
    GUID guidItem;
    HICON hBalloonIcon;
} NOTIFYICONDATAA, *PNOTIFYICONDATAA;

typedef struct _NOTIFYICONDATAW {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    WCHAR  szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    WCHAR  szInfo[256];
    union {
        UINT  uTimeout;
        UINT  uVersion;  // used with NIM_SETVERSION, values 0, 3 and 4
    } /*DUMMYUNIONNAME*/;
    WCHAR  szInfoTitle[64];
    DWORD dwInfoFlags;
    GUID guidItem;
    HICON hBalloonIcon;
} NOTIFYICONDATAW, *PNOTIFYICONDATAW;

#ifdef UNICODE
typedef NOTIFYICONDATAW NOTIFYICONDATA;
typedef PNOTIFYICONDATAW PNOTIFYICONDATA;
#else
typedef NOTIFYICONDATAA NOTIFYICONDATA;
typedef PNOTIFYICONDATAA PNOTIFYICONDATA;
#endif // UNICODE


#endif
