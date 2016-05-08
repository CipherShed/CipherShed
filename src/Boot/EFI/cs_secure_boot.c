/*  cs_secure_boot.c - CipherShed EFI boot loader
 *
 *	Copyright (c) 2015-2016  Falk Nedwal
 *
 *	Governed by the Apache 2.0 License the full text of which is contained in
 *	the file License.txt included in CipherShed binary and source code distribution
 *	packages.
 */

#include "cs_secure_boot.h"
#include "cs_debug.h"

EFI_GUID SHIM_LOCK_GUID = { 0x605dab50, 0xe046, 0x4300, {0xab, 0xb6, 0x3d, 0xd8, 0x10, 0xdd, 0x8b, 0x23} };

/*
 *	\brief	detect whether the Secure Boot feature is enabled
 *
 *	The function reads the global EFI variable "SecureBoot" and returns its (numeric) value.
 *	In case that the variable is not defined or cannot be read, then 0 is returned.
 *
 *	\return		value of the SecureBoot variable (or 0 in case of error)
 */
UINT8 cs_secureboot_enabled(void)
{
	/* return false if variable doesn't exist */
	UINT8 SecureBoot = 0;
	UINTN DataSize;
	EFI_STATUS error;
	EFI_GUID GV_GUID = EFI_GLOBAL_VARIABLE;

	DataSize = sizeof(SecureBoot);
	error = uefi_call_wrapper(RT->GetVariable, 5, L"SecureBoot", &GV_GUID, NULL,
				   &DataSize, &SecureBoot);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_WARN, L"unable to get global EFI variable \"SecureBoot\": %r\n", error));
		return 0;
	}
	CS_DEBUG((D_INFO, L"global EFI variable \"SecureBoot\" found: 0x%x\n", SecureBoot));

	return SecureBoot;
}

/*
 *	\brief	verify whether the given data are authentic in Secure Boot process
 *
 *	The function verifies the digital signature over the given data. This is done by the
 *	SHIM protocol: the boot loader "shim" is expected to be called in order to provide
 *	this protocol. Shim performs the signature verification using its own database of
 *	public keys, see http://mjg59.dreamwidth.org/19448.html
 *	That means that the use of the shim loader is the only way for the CipherShed loader
 *	to support Secure Boot. This presumes that the CipherShed loader itself as well as the
 *	CipherShed driver is correctly signed.
 *
 *	\param	ControllerHandle	handle of the controller
 *	\param	data				pointer to buffer containing the data to verify
 *	\param	size				size of data in buffer to verify
 *
 *	\return		the success state of the function
 */
BOOLEAN cs_secureboot_validate(IN EFI_HANDLE ControllerHandle, IN VOID *data, IN UINT32 size)
{
	EFI_STATUS error;
	EFI_GUID shimLockProtocol = SHIM_LOCK_GUID;
	struct grub_efi_shim_lock *shim_lock;

	error = uefi_call_wrapper(BS->HandleProtocol, 3, ControllerHandle, &shimLockProtocol,
			  (VOID **)&shim_lock);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_INFO, L"unable to get shim lock protocol: %r\n", error));
		return FALSE;
	}
	if (!shim_lock) {
		CS_DEBUG((D_ERROR, L"invalid get shim lock protocol handle\n"));
		return FALSE;
	}

	/* Attention: according to shim.h the verify() function seems to be NOT defined as
	 *    EFI_STATUS EFIAPI (*verify)(IN VOID *buffer, IN UINT32 size); but as:
	 *    EFI_STATUS        (*verify)(IN VOID *buffer, IN UINT32 size);
	 * hence the call of the verify() function does not need uefi_call_wrapper() */
	error = shim_lock->verify(data, size);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_WARN, L"signature verification failed: %r\n", error));
		return FALSE;
	}
	CS_DEBUG((D_INFO, L"signature verification succeeded\n"));
	return TRUE;
}

