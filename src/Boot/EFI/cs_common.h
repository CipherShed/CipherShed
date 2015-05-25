/*  cs_common.h - CipherShed EFI boot loader
 *
 *
 *
 */

#ifndef _CS_COMMON_H_
#define _CS_COMMON_H_

#include <efi.h>
#include <efilib.h>
#include <efibind.h>

#include "cs_crypto.h"
#include "cs_debug.h"

#define CS_LOADER_NAME		WIDEN("CipherShed")
#define CS_LOADER_VERSION	0.1

#define CS_DRIVER_NAME		CS_LOADER_NAME
#define CS_CONTROLLER_NAME	CS_LOADER_NAME
#define CS_DRIVER_VERSION	CS_LOADER_VERSION

/* only for test purposes: allow to build arbitrary volume header */
#define CS_TEST_CREATE_VOLUME_HEADER

#ifndef ARRAYSIZE
#define ARRAYSIZE(A)	(sizeof(A)/sizeof((A)[0]))
#endif

#ifndef MIN
#define MIN(x,y)		((x)<(y)?(x):(y))
#endif

#define _STRINGIFY(s)	#s
#define STRINGIFY(s)	_STRINGIFY(s)

#define _WIDEN(s)		L ## s
#define WIDEN(s)		_WIDEN(s)

#define CS_VOLUME_HEADER_SIZE		512			/* size of volume header, needed for buffer allocation */
#define CS_LENGTH_FILENAME_VOLUMNE_HEADER	36	/* length of a GUID */
#define CS_MAX_DRIVER_PATH_SIZE		1024		/* maximum path size to the crypto driver */
#define CS_CHILD_PATH_EXTENSION		WIDEN("crypto")		/* extension for Device Path Protocol for the new
 	 	 	 	 	 	 	 	 	 	 	 	 	 	   created device offering BlockIO protocol */
#define CS_HANDOVER_VARIABLE_NAME	WIDEN("cs_data")	/* name of the UEFI variable for runtime service
 	 	 	 	 	 	 	 	 	 	 	 	 	 	   for the hand-over data to OS driver */
/* the following GUID is needed to access the EFI variable, see SetVariable/GetVariable */
#define CS_HANDOVER_VARIABLE_GUID     \
    { 0x16ca79bf, 0x55b8, 0x478a, {0xb8, 0xf1, 0xfe, 0x39, 0x3b, 0xdd, 0xa1, 0x06} }

/* taken (and modified) from TC BootDiskIo.h: struct Partition
 * -> this might be adjusted since in TC it's based on MBR based partitions */
struct cs_partition_info
{
	byte Number;
	byte Drive;
	BOOLEAN Active;
	uint64 EndSector;
	BOOLEAN Primary;
	uint64 SectorCount;
	uint64 StartSector;
	byte Type;
};

/* set of data that is needed to use encryption algorithms */
struct cs_cipher_data {
	UINT8 algo;						/* disk encryption algorithm */
	UINT8 mode;						/* encryption mode */
	UINT8 ks[MAX_EXPANDED_KEY];		/* Primary key schedule (if it is a cascade,
										it contains multiple concatenated keys) */
	UINT8 ks2[MAX_EXPANDED_KEY];	/* Secondary key schedule
										(if cascade, multiple concatenated) for XTS mode. */
};

/* structure of data that are handed over from loader to EFI driver */
struct cs_efi_option_data {
	uint64 StartSector;				/* TC: PartitionFollowingActive.StartSector */
	uint64 EndSector;				/* TC: PartitionFollowingActive.EndSector */
	uint64 SectorCount;				/* TC: PartitionFollowingActive.SectorCount */
	BOOLEAN isHiddenVolume;			/* TC: BootCryptoInfo->hiddenVolume */
	uint64 HiddenVolumeStartSector;
	uint64 HiddenVolumeStartUnitNo;

	struct cs_cipher_data cipher;	/* cipher parameters and key data */
	UINTN debug;					/* the debug level of the driver */
};

struct _cs_device_path {
	FILEPATH_DEVICE_PATH value;
	CHAR8 __cs_path[CS_MAX_DRIVER_PATH_SIZE];	/* used to allocate memory, don't access this directly! */
};

#pragma pack(1)
/* taken from TrueCrypt: needed for hand over of data to OS driver in RAM */
typedef struct
{
	/* Modifying this structure can introduce incompatibility with previous versions */
	char Signature[8];
	UINT16 BootLoaderVersion;
	UINT16 CryptoInfoOffset;
	UINT16 CryptoInfoLength;
	UINT32 HeaderSaltCrc32;
	Password BootPassword;
	UINT64 HiddenSystemPartitionStart;
	UINT64 DecoySystemPartitionStart;
	UINT32 Flags;
	UINT32 BootDriveSignature;
	UINT32 BootArgumentsCrc32;
} BootArguments;

/* the following structure is intended to be handed over to the OS driver,
 * it contains the necessary keys and information */
struct cs_driver_data {
	BootArguments boot_arguments;
	CRYPTO_INFO crypto_info;
	UINT8 volume_header[CS_VOLUME_HEADER_SIZE];	/* encrypted volume header */
	/* some more data are needed:
	 * - GUID of the partition where the volume header is located, see context.caller_disk.signature
	 * - full path to the volume header file at this partition, see context.vh_path[] */
};
#pragma pack()

/* Modifying these values can introduce incompatibility with previous versions */
#define TC_BOOT_ARGS_FLAG_EXTRA_BOOT_PARTITION				0x1
/* indicates whether theencrypted volume header sector was handed over by the boot loader */
#define TC_BOOT_ARGS_FLAG_BOOT_VOLUME_HEADER_PRESENT		0x2

/* Boot arguments signature should not be defined as a static string
   Modifying these values can introduce incompatibility with previous versions */
#define TC_SET_BOOT_ARGUMENTS_SIGNATURE(SG) do { SG[0]  = 'T';   SG[1]  = 'R';   SG[2]  = 'U';   SG[3]  = 'E';   SG[4]  = 0x11;   SG[5]  = 0x23;   SG[6]  = 0x45;   SG[7]  = 0x66; } while (FALSE)
#define TC_IS_BOOT_ARGUMENTS_SIGNATURE(SG)      (SG[0] == 'T' && SG[1] == 'R' && SG[2] == 'U' && SG[3] == 'E' && SG[4] == 0x11 && SG[5] == 0x23 && SG[6] == 0x45 && SG[7] == 0x66)

#define TC_LB_SIZE_BIT_SHIFT_DIVISOR 9	/* attention: hard coded media sector size: 512 byte */
#define TC_FIRST_BIOS_DRIVE 0x80
#define TC_LAST_BIOS_DRIVE 0x8f
#define TC_INVALID_BIOS_DRIVE (TC_FIRST_BIOS_DRIVE - 1)


void cs_print_msg(IN CHAR16 *format, ...);
void cs_exception(IN CHAR16 *format, ...);
void cs_sleep(IN UINTN n);
UINT32 __div64_32(UINT64 *n, UINT32 base);

#endif /* _CS_COMMON_H_ */
