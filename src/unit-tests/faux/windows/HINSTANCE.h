#ifndef _faux_windows_hinstance_h_
#define _faux_windows_hinstance_h_

#include "DECLARE_HANDLE.h"
DECLARE_HANDLE(HINSTANCE);


typedef HINSTANCE HMODULE;      /* HMODULEs can be used in place of HINSTANCEs */

#endif