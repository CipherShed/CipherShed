/*
 Copyright (c) 2008 TrueCrypt Foundation. All rights reserved.

 Governed by the TrueCrypt License 2.5 the full text of which is contained
 in the file License.txt included in TrueCrypt binary and source code
 distribution packages.
*/

#include "BootConfig.h"

byte BootSectorFlags;

byte BootLoaderDrive;
byte BootDrive;
bool BootDriveGeometryValid = false;
bool PreventHiddenSystemBoot = false;

DriveGeometry BootDriveGeometry;

CRYPTO_INFO *BootCryptoInfo;
Partition EncryptedVirtualPartition;

Partition ActivePartition;
Partition PartitionFollowingActive;
uint64 HiddenVolumeStartUnitNo;
uint64 HiddenVolumeStartSector;


BiosResult UpdateBootSectorConfiguration (byte drive)
{
	AcquireSectorBuffer();

	BiosResult result = ReadWriteMBR (false, drive);
	if (result != BiosResultSuccess)
		goto ret;

	SectorBuffer[TC_BOOT_SECTOR_CONFIG_OFFSET] = BootSectorFlags;
	result = ReadWriteMBR (true, drive);

ret:
	ReleaseSectorBuffer();
	return result;
}
