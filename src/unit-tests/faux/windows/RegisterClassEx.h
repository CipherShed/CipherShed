#ifndef _CS_unittesting_faux_windows_RegisterClassEx_h_
#define _CS_unittesting_faux_windows_RegisterClassEx_h_

#include "CONST.h"
#include "WNDCLASSEX.h"
#include "ATOM.h"

#ifdef __cplusplus
extern "C" {
#endif

ATOM RegisterClassExA(CONST WNDCLASSEXA *);

ATOM RegisterClassExW(CONST WNDCLASSEXW *);

#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define RegisterClassEx  RegisterClassExW
#else
#define RegisterClassEx  RegisterClassExA
#endif // !UNICODE


#endif