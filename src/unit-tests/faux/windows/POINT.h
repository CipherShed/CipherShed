#ifndef _faux_windows_point_h_
#define _faux_windows_point_h_

#include "LONG.h"
#include "NEAR.h"
#include "FAR.h"

typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT, *PPOINT, NEAR *NPPOINT, FAR *LPPOINT;

#endif