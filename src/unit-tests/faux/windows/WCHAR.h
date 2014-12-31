#ifndef _faux_windows_wchar_h_
#define _faux_windows_wchar_h_

#include "CONST.h"

#ifndef __nullterminated 
#define __nullterminated 
#endif

#ifndef __nullnullterminated
#define __nullnullterminated
#endif

#ifndef __possibly_notnullterminated
#define __possibly_notnullterminated
#endif

#define UNALIGNED
#ifdef _MSC_VER
typedef unsigned short WCHAR;
#else
typedef wchar_t WCHAR;
#endif
//typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character

typedef WCHAR *PWCHAR, *LPWCH, *PWCH;
typedef CONST WCHAR *LPCWCH, *PCWCH;

typedef __nullterminated WCHAR *NWPSTR, *LPWSTR, *PWSTR;
typedef __nullterminated PWSTR *PZPWSTR;
typedef __nullterminated CONST PWSTR *PCZPWSTR;
typedef __nullterminated WCHAR UNALIGNED *LPUWSTR, *PUWSTR;
typedef __nullterminated CONST WCHAR *LPCWSTR, *PCWSTR;
typedef __nullterminated PCWSTR *PZPCWSTR;
typedef __nullterminated CONST WCHAR UNALIGNED *LPCUWSTR, *PCUWSTR;

typedef __nullnullterminated WCHAR *PZZWSTR;
typedef __nullnullterminated CONST WCHAR *PCZZWSTR;
typedef __nullnullterminated WCHAR UNALIGNED *PUZZWSTR;
typedef __nullnullterminated CONST WCHAR UNALIGNED *PCUZZWSTR;

typedef __possibly_notnullterminated WCHAR *PNZWCH;
typedef __possibly_notnullterminated CONST WCHAR *PCNZWCH;
typedef __possibly_notnullterminated WCHAR UNALIGNED *PUNZWCH;
typedef __possibly_notnullterminated CONST WCHAR UNALIGNED *PCUNZWCH;


#endif