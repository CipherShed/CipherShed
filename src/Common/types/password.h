#ifndef _CS_Common_types_password_h_
#define _CS_Common_types_password_h_

#include "../constants.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	// Modifying this structure can introduce incompatibility with previous versions
	unsigned __int32 Length;
	unsigned char Text[MAX_PASSWORD + 1];
	char Pad[3]; // keep 64-bit alignment
} Password;

#ifdef __cplusplus
}
#endif

#endif
