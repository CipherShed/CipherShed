#ifndef CS_unittests_faux_windows_FormatMessage_h_
#define CS_unittests_faux_windows_FormatMessage_h_

#include <stdarg.h> 
#include "DWORD.h"
#include "VOID.h"
#include "CHAR.h"
#include "WCHAR.h"

#ifdef __cplusplus
extern "C" {
#endif

DWORD
FormatMessageA(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments
    );

DWORD
FormatMessageW(
    DWORD dwFlags,
    LPCVOID lpSource,
    DWORD dwMessageId,
    DWORD dwLanguageId,
    LPWSTR lpBuffer,
    DWORD nSize,
    va_list *Arguments
    );

#ifdef __cplusplus
}
#endif

#ifdef UNICODE
#define FormatMessage  FormatMessageW
#else
#define FormatMessage  FormatMessageA
#endif // !UNICODE


#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_IGNORE_INSERTS  0x00000200
#define FORMAT_MESSAGE_FROM_STRING     0x00000400
#define FORMAT_MESSAGE_FROM_HMODULE    0x00000800
#define FORMAT_MESSAGE_FROM_SYSTEM     0x00001000
#define FORMAT_MESSAGE_ARGUMENT_ARRAY  0x00002000
#define FORMAT_MESSAGE_MAX_WIDTH_MASK  0x000000FF


#endif
