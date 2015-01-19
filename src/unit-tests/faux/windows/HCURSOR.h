#ifndef CS_unittesting_faux_windows_hcursor_h_
#define CS_unittesting_faux_windows_hcursor_h_

#include "HICON.h"
#include "DECLARE_HANDLE.h"

#ifndef _MAC
typedef int HFILE;
typedef HICON HCURSOR;      /* HICONs & HCURSORs are polymorphic */
#else
typedef short HFILE;
DECLARE_HANDLE(HCURSOR);    /* HICONs & HCURSORs are not polymorphic */
#endif

#endif