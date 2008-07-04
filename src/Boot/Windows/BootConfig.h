/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.5 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#ifndef TC_HEADER_Boot_BootConfig
#define TC_HEADER_Boot_BootConfig

#include "Crypto.h"
#include "Platform.h"
#include "BootDiskIo.h"

extern byte BootSectorFlags;

extern byte BootLoaderDrive;
extern byte BootDrive;
extern bool BootDriveGeometryValid;
extern DriveGeometry BootDriveGeometry;
extern bool PreventHiddenSystemBoot;

extern CRYPTO_INFO *BootCryptoInfo;
extern Partition EncryptedVirtualPartition;

extern Partition ActivePartition;
extern Partition PartitionFollowingActive;
extern uint64 HiddenVolumeStartUnitNo;
extern uint64 HiddenVolumeStartSector;


BiosResult UpdateBootSectorConfiguration (byte drive);

#endif // TC_HEADER_Boot_BootConfig
