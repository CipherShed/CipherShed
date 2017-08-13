#ifndef _CS_Common_fsutil_fsutil_h_
#define _CS_Common_fsutil_fsutil_h_

#include "../Tcdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL ResolveSymbolicLink (const wchar_t *symLinkName, PWSTR targetName);
int FileSystemAppearsEmpty (const char *devicePath);

#ifdef __cplusplus
}
#endif


#endif