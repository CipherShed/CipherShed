#ifndef CS_Common_util_unicode_strcmpw_h_
#define CS_Common_util_unicode_strcmpw_h_

#ifndef CS_UNITTESTING
#include <windows.h>
#else
#include "../../../unit-tests/faux/windows/WCHAR.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

int strcmpw(WCHAR* a, WCHAR* b);

#ifdef __cplusplus
}
#endif

#endif