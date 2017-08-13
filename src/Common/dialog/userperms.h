#ifndef CS_Common_dialog_userperms_h_
#define CS_Common_dialog_userperms_h_

#ifndef CS_UNITTESTING
#include <windows.h>
#else
#include "../../unit-tests/faux/windows/BOOL.h"
#endif

#include "../Common.h"
#include "../Dlgcode.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL UacElevated;

BOOL IsUacSupported();
BOOL IsOSAtLeast(OSVersionEnum reqMinOS);
BOOL IsOSVersionAtLeast(OSVersionEnum reqMinOS, int reqMinServicePack);
BOOL Is64BitOs();
BOOL IsServerOS();
BOOL IsAdmin (void);
BOOL IsBuiltInAdmin ();

void EnableElevatedCursorChange (HWND parent);

#ifdef __cplusplus
}
#endif


#endif
