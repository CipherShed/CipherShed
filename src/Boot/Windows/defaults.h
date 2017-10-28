#ifndef _boot_windows_defaults_h_
#define _boot_windows_defaults_h_

#if !defined(_MSC_VER) && defined(__GNUC__) && __ia16__==1

#define far
#define LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#define BYTE_ORDER LITTLE_ENDIAN
#define TRUE 1
#define FALSE 0
#define __int8 char
#define __int16 int
#define __int32 long
#define BOOL char
#define TC_WINDOWS_BOOT
#define TC_MINIMIZE_CODE_SIZE
#define TC_NO_COMPILER_INT64

#endif

#endif
