#ifndef _faux_windows_bool_h_
#define _faux_windows_bool_h_

#include "FAR.h"
#include "NEAR.h"

typedef int                 BOOL;

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

typedef BOOL near           *PBOOL;
typedef BOOL far            *LPBOOL;


#endif