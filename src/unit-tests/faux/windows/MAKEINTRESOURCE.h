#ifndef CS_unittesting_faux_windows_MAKEINTRESOURCE_h_
#define CS_unittesting_faux_windows_MAKEINTRESOURCE_h_

#include "LONG.h"
#include "LONG_PTR.h"
#include "CHAR.h"
#include "WCHAR.h"
#include "WORD.h"

#define IS_INTRESOURCE(_r) ((((ULONG_PTR)(_r)) >> 16) == 0)
#define MAKEINTRESOURCEA(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
#define MAKEINTRESOURCEW(i) ((LPWSTR)((ULONG_PTR)((WORD)(i))))
#ifdef UNICODE
#define MAKEINTRESOURCE  MAKEINTRESOURCEW
#else
#define MAKEINTRESOURCE  MAKEINTRESOURCEA
#endif // !UNICODE


#endif
