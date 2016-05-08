/*
 Portions of this file are
 Licensed to the The CipherShed Project (CP) under one
 or more contributor license agreements.  See the NOTICE file
 distributed with this work for additional information
 regarding copyright ownership.  The CP licenses this file
 to you under the Apache License, Version 2.0 (the
 "License"); you may not use this file except in compliance
 with the License.  You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing,
 software distributed under the License is distributed on an
 "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 KIND, either express or implied.  See the License for the
 specific language governing permissions and limitations
 under the License.

 Copyright (c) 2008-2010 TrueCrypt Developers Association. All rights reserved.

 Governed by the TrueCrypt License 3.0 the full text of which is contained in
 the file License.txt included in TrueCrypt binary and source code distribution
 packages.
*/

#ifndef TC_HEADER_DRIVER_DRIVE_FILTER
#define TC_HEADER_DRIVER_DRIVE_FILTER

#include "../Common/Tcdefs.h"
#include "../Boot/Windows/BootCommon.h"
#include "EncryptedIoQueue.h"

typedef struct _DriveFilterExtension
{
	BOOL bRootDevice;
	BOOL IsVolumeDevice;
	BOOL IsDriveFilterDevice;
	BOOL IsVolumeFilterDevice;
	uint64 MagicNumber;

	PDEVICE_OBJECT DeviceObject;
	PDEVICE_OBJECT LowerDeviceObject;
	PDEVICE_OBJECT Pdo;

	ULONG SystemStorageDeviceNumber;
	BOOL SystemStorageDeviceNumberValid;

	int64 ConfiguredEncryptedAreaStart;
	int64 ConfiguredEncryptedAreaEnd;

	uint32 VolumeHeaderSaltCrc32;
	EncryptedIoQueue Queue;

	BOOL BootDrive;
	BOOL VolumeHeaderPresent;
	BOOL DriveMounted;

	KEVENT MountWorkItemCompletedEvent;

	CRYPTO_INFO *HeaderCryptoInfo;
	BOOL HiddenSystem;

} DriveFilterExtension;

/* Modifying 'TRUE' can introduce incompatibility with previous versions. */
#define TC_BOOT_DRIVE_FILTER_EXTENSION_MAGIC_NUMBER 0x5452554542455854

extern BOOL BootArgsValid;
extern BootArguments BootArgs;
extern PKTHREAD EncryptionSetupThread;
extern PKTHREAD DecoySystemWipeThread;

NTSTATUS AbortBootEncryptionSetup ();
NTSTATUS DriveFilterAddDevice (PDRIVER_OBJECT driverObject, PDEVICE_OBJECT pdo);
NTSTATUS DriveFilterDispatchIrp (PDEVICE_OBJECT DeviceObject, PIRP Irp);
void GetBootDriveVolumeProperties (PIRP irp, PIO_STACK_LOCATION irpSp);
void GetBootEncryptionAlgorithmName (PIRP irp, PIO_STACK_LOCATION irpSp);
void GetBootEncryptionStatus (PIRP irp, PIO_STACK_LOCATION irpSp);
void GetBootLoaderVersion (PIRP irp, PIO_STACK_LOCATION irpSp);
NTSTATUS GetSetupResult ();
DriveFilterExtension *GetBootDriveFilterExtension ();
CRYPTO_INFO *GetSystemDriveCryptoInfo ();
BOOL IsBootDriveMounted ();
BOOL IsBootEncryptionSetupInProgress ();
BOOL IsHiddenSystemRunning ();
NTSTATUS LoadBootArguments ();
static NTSTATUS SaveDriveVolumeHeader (DriveFilterExtension *Extension);
NTSTATUS StartBootEncryptionSetup (PDEVICE_OBJECT DeviceObject, PIRP irp, PIO_STACK_LOCATION irpSp);
void ReopenBootVolumeHeader (PIRP irp, PIO_STACK_LOCATION irpSp);
NTSTATUS StartDecoySystemWipe (PDEVICE_OBJECT DeviceObject, PIRP irp, PIO_STACK_LOCATION irpSp);
void StartLegacyHibernationDriverFilter ();
NTSTATUS AbortDecoySystemWipe ();
BOOL IsDecoySystemWipeInProgress();
NTSTATUS GetDecoySystemWipeResult();
void GetDecoySystemWipeStatus (PIRP irp, PIO_STACK_LOCATION irpSp);
uint64 GetBootDriveLength ();
NTSTATUS WriteBootDriveSector (PIRP irp, PIO_STACK_LOCATION irpSp);
#ifdef EFI_WINDOWS_DRIVER
void GetBootVolumeHeader (PIRP irp, PIO_STACK_LOCATION irpSp);
#endif

#define TC_ENCRYPTION_SETUP_IO_BLOCK_SIZE (1536 * 1024)
#define TC_ENCRYPTION_SETUP_HEADER_UPDATE_THRESHOLD (64 * 1024 * 1024)
#define TC_HIBERNATION_WRITE_BUFFER_SIZE (128 * 1024)

#endif // TC_HEADER_DRIVER_DRIVE_FILTER
