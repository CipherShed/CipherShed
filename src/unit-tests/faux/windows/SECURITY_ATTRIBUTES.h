#ifndef _faux_windows_SECURITY_ATTRIBUTES_h_
#define _faux_windows_SECURITY_ATTRIBUTES_h_

#include "DWORD.h"
#include "BOOL.h"
#include "VOID.h"

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;


#endif