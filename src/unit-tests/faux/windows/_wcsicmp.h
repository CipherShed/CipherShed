#ifndef _CS_unittesting_faux_windows__wcsicmp_h_
#define _CS_unittesting_faux_windows__wcsicmp_h_

#ifndef _MSC_VER
int _wcsicmp(const wchar_t *string1, const wchar_t *string2);
#else
#include <string.h>
#include <wchar.h>
#endif

#endif
