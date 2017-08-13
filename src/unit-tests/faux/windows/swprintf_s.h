#ifndef _CS_unittesting_faux_windows_swprintf_s_h_
#define _CS_unittesting_faux_windows_swprintf_s_h_

//stdio.h
#ifndef _MSC_VER
int swprintf_s(wchar_t * _Dst, unsigned int _SizeInWords, const wchar_t * _Format, ...);
#else
#include <stdio.h>
#include <wchar.h>
#endif


#endif
