#ifndef _faux_windows_wchar_h_
#define _faux_windows_wchar_h_

#ifndef __nullterminated 
#define __nullterminated 
#endif

typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
typedef __nullterminated WCHAR *NWPSTR, *LPWSTR, *PWSTR;

#endif