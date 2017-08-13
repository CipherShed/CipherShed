#ifndef _faux_windows_disk_geometry_h_
#define _faux_windows_disk_geometry_h_

#include "MEDIA_TYPE.h"

typedef struct _DISK_GEOMETRY {
    LARGE_INTEGER Cylinders;
    MEDIA_TYPE MediaType;
    DWORD TracksPerCylinder;
    DWORD SectorsPerTrack;
    DWORD BytesPerSector;
} DISK_GEOMETRY, *PDISK_GEOMETRY;

#endif