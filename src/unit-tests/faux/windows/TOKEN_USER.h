#ifndef _CS_unittesting_faux_windows_TOKEN_USER_h_
#define _CS_unittesting_faux_windows_TOKEN_USER_h_

#include "SID_AND_ATTRIBUTES.h"

typedef struct _TOKEN_USER {
    SID_AND_ATTRIBUTES User;
} TOKEN_USER, *PTOKEN_USER;

#endif