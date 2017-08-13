#ifndef _CS_Common_volume_volutil_h_
#define _CS_Common_volume_volutil_h_

#include "../Tcdefs.h"

#ifdef CS_UNITTESTING
#include "../../unit-tests/faux/windows/BOOL.h"
#include "../../unit-tests/faux/windows/LPWSTR.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void CreateFullVolumePath (char *lpszDiskFile, const char *lpszFileName, BOOL * bDevice);
BOOL IsVolumeDeviceHosted (const char *lpszDiskFile);
BOOL IsMountedVolume (const char *volname);
int GetMountedVolumeDriveNo (char *volname);

#ifdef __cplusplus
}
#include <string>
std::string VolumeGuidPathToDevicePath (std::string volumeGuidPath);
std::string HarddiskVolumePathToPartitionPath (const std::string &harddiskVolumePath);
#endif


#endif