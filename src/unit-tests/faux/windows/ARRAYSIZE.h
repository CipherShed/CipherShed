#ifndef CS_unittests_faux_windows_ARRAYSIZE_h_
#define CS_unittests_faux_windows_ARRAYSIZE_h_

#include "size_t.h"

extern "C++" // templates cannot be declared to have 'C' linkage
template <typename T, size_t N>
char (*RtlpNumberOf( UNALIGNED T (&)[N] ))[N];

#define RTL_NUMBER_OF_V2(A) (sizeof(*RtlpNumberOf(A)))

#define RTL_NUMBER_OF_V1(A) (sizeof(A)/sizeof((A)[0]))


#define ARRAYSIZE(A)    RTL_NUMBER_OF_V2(A)
#define _ARRAYSIZE(A)   RTL_NUMBER_OF_V1(A)

#endif