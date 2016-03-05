/*  cs_secure_boot.h - CipherShed EFI boot loader
 *
 *	Copyright (c) 2015-2016  Falk Nedwal
 *
 *	Governed by the Apache 2.0 License the full text of which is contained in
 *	the file License.txt included in CipherShed binary and source code distribution
 *	packages.
 */

#ifndef _CS_SECURE_BOOT_H_
#define _CS_SECURE_BOOT_H_

#include <efi.h>
#include <efilib.h>
#include <efibind.h>
#include "include/PeImage.h"

/*  taken from shim.h, surprisingly this definition does not use the EFIAPI... */
struct grub_efi_shim_lock
{
  EFI_STATUS (*verify)(IN VOID *buffer, IN UINT32 size);
  EFI_STATUS (*hash)(IN char *data,	IN int datasize, PE_COFF_LOADER_IMAGE_CONTEXT *context,
						UINT8 *sha256hash, UINT8 *sha1hash);
  EFI_STATUS (*context)(IN VOID *data, IN unsigned int datasize, PE_COFF_LOADER_IMAGE_CONTEXT *context);
};

UINT8 cs_secureboot_enabled(void);
BOOLEAN cs_secureboot_validate(IN EFI_HANDLE ControllerHandle, IN VOID *data, IN UINT32 size);

#endif /* _CS_SECURE_BOOT_H_ */
