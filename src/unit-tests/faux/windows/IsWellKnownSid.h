#ifndef _CS_unittesting_faux_windows_IsWellKnownSid_h_
#define _CS_unittesting_faux_windows_IsWellKnownSid_h_

#include "BOOL.h"
#include "PSID.h"
#include "WELL_KNOWN_SID_TYPE.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL IsWellKnownSid (PSID pSid, WELL_KNOWN_SID_TYPE WellKnownSidType);

#ifdef __cplusplus
}
#endif

#endif
