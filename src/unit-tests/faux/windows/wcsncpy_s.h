#ifndef CS_unittests_faux_windows_wcsncpy_s_h_
#define CS_unittests_faux_windows_wcsncpy_s_h_

#ifdef _MSC_VER
#include <wchar.h>
#else
#include "errno_t.h"
#include "size_t.h"
errno_t wcsncpy_s(wchar_t *strDest, size_t numberOfElements, const wchar_t *strSource, size_t count);
#endif

#endif