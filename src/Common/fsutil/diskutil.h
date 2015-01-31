#ifndef _CS_Common_fsutil_diskutil_h_
#define _CS_Common_fsutil_diskutil_h_

#include "../Tcdefs.h"
#include "../Dlgcode.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL GetDeviceInfo (const char *deviceName, DISK_PARTITION_INFO_STRUCT *info);
BOOL GetDriveGeometry (const char *deviceName, PDISK_GEOMETRY diskGeometry);
int GetDiskDeviceDriveLetter (PWSTR deviceName);

#ifdef __cplusplus
}
#endif

#endif
