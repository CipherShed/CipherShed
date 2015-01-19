#ifndef CS_unittests_faux_windows_shellnotifyicon_h_
#define CS_unittests_faux_windows_shellnotifyicon_h_

#include "BOOL.h"
#include "DWORD.h"
#include "NOTIFYICON.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL Shell_NotifyIconA(DWORD dwMessage, PNOTIFYICONDATAA lpData);
BOOL Shell_NotifyIconW(DWORD dwMessage, PNOTIFYICONDATAW lpData);

#ifdef __cplusplus
}
#endif


#endif