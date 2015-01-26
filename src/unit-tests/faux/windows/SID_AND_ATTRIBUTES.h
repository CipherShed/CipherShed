#ifndef _CS_unittesting_faux_windows_SID_AND_ATTRIBUTES_h_
#define _CS_unittesting_faux_windows_SID_AND_ATTRIBUTES_h_

//#ifdef MIDL_PASS
//#include "PISID.h"
//#else // MIDL_PASS
#include "PSID.h"
//#endif // MIDL_PASS

#include "ULONG.h"

typedef struct _SID_AND_ATTRIBUTES {
//#ifdef MIDL_PASS
//    PISID Sid;
//#else // MIDL_PASS
    PSID Sid;
//#endif // MIDL_PASS
    ULONG Attributes;
    } SID_AND_ATTRIBUTES, * PSID_AND_ATTRIBUTES;


#endif