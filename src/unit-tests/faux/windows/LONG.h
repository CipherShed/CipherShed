#ifndef _faux_windows_long_h_
#define _faux_windows_long_h_

//#include <cstdint>
//#include <stdint.h>
//#define __int64 int64_t

typedef long LONG;
typedef __int64 LONGLONG;
typedef unsigned __int64 ULONGLONG;

#define MAXLONGLONG                         (0x7fffffffffffffff)

typedef unsigned long ULONG;
typedef ULONG *PULONG;



#endif