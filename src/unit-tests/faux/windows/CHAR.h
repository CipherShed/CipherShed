#ifndef _faux_windows_char_h_
#define _faux_windows_char_h_

#include "CONST.h"

#include "__nullterminated.h"

#ifndef __nullnullterminated
#define __nullnullterminated
#endif

#ifndef __possibly_notnullterminated
#define __possibly_notnullterminated
#endif

typedef char CHAR;

typedef CHAR *PCHAR, *LPCH, *PCH;
typedef CONST CHAR *LPCCH, *PCCH;

typedef __nullterminated CHAR *NPSTR, *LPSTR, *PSTR;
typedef __nullterminated PSTR *PZPSTR;
typedef __nullterminated CONST PSTR *PCZPSTR;
typedef __nullterminated CONST CHAR *LPCSTR, *PCSTR;
typedef __nullterminated PCSTR *PZPCSTR;

typedef __nullnullterminated CHAR *PZZSTR;
typedef __nullnullterminated CONST CHAR *PCZZSTR;

typedef __possibly_notnullterminated CHAR *PNZCH;
typedef __possibly_notnullterminated CONST CHAR *PCNZCH;

typedef unsigned char UCHAR;
typedef UCHAR *PUCHAR;


#endif