/*  cs_driver.c - CipherShed EFI boot loader
 *  main file for the CipherShed EFI driver providing access to the encrypted partition
 *
 *	Copyright (c) 2015-2016  Falk Nedwal
 *
 *	Governed by the Apache 2.0 License the full text of which is contained in
 *	the file License.txt included in CipherShed binary and source code distribution
 *	packages.
 */


#include <efi.h>
#include <efilib.h>
#include <efibind.h>
/* The following header files are needed to define protocols that are not defined
 * in the GNU-EFI package. They are taken from the EDK2 project */
#include <edk2/DriverBinding.h>
#include <edk2/ComponentName.h>
#include <edk2/ComponentName2.h>
#include <edk2/BlockIo2.h>

#include "cs_common.h"
#include "cs_driver.h"
#include "cs_crypto.h"
#include "cs_debug.h"

struct _cs_device_path {
	FILEPATH_DEVICE_PATH value;
	CHAR8 __cs_path[CS_MAX_DRIVER_PATH_SIZE];	/* used to allocate memory, don't access this directly! */
};

EFI_DRIVER_ENTRY_POINT(CsDriverInstall)

/* These ones are not defined in gnu-efi yet */
EFI_GUID DriverBindingProtocol = EFI_DRIVER_BINDING_PROTOCOL_GUID;
EFI_GUID ComponentNameProtocol = EFI_COMPONENT_NAME_PROTOCOL_GUID;
EFI_GUID ComponentName2Protocol = EFI_COMPONENT_NAME2_PROTOCOL_GUID;
EFI_GUID BlockIo2Protocol = EFI_BLOCK_IO2_PROTOCOL_GUID;
/* this is a self generated GUID to identify that the CS driver is already connected */
extern EFI_GUID CsCallerIdGuid;


EFI_DRIVER_BINDING_PROTOCOL CsDriverBinding = {
		.Supported = CsDriverBindingSupported,
		.Start = CsDriverBindingStart,
		.Stop = CsDriverBindingStop,
		.Version = CS_DRIVER_BIND_VERSION,
		.ImageHandle = NULL,
		.DriverBindingHandle = NULL
};

EFI_COMPONENT_NAME_PROTOCOL CsComponentName = {
		.GetDriverName = CsGetDriverName,
		.GetControllerName = CsGetControllerName,
		.SupportedLanguages = (CHAR8 *) "eng"
};

EFI_COMPONENT_NAME2_PROTOCOL CsComponentName2 = {
		.GetDriverName = CsGetDriverName2,
		.GetControllerName = CsGetControllerName2,
		.SupportedLanguages = (CHAR8 *) "en"
};

/* the block IO protocol provided at the child device */
EFI_BLOCK_IO CsChildBlockIo = {
		.Revision = EFI_BLOCK_IO_INTERFACE_REVISION3,
		.Media = NULL,
		.Reset = CsReset,
		.ReadBlocks = CsChildReadBlocks,
		.WriteBlocks = CsChildWriteBlocks,
		.FlushBlocks = CsChildFlushBlocks
};

CHAR16 *DriverNameString = CS_DRIVER_NAME L" Version " WIDEN(STRINGIFY(CS_DRIVER_VERSION));
CHAR16 *ControllerNameString = CS_CONTROLLER_NAME L" Version " WIDEN(STRINGIFY(CS_DRIVER_VERSION));

/* identifiers to define a dedicated function of the BLOCK IO or BLOCK IO2 protocol */
enum cs_io_access_mode {
	CS_BLOCK_IO_READ,
	CS_BLOCK_IO_WRITE,
	CS_BLOCK_IO2_READ,
	CS_BLOCK_IO2_WRITE,
	CS_BLOCK_IO_LAST		/* only denotes the end of the enumeration */
};

/* 32-bit relative jump */
#pragma pack(push, 1)
struct JMP_REL
{
    UINT8  opcode;      /* E9 xxxxxxxx: JMP +5+xxxxxxxx */
    UINT32 operand;     /* relative destination address */
};
#pragma pack(pop)

static struct cs_driver_context {
	UINT16 factorMediaBlock;	/* ratio between media block size and crypto unit size
	 	 	 	 	 	 	 	   (the crypto unit size is always ENCRYPTION_DATA_UNIT_SIZE);
	 	 	 	 	 	 	 	   this shall support media block sizes bigger than
	 	 	 	 	 	 	 	   ENCRYPTION_DATA_UNIT_SIZE */
	struct {
		UINT32 blockIoInstalled:1;
		UINT32 createChildDevice:1;
	} status;
	EFI_BLOCK_IO *ConsumedBlockIo;
	EFI_BLOCK_IO2 *ConsumedBlockIo2;
	struct JMP_REL patchedBytes[CS_BLOCK_IO_LAST];
									/* original bytes of patched function before patching */
	struct JMP_REL patchBytes[CS_BLOCK_IO_LAST];
									/* modified bytes of patched function after patching */
	EFI_HANDLE ChildHandle;
	EFI_DEVICE_PATH *ChildDevicePath;
	UINT64 hiddenSectorOffset;		/* only used for hidden volume */
	UINT64 hiddenUnitOffset;		/* only used for hidden volume */
	struct cs_efi_option_data options;
	UINT64 EndSector;				/* options.StartSector + options.SectorCount */
} context;

EFI_STATUS EFIAPI CsReadBlocks(
		IN EFI_BLOCK_IO		              *This,
		IN UINT32                         MediaId,
		IN EFI_LBA                        Lba,
		IN UINTN                          BufferSize,
		OUT VOID                          *Buffer);
EFI_STATUS EFIAPI CsWriteBlocks(
		IN EFI_BLOCK_IO		              *This,
		IN UINT32                         MediaId,
		IN EFI_LBA                        Lba,
		IN UINTN                          BufferSize,
		IN VOID                           *Buffer);
EFI_STATUS EFIAPI CsReadBlocksEx(
		IN     EFI_BLOCK_IO2_PROTOCOL *This,
		IN     UINT32                 MediaId,
		IN     EFI_LBA                Lba,
		IN OUT EFI_BLOCK_IO2_TOKEN    *Token,
		IN     UINTN                  BufferSize,
		OUT    VOID                   *Buffer);
EFI_STATUS EFIAPI CsWriteBlocksEx(
		IN     EFI_BLOCK_IO2_PROTOCOL  *This,
		IN     UINT32                 MediaId,
		IN     EFI_LBA                Lba,
		IN OUT EFI_BLOCK_IO2_TOKEN    *Token,
		IN     UINTN                  BufferSize,
		IN     VOID                   *Buffer);


/*
 * \brief	encrypt a buffer (in-place)
 *
 * \param	lba			logical block number of the request
 * \param	BufferSize	Size of Buffer, must be a multiple of device block size.
 * \param	Buffer		A pointer to the destination buffer for the data. The caller is
 *						responsible for either having implicit or explicit ownership of the buffer.
 *
 * \return	the success state of the operation
 */
EFI_STATUS EFIAPI encryptBlocks(IN EFI_LBA lba, IN UINTN BufferSize, OUT VOID *Buffer) {

	UINT16 blockCount = BufferSize / ENCRYPTION_DATA_UNIT_SIZE;

	ASSERT(Buffer != NULL);

	CS_DEBUG((D_INFO, L"encrypt data (0x%x byte)\n", BufferSize));

	ASSERT(BufferSize % ENCRYPTION_DATA_UNIT_SIZE == 0);

	if (context.options.isHiddenVolume) {
		lba += context.hiddenUnitOffset;
	}

	while (blockCount-- > 0) {

		if ((lba >= context.options.StartSector) && (lba < context.EndSector)) {

			CS_DEBUG((D_INFO, L"encrypt block (0x%x)\n", lba));

			EncryptDataUnits(Buffer, &lba, 1 /* one block */,
					context.options.cipher.algo, context.options.cipher.mode,
					context.options.cipher.ks, context.options.cipher.ks2);
		}
		++lba;
		Buffer += ENCRYPTION_DATA_UNIT_SIZE;
	}

	return EFI_SUCCESS;
}

/*
 * \brief	decrypt a buffer (in-place)
 *
 * \param	lba			logical block number of the request
 * \param	BufferSize	Size of Buffer, must be a multiple of device block size.
 * \param	Buffer		A pointer to the destination buffer for the data. The caller is
 *						responsible for either having implicit or explicit ownership of the buffer.
 *
 * \return	the success state of the operation
 */
EFI_STATUS EFIAPI decryptBlocks(IN EFI_LBA lba, IN UINTN BufferSize, OUT VOID *Buffer) {

	UINT16 blockCount = BufferSize / ENCRYPTION_DATA_UNIT_SIZE;

    ASSERT(Buffer != NULL);

	CS_DEBUG((D_INFO, L"decrypt data (0x%x byte)\n", BufferSize));

	ASSERT(BufferSize % ENCRYPTION_DATA_UNIT_SIZE == 0);

	if (context.options.isHiddenVolume) {
		lba += context.hiddenUnitOffset;
	}

	while (blockCount-- > 0) {

		if ((context.options.isHiddenVolume) ||
			((lba >= context.options.StartSector) && (lba < context.EndSector))) {

			CS_DEBUG((D_INFO, L"decrypt block (0x%x)\n", lba));

			DecryptDataUnits (Buffer, &lba, 1 /* one block */,
					context.options.cipher.algo, context.options.cipher.mode,
					context.options.cipher.ks, context.options.cipher.ks2);
		}
		++lba;
		Buffer += ENCRYPTION_DATA_UNIT_SIZE;
	}

	return EFI_SUCCESS;
}

/*
 * \brief	Patch the code of the indicated BLOCK IO or BLOCK IO2 function.
 *
 * This function replaces the first few bytes of the indicated function with new values
 * taken from the system context (as defined using CsPatchCodeInit()).
 *
 * taken from the system context (as defined using CsPatchCodeInit()).
 */
static VOID CsPatchCode(
		IN enum cs_io_access_mode	mode
		) {

	switch (mode) {
	case CS_BLOCK_IO_READ:
		ASSERT(context.ConsumedBlockIo != NULL);
		CopyMem(context.ConsumedBlockIo->ReadBlocks, &context.patchBytes[CS_BLOCK_IO_READ],
				sizeof(context.patchBytes[CS_BLOCK_IO_READ]));
		break;
	case CS_BLOCK_IO_WRITE:
		ASSERT(context.ConsumedBlockIo != NULL);
		CopyMem(context.ConsumedBlockIo->WriteBlocks, &context.patchBytes[CS_BLOCK_IO_WRITE],
				sizeof(context.patchBytes[CS_BLOCK_IO_WRITE]));
		break;
	case CS_BLOCK_IO2_READ:
		ASSERT(context.ConsumedBlockIo2 != NULL);
		CopyMem(context.ConsumedBlockIo2->ReadBlocksEx, &context.patchBytes[CS_BLOCK_IO2_READ],
				sizeof(context.patchBytes[CS_BLOCK_IO2_READ]));
		break;
	case CS_BLOCK_IO2_WRITE:
		ASSERT(context.ConsumedBlockIo2 != NULL);
		CopyMem(context.ConsumedBlockIo2->WriteBlocksEx, &context.patchBytes[CS_BLOCK_IO2_WRITE],
				sizeof(context.patchBytes[CS_BLOCK_IO2_WRITE]));
		break;
	default:
		break;
	}
}

/*
 * \brief	Backup the original bytes of the given function.
 *
 * This function saves the first bytes of the given functionToPatch to the system context.
 * This is done to be able to restore them later. After this backup, these first bytes
 * can be replaced by a patch.
 * Also, the new bytes (the patch) are prepared in the context, but the replacement (patch)
 * of the BLOCK IO or BLOCK IO2 function is not performed by this function. The patch
 * implements a relative jump to the address of the given newFunction.
 * The function acts as a helper function for CsPatchCodeInit().
 *
 * \param[in]       mode				indicates which function to prepare and backup
 * \param[in]       functionToPatch	pointer to the (original) function to be patched
 * \param[in]       newFunction		pointer to the replacement of the original function
 */
static inline VOID _CsPatchCodeInit(
		IN enum cs_io_access_mode	mode,
		IN VOID *functionToPatch,
		IN VOID *newFunction
		) {
	ASSERT(functionToPatch != NULL);
	ASSERT(newFunction != NULL);

	CS_DEBUG((D_INFO, L"init patch code: mode: %x, function to patch: 0x%x, new address: 0x%x\n",
			mode, functionToPatch, newFunction));

	CopyMem(&context.patchedBytes[mode], functionToPatch, sizeof(context.patchedBytes[mode]));
	context.patchBytes[mode].opcode = 0xE9;	/* JMP */
	context.patchBytes[mode].operand = (UINT32)((UINT8 *)newFunction -
			((UINT8 *)functionToPatch + sizeof(struct JMP_REL)));
}

/*
 * \brief	Backup the original bytes of the indicated BLOCK IO or BLOCK IO2 function.
 *
 * This function saves the first bytes of the indicated BLOCK IO or BLOCK IO2 function
 * to the system context. This is done to be able to restore them later. After this backup,
 * these first bytes can be replaced by a patch.
 * Also, the new bytes (the patch) are prepared in the context, but the replacement (patch)
 * of the BLOCK IO or BLOCK IO2 function is not performed by this function.
 *
 * \param[in]       mode			indicates which function to prepare and backup
 */
static VOID CsPatchCodeInit(
		IN enum cs_io_access_mode	mode
		) {

	switch (mode) {
	case CS_BLOCK_IO_READ:
		ASSERT(context.ConsumedBlockIo != NULL);
		_CsPatchCodeInit(CS_BLOCK_IO_READ, context.ConsumedBlockIo->ReadBlocks, CsReadBlocks);
		break;
	case CS_BLOCK_IO_WRITE:
		ASSERT(context.ConsumedBlockIo != NULL);
		_CsPatchCodeInit(CS_BLOCK_IO_WRITE, context.ConsumedBlockIo->WriteBlocks, CsWriteBlocks);
		break;
	case CS_BLOCK_IO2_READ:
		ASSERT(context.ConsumedBlockIo2 != NULL);
		_CsPatchCodeInit(CS_BLOCK_IO2_READ, context.ConsumedBlockIo2->ReadBlocksEx, CsReadBlocksEx);
		break;
	case CS_BLOCK_IO2_WRITE:
		ASSERT(context.ConsumedBlockIo2 != NULL);
		_CsPatchCodeInit(CS_BLOCK_IO2_WRITE, context.ConsumedBlockIo2->WriteBlocksEx, CsWriteBlocksEx);
		break;
	default:
		break;
	}
}

/*
 * \brief	Reverse the patch of the indicated BLOCK IO or BLOCK IO2 function.
 *
 * This function replaces the patched bytes in the indicated BLOCK IO or BLOCK IO2
 * function with the original bytes in order to restore the original behavior of the
 * function.
 *
 * \param[in]       mode			indicates which function to restore
 */
static VOID CsUnpatchCode(
		IN enum cs_io_access_mode	mode
		) {

	switch (mode) {
	case CS_BLOCK_IO_READ:
		ASSERT(context.ConsumedBlockIo != NULL);
		CopyMem(context.ConsumedBlockIo->ReadBlocks, &context.patchedBytes[CS_BLOCK_IO_READ],
				sizeof(context.patchedBytes[CS_BLOCK_IO_READ]));
		break;
	case CS_BLOCK_IO_WRITE:
		ASSERT(context.ConsumedBlockIo != NULL);
		CopyMem(context.ConsumedBlockIo->WriteBlocks, &context.patchedBytes[CS_BLOCK_IO_WRITE],
				sizeof(context.patchedBytes[CS_BLOCK_IO_WRITE]));
		break;
	case CS_BLOCK_IO2_READ:
		ASSERT(context.ConsumedBlockIo2 != NULL);
		CopyMem(context.ConsumedBlockIo2->ReadBlocksEx, &context.patchedBytes[CS_BLOCK_IO2_READ],
				sizeof(context.patchedBytes[CS_BLOCK_IO2_READ]));
		break;
	case CS_BLOCK_IO2_WRITE:
		ASSERT(context.ConsumedBlockIo2 != NULL);
		CopyMem(context.ConsumedBlockIo2->WriteBlocksEx, &context.patchedBytes[CS_BLOCK_IO2_WRITE],
				sizeof(context.patchedBytes[CS_BLOCK_IO2_WRITE]));
		break;
	default:
		break;
	}
}

/*
 * \brief	Decide whether the media need to be handled by the crypto driver.
 *
 * This function decides based on the given parameters whether the defined media
 * needs to be handled by the crypto driver or the original driver must be used.
 *
 * \param[in]       Media	pointer to the BLOCK IO media structure
 * \param[in]       MediaId	value of the given media ID as passed to the BLOCK IO
 * 	  	  	  	  	protocol interface
 *
 * \return TRUE				indicates that the device has to be handled by the
 *	 	  	  	  	  	  	crypto driver because it might be encrypted
 * \return FALSE			indicates that the device is not encrypted and
 *	  	  	  	  	  	  	does not need to be handled by the crypto driver
 */
static BOOLEAN CsCheckForCorrectDevice(
		IN EFI_BLOCK_IO_MEDIA			*Media,
		IN UINT32						MediaId
	) {

	CS_DEBUG((D_INFO, L"check device: logical partition: %x, removable media: %x, present: %x, ID: %x",
			Media->LogicalPartition, Media->RemovableMedia, Media->MediaPresent, MediaId));

	if (Media->LogicalPartition == TRUE &&
		Media->RemovableMedia == FALSE &&
		Media->MediaPresent == TRUE &&
		MediaId == context.ConsumedBlockIo->Media->MediaId &&
		Media->LastBlock == context.ConsumedBlockIo->Media->LastBlock) {

		CS_DEBUG((D_INFO, L", Ok.\n"));
		return TRUE;
	}

	CS_DEBUG((D_INFO, L", skip\n"));

	return FALSE;
}

/*
 * \brief	Read or write bytes to disk (without any encryption/decryption).
 *
 * This function calls the original (unpatched) functions of the BLOCK IO or
 * BLOCK IO2 protocol in order to read or write data from or to the given device.
 * This is implemented by reversing the patch of the disk access function, then
 * calling the function, then patching it again.
 *
 * \param[in]       mode			indicates which function to call (read or write)
 * \param[in]       _BlockIo		pointer to the BLOCK IO or BLOCK IO2 protocol
 * \param[in]       MediaId			Id of the media, changes every time the media is
 *                              	replaced.
 * \param[in]       Lba				The starting Logical Block Address to access
 * \param[in, out]  Token			A pointer to the token associated with the transaction.
 * \param[in]       BufferSize		Size of Buffer, must be a multiple of device block size.
 * \param[out]      Buffer			A pointer to the destination buffer for the data. The
 *                               	caller is responsible for either having implicit or
 *                               	explicit ownership of the buffer.
 *
 * \return EFI_SUCCESS           	The read request was queued if Token->Event is
 *                                  not NULL.The data was read correctly from the
 *                                  device if the Token->Event is NULL.
 * \return EFI_DEVICE_ERROR      	The device reported an error while performing
 *                                  the read.
 * \return EFI_NO_MEDIA          	There is no media in the device.
 * \return EFI_MEDIA_CHANGED     	The MediaId is not for the current media.
 * \return EFI_BAD_BUFFER_SIZE   	The BufferSize parameter is not a multiple of the
 *                                  intrinsic block size of the device.
 * \return EFI_INVALID_PARAMETER 	The read request contains LBAs that are not valid,
 *                                  or the buffer is not on proper alignment.
 * \return EFI_OUT_OF_RESOURCES  	The request could not be completed due to a lack
 *                                  of resources.
**/
static EFI_STATUS CsParentReadWriteBlocks(
		IN enum cs_io_access_mode		mode,
		IN VOID				            *_BlockIo,
		IN UINT32						MediaId,
		IN EFI_LBA						Lba,
		IN OUT EFI_BLOCK_IO2_TOKEN		*Token,
		IN UINTN						BufferSize,
		IN OUT VOID						*Buffer
	) {

	EFI_STATUS error = EFI_SUCCESS;
	EFI_BLOCK_IO *BlockIo = (EFI_BLOCK_IO *)_BlockIo;
	EFI_BLOCK_IO2 *BlockIo2 = (EFI_BLOCK_IO2 *)_BlockIo;

	ASSERT(_BlockIo != NULL);

	CsUnpatchCode(mode);

	switch (mode) {
	case CS_BLOCK_IO_READ:
		error = uefi_call_wrapper(BlockIo->ReadBlocks, 5, BlockIo, MediaId, Lba, BufferSize, Buffer);
		break;
	case CS_BLOCK_IO_WRITE:
		error = uefi_call_wrapper(BlockIo->WriteBlocks, 5, BlockIo, MediaId, Lba, BufferSize, Buffer);
		break;
	case CS_BLOCK_IO2_READ:
		error = uefi_call_wrapper(BlockIo2->ReadBlocksEx, 6, BlockIo2, MediaId, Lba, Token, BufferSize, Buffer);
		break;
	case CS_BLOCK_IO2_WRITE:
		error = uefi_call_wrapper(BlockIo2->WriteBlocksEx, 6, BlockIo2, MediaId, Lba, Token, BufferSize, Buffer);
		break;
	default:
		error = EFI_INVALID_PARAMETER;
		break;
	}
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"CsParentReadWriteBlocks(): access to disk failed: %r\n", error));
	}

	CsPatchCode(mode);

	return error;
}

/*
 * \brief	Read BufferSize bytes from Lba into Buffer.
 *
 * This function is the patched version of the corresponding BLOCK IO protocol function.
 * It handles all requests of the original BLOCK IO driver and has to distinguish between
 * accesses to the encrypted device and accesses to other devices. For the accesses to
 * other devices, the original (unpatched) version of this function is called, otherwise
 * the content is encrypted/decrypted.
 *
 * \param  This       Indicates a pointer to the calling context.
 * \param  MediaId    Id of the media, changes every time the media is replaced.
 * \param  Lba        The starting Logical Block Address to read from
 * \param  BufferSize Size of Buffer, must be a multiple of device block size.
 * \param  Buffer     A pointer to the destination buffer for the data. The caller is
 *                    responsible for either having implicit or explicit ownership of the buffer.
 *
 * \return EFI_SUCCESS           The data was read correctly from the device.
 * \return EFI_DEVICE_ERROR      The device reported an error while performing the read.
 * \return EFI_NO_MEDIA          There is no media in the device.
 * \return EFI_MEDIA_CHANGED     The MediaId does not matched the current device.
 * \return EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
 * \return EFI_INVALID_PARAMETER The read request contains LBAs that are not valid,
 *                               or the buffer is not on proper alignment.
 */
EFI_STATUS EFIAPI CsReadBlocks(
		IN EFI_BLOCK_IO		              *This,
		IN UINT32                         MediaId,
		IN EFI_LBA                        Lba,
		IN UINTN                          BufferSize,
		OUT VOID                          *Buffer
	) {
	EFI_STATUS error;
	EFI_LBA block;
	BOOLEAN needEncryption = CsCheckForCorrectDevice(This->Media, MediaId);

	CS_DEBUG((D_INFO, L"CsReadBlocks(ID=0x%x, ", MediaId));
	CS_DEBUG((D_INFO, L"LBA=0x%x, ", Lba));
	CS_DEBUG((D_INFO, L"Size=0x%x, ", BufferSize));
	CS_DEBUG((D_INFO, L"Enc=%x) called.\n", needEncryption));

	if (needEncryption) {
		block = Lba * context.factorMediaBlock;	/* block regards to the encryption/decryption unit,
													Lba regards to the media unit */
		if (context.options.isHiddenVolume) {
			Lba += context.hiddenSectorOffset;
		}
	}

	/* read data from disk */
	error = CsParentReadWriteBlocks(CS_BLOCK_IO_READ, This, MediaId, Lba, NULL, BufferSize, Buffer);
	if (EFI_ERROR(error)) {
		return error;
	}

	if (needEncryption == FALSE) {
		return EFI_SUCCESS;	/* finished: no decryption needed */
	}

	error = decryptBlocks(block, BufferSize, Buffer);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"decryptBlocks() failed (%r)\n", error));
	}

	return EFI_SUCCESS;
}

/*
 * \brief	Write BufferSize bytes from Lba into Buffer.
 *
 * This function is the patched version of the corresponding BLOCK IO protocol function.
 * It handles all requests of the original BLOCK IO driver and has to distinguish between
 * accesses to the encrypted device and accesses to other devices. For the accesses to
 * other devices, the original (unpatched) version of this function is called, otherwise
 * the content is encrypted/decrypted.
 *
 * \param  This       Indicates a pointer to the calling context.
 * \param  MediaId    The media ID that the write request is for.
 * \param  Lba        The starting logical block address to be written. The caller is
 *                    responsible for writing to only legitimate locations.
 * \param  BufferSize Size of Buffer, must be a multiple of device block size.
 * \param  Buffer     A pointer to the source buffer for the data.
 *
 * \return EFI_SUCCESS           The data was written correctly to the device.
 * \return EFI_WRITE_PROTECTED   The device can not be written to.
 * \return EFI_DEVICE_ERROR      The device reported an error while performing the write.
 * \return EFI_NO_MEDIA          There is no media in the device.
 * \return EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
 * \return EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
 * \return EFI_INVALID_PARAMETER The write request contains LBAs that are not valid,
 *                               or the buffer is not on proper alignment.
 */
EFI_STATUS EFIAPI CsWriteBlocks(
		IN EFI_BLOCK_IO		              *This,
		IN UINT32                         MediaId,
		IN EFI_LBA                        Lba,
		IN UINTN                          BufferSize,
		IN VOID                           *Buffer
	) {

	EFI_STATUS error;
	EFI_LBA block;
	BOOLEAN needEncryption = CsCheckForCorrectDevice(This->Media, MediaId);

	CS_DEBUG((D_INFO, L"CsWriteBlocks(ID=0x%x, ", MediaId));
	CS_DEBUG((D_INFO, L"LBA=0x%x, ", Lba));
	CS_DEBUG((D_INFO, L"Size=0x%x, ", BufferSize));
	CS_DEBUG((D_INFO, L"Enc=%x) called.\n", needEncryption));

	if (needEncryption) {

		block = Lba * context.factorMediaBlock;	/* block regards to the encryption/decryption unit,
															   Lba regards to the media unit */
		error = encryptBlocks(block, BufferSize, Buffer);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"encryptBlocks() failed (%r)\n", error));
			return error;
		}
		if (context.options.isHiddenVolume) {
			Lba += context.hiddenSectorOffset;
		}
	}

	error = CsParentReadWriteBlocks(CS_BLOCK_IO_WRITE, This, MediaId, Lba, NULL, BufferSize, Buffer);

	if (needEncryption) {
		/* to recover the original buffer content: */
		error = decryptBlocks(block, BufferSize, Buffer);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"decryptBlocks() failed (%r)\n", error));
		}
	}

	return error;
}

/*
 * \brief	Read BufferSize bytes from Lba into Buffer.
 *
 * This function reads the requested number of blocks from the device. All the
 * blocks are read, or an error is returned.
 * If EFI_DEVICE_ERROR, EFI_NO_MEDIA,_or EFI_MEDIA_CHANGED is returned and
 * non-blocking I/O is being used, the Event associated with this request will
 * not be signaled.
 *
 * This function is the patched version of the corresponding BLOCK IO2 protocol function.
 * It handles all requests of the original BLOCK IO2 driver and has to distinguish between
 * accesses to the encrypted device and accesses to other devices. For the accesses to
 * other devices, the original (unpatched) version of this function is called, otherwise
 * the content is encrypted/decrypted.
 *
 * \param[in]       This       Indicates a pointer to the calling context.
 * \param[in]       MediaId    Id of the media, changes every time the media is
 *                             replaced.
 * \param[in]       Lba        The starting Logical Block Address to read from.
 * \param[in, out]  Token            A pointer to the token associated with the transaction.
 * \param[in]       BufferSize Size of Buffer, must be a multiple of device block size.
 * \param[out]      Buffer     A pointer to the destination buffer for the data. The
 *                             caller is responsible for either having implicit or
 *                             explicit ownership of the buffer.
 *
 * \return EFI_SUCCESS           The read request was queued if Token->Event is
 *                               not NULL.The data was read correctly from the
 *                               device if the Token->Event is NULL.
 * \return EFI_DEVICE_ERROR      The device reported an error while performing
 *                               the read.
 * \return EFI_NO_MEDIA          There is no media in the device.
 * \return EFI_MEDIA_CHANGED     The MediaId is not for the current media.
 * \return EFI_BAD_BUFFER_SIZE   The BufferSize parameter is not a multiple of the
 *                               intrinsic block size of the device.
 * \return EFI_INVALID_PARAMETER The read request contains LBAs that are not valid,
 *                               or the buffer is not on proper alignment.
 * \return EFI_OUT_OF_RESOURCES  The request could not be completed due to a lack
 *                               of resources.
 */
EFI_STATUS EFIAPI CsReadBlocksEx(
		IN     EFI_BLOCK_IO2_PROTOCOL *This,
		IN     UINT32                 MediaId,
		IN     EFI_LBA                Lba,
		IN OUT EFI_BLOCK_IO2_TOKEN    *Token,
		IN     UINTN                  BufferSize,
		OUT    VOID                   *Buffer
	) {
	EFI_STATUS error;
	EFI_LBA block;
	BOOLEAN needEncryption = CsCheckForCorrectDevice(This->Media, MediaId);

	CS_DEBUG((D_INFO, L"CsReadBlocksEx(ID=0x%x, ", MediaId));
	CS_DEBUG((D_INFO, L"LBA=0x%x, ", Lba));
	CS_DEBUG((D_INFO, L"Size=0x%x, ", BufferSize));
	CS_DEBUG((D_INFO, L"Enc=%x) called.\n", needEncryption));

	if (needEncryption == FALSE) {
		return CsParentReadWriteBlocks(CS_BLOCK_IO2_READ, This, MediaId, Lba, Token, BufferSize, Buffer);
	}

	/* blocking access!! */
	block = Lba * context.factorMediaBlock;	/* block regards to the encryption/decryption unit,
													Lba regards to the media unit */
	if (context.options.isHiddenVolume) {
		Lba += context.hiddenSectorOffset;
	}

	/* read data from disk */
	error = CsParentReadWriteBlocks(CS_BLOCK_IO_READ, This, MediaId, Lba, NULL, BufferSize, Buffer);
	if (!EFI_ERROR(error)) {
		error = decryptBlocks(block, BufferSize, Buffer);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"decryptBlocks() failed (%r)\n", error));
		}
	}

	if (Token) {
		if (Token->Event) {
			Token->TransactionStatus = error;
			BS->SignalEvent(Token->Event);
		}
	}

	return error;
}

/*
 * \brief	Write BufferSize bytes from Lba into Buffer.
 *
 * This function writes the requested number of blocks to the device. All blocks
 * are written, or an error is returned.If EFI_DEVICE_ERROR, EFI_NO_MEDIA,
 * EFI_WRITE_PROTECTED or EFI_MEDIA_CHANGED is returned and non-blocking I/O is
 * being used, the Event associated with this request will not be signaled.
 *
 * This function is the patched version of the corresponding BLOCK IO2 protocol function.
 * It handles all requests of the original BLOCK IO2 driver and has to distinguish between
 * accesses to the encrypted device and accesses to other devices. For the accesses to
 * other devices, the original (unpatched) version of this function is called, otherwise
 * the content is encrypted/decrypted.
 *
 * \param[in]       This       Indicates a pointer to the calling context.
 * \param[in]       MediaId    The media ID that the write request is for.
 * \param[in]       Lba        The starting logical block address to be written. The
 *                             caller is responsible for writing to only legitimate
 *                             locations.
 * \param[in, out]  Token      A pointer to the token associated with the transaction.
 * \param[in]       BufferSize Size of Buffer, must be a multiple of device block size.
 * \param[in]       Buffer     A pointer to the source buffer for the data.
 *
 * \return EFI_SUCCESS           The write request was queued if Event is not NULL.
 *                               The data was written correctly to the device if
 *                               the Event is NULL.
 * \return EFI_WRITE_PROTECTED   The device can not be written to.
 * \return EFI_NO_MEDIA          There is no media in the device.
 * \return EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
 * \return EFI_DEVICE_ERROR      The device reported an error while performing the write.
 * \return EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
 * \return EFI_INVALID_PARAMETER The write request contains LBAs that are not valid,
 *                               or the buffer is not on proper alignment.
 * \return EFI_OUT_OF_RESOURCES  The request could not be completed due to a lack
 *                               of resources.
 */
EFI_STATUS EFIAPI CsWriteBlocksEx(
		IN     EFI_BLOCK_IO2_PROTOCOL  *This,
		IN     UINT32                 MediaId,
		IN     EFI_LBA                Lba,
		IN OUT EFI_BLOCK_IO2_TOKEN    *Token,
		IN     UINTN                  BufferSize,
		IN     VOID                   *Buffer
	) {
	EFI_STATUS error;
	EFI_LBA block;
	BOOLEAN needEncryption = CsCheckForCorrectDevice(This->Media, MediaId);

	CS_DEBUG((D_INFO, L"CsWriteBlocksEx(ID=0x%x, ", MediaId));
	CS_DEBUG((D_INFO, L"LBA=0x%x, ", Lba));
	CS_DEBUG((D_INFO, L"Size=0x%x, ", BufferSize));
	CS_DEBUG((D_INFO, L"Enc=%x) called.\n", needEncryption));

	if (needEncryption == FALSE) {
		return CsParentReadWriteBlocks(CS_BLOCK_IO2_WRITE, This, MediaId, Lba, Token, BufferSize, Buffer);
	}

	/* blocking access!! */
	block = Lba * context.factorMediaBlock;	/* block regards to the encryption/decryption unit,
														   Lba regards to the media unit */
	error = encryptBlocks(block, BufferSize, Buffer);
	if (!EFI_ERROR(error)) {
		if (context.options.isHiddenVolume) {
			Lba += context.hiddenSectorOffset;
		}
		error = CsParentReadWriteBlocks(CS_BLOCK_IO_WRITE, This, MediaId, Lba, NULL, BufferSize, Buffer);
		if (!EFI_ERROR(error)) {
			/* to recover the original buffer content: */
			error = decryptBlocks(block, BufferSize, Buffer);
			if (EFI_ERROR(error)) {
				CS_DEBUG((D_ERROR, L"decryptBlocks() failed (%r)\n", error));
			}
		}
	}

	if (Token) {
		if (Token->Event) {
			Token->TransactionStatus = error;
			BS->SignalEvent(Token->Event);
		}
	}

	return error;
}


/*
 * \brief	Test to see if this driver supports ControllerHandle.
 *
 * This service is called by the EFI boot service ConnectController(). In order
 * to make drivers as small as possible, there are a few calling restrictions for
 * this service. ConnectController() must follow these calling restrictions.
 * If any other agent wishes to call Supported() it must also follow these
 * calling restrictions.
 *
 * \param  This                Protocol instance pointer.
 * \param  Controller          Handle of device to test
 * \param  RemainingDevicePath Optional parameter use to pick a specific child
 *                             device to start.
 *
 * \return EFI_SUCCESS         This driver supports this device
 * \return EFI_ALREADY_STARTED This driver is already running on this device
 * \return other               This driver does not support this device
 */
EFI_STATUS EFIAPI CsDriverBindingSupported (
		IN EFI_DRIVER_BINDING_PROTOCOL  *This,
		IN EFI_HANDLE                   Controller,
		IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
	) {
	EFI_STATUS error;
	EFI_BLOCK_IO *BlockIo;

    ASSERT(This != NULL);

	CS_DEBUG((D_INFO, L"CsDriverBindingSupported(0x%x) started.\n", Controller));

	/* Don't handle this unless we can get exclusive access to BlockIO through it */
	error = uefi_call_wrapper(BS->OpenProtocol, 6, Controller, &BlockIoProtocol, (VOID **) &BlockIo,
			This->DriverBindingHandle, Controller, EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_WARN, L"Unable to open BlockIO Protocol: %r\n", error));
		return error;
	}

	/* The whole concept of BindingSupported is to hint at what we may
	* actually support, but not check if the target is valid or
	* initialize anything, so we must close all protocols we opened.
	*/
	error = uefi_call_wrapper(BS->CloseProtocol, 4, Controller, &BlockIoProtocol,
			This->DriverBindingHandle, Controller);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_WARN, L"CloseProtocol(BlockIoProtocol) returned: %r\n", error));
	}

	if (!EFI_ERROR(error)) {
		/* check whether the driver is already connected by opening the CsCallerIdGuid protocol,
		 * if this succeeds, then the start() function exits */
		if (is_cs_child_device(This->DriverBindingHandle, Controller)) {
			CS_DEBUG((D_INFO, L"driver is already connected, exiting...\n"));
			error = EFI_ALREADY_STARTED;
		} else {
			error = EFI_SUCCESS;
		}
	}

	if (!EFI_ERROR(error)) {
		/* check whether the driver is already connected to another controller by checking the patch,
		 * if this succeeds, then the start() function exits */
		if (context.patchBytes[CS_BLOCK_IO_READ].opcode != 0) {
			CS_DEBUG((D_INFO, L"driver is already connected to another device, exiting...\n"));
			error = EFI_ALREADY_STARTED;
		} else {
			error = EFI_SUCCESS;
		}
	}

	return error;
}

/*
 * \brief	uninstall the provided protocol interfaces to the given controller handle
 *
 * This function tries to uninstall the BLOCK_IO protocol
 * (if this protocol is provided) from the given controller handle.
 *
 * \param  ControllerHandle		controller handle
 */
static void uninstall_provided_protocols(IN EFI_HANDLE ControllerHandle) {

	ASSERT(ControllerHandle != NULL);

	/* uninstall the provided protocol interfaces */
	if (context.status.blockIoInstalled) {
		LibUninstallProtocolInterfaces(ControllerHandle,
				&BlockIoProtocol, &CsChildBlockIo,
				&CsCallerIdGuid, NULL /* no data required */,
				NULL);
		context.status.blockIoInstalled = 0;
	}
	if (context.ChildDevicePath) {
		LibUninstallProtocolInterfaces(ControllerHandle,
				&DevicePathProtocol, context.ChildDevicePath,
				NULL);
		FreePool(context.ChildDevicePath);
		context.ChildDevicePath = NULL;
	}
	LibUninstallProtocolInterfaces(ControllerHandle,
			&ComponentNameProtocol, &CsComponentName,
			&ComponentName2Protocol, &CsComponentName2,
			NULL);
}

/*
 * \brief	install the provided protocol interfaces to the given controller handle
 *
 * This function tries to install the BLOCK_IO protocol
 * (if this protocol is consumed) to the given controller handle. Also, the function
 * installs the devicePathProtocol interface, if DevicePath is not NULL.
 *
 * \param  ControllerHandle		controller handle
 * \param  DevicePath			device path of the handle
 *
 * \return	the success state of the function
 */
static EFI_STATUS install_provided_protocols(IN EFI_HANDLE *pControllerHandle, IN EFI_DEVICE_PATH *DevicePath) {
	EFI_STATUS error;

	CsChildBlockIo.Media = context.ConsumedBlockIo->Media;
	error = LibInstallProtocolInterfaces(pControllerHandle,
			&BlockIoProtocol, &CsChildBlockIo,
			&ComponentNameProtocol, &CsComponentName,
			&ComponentName2Protocol, &CsComponentName2,
			&CsCallerIdGuid, NULL /* no data required */,
			(DevicePath ? &DevicePathProtocol : NULL), DevicePath,
			NULL);
	if (!EFI_ERROR(error)) {
		context.status.blockIoInstalled = 1;
	}
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"error while installation of provided protocols: %r\n", error));
	} else {
		context.factorMediaBlock = CsChildBlockIo.Media->BlockSize / ENCRYPTION_DATA_UNIT_SIZE;

		CS_DEBUG((D_INFO, L"media block size is 0x%x\n", CsChildBlockIo.Media->BlockSize));
		CS_DEBUG((D_INFO, L"removable media/logical partition/ID: %x/%x/%x\n",
				CsChildBlockIo.Media->RemovableMedia, CsChildBlockIo.Media->LogicalPartition,
				CsChildBlockIo.Media->MediaId));
		CS_DEBUG((D_INFO, L"first/last LBA 0x%lx/0x%lx\n",
				CsChildBlockIo.Media->LowestAlignedLba, CsChildBlockIo.Media->LastBlock));
		ASSERT(CsChildBlockIo.Media->BlockSize >= ENCRYPTION_DATA_UNIT_SIZE);
		ASSERT(CsChildBlockIo.Media->BlockSize % ENCRYPTION_DATA_UNIT_SIZE == 0);
	}

	return error;
}

/*
 * \brief	close the consumed protocols (BlockIO/BlockIO2) at the given controller
 *
 * This function close the protocols that are consumed by the driver at the given controller.
 *
 * \param  This              Protocol instance pointer
 * \param  ControllerHandle	parent controller handle
 *
 * \return	the success state of the function
 */
static EFI_STATUS close_consumed_protocols(IN EFI_DRIVER_BINDING_PROTOCOL *This,
		IN EFI_HANDLE ControllerHandle) {
	EFI_STATUS error;

	ASSERT(This != NULL);

	if (context.ConsumedBlockIo) {
		error = uefi_call_wrapper(BS->CloseProtocol, 4, ControllerHandle, &BlockIoProtocol,
				This->DriverBindingHandle, ControllerHandle);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_INFO, L"CloseProtocol(BlockIo) returned: %r\n", error));
		}
		context.ConsumedBlockIo = NULL;
	}
	if (context.ConsumedBlockIo2) {
		error = uefi_call_wrapper(BS->CloseProtocol, 4, ControllerHandle, &BlockIo2Protocol,
				This->DriverBindingHandle, ControllerHandle);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_INFO, L"CloseProtocol(BlockIo2) returned: %r\n", error));
		}
		context.ConsumedBlockIo2 = NULL;
	}

	return error;
}

/*
 * \brief	open the consumed protocols (BlockIO/BlockIO2) at the given controller
 *
 * This function opens the protocols that are consumed by the driver at the given controller.
 * Depending on the given flag, the open mode changes: when by_child is set, then the protocols
 * are opened with the child flag and the child handle is given as argument instead of the parent
 * handle. This is used to create the link between the parent and the child handle (see
 * Driver Writers Guide, example 31).
 *
 * \param  This              Protocol instance pointer
 * \param  ControllerHandle	parent controller handle
 * \param  by_client			switch whether the protocol is opened in child mode
 *
 * \return	the success state of the function
 */
static EFI_STATUS open_consumed_protocols(IN EFI_DRIVER_BINDING_PROTOCOL *This,
		IN EFI_HANDLE ControllerHandle, IN BOOLEAN by_child) {
	EFI_STATUS error;
	EFI_HANDLE handle = (by_child ? context.ChildHandle : ControllerHandle);
	UINTN mode = (by_child ? EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER :
			EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE);

    ASSERT(This != NULL);

	/* open the protocols that are consumed by the driver: BlockIoProtocol and BlockIo2Protocol */
	error = uefi_call_wrapper(BS->OpenProtocol, 6, ControllerHandle, &BlockIoProtocol,
			(VOID **) &context.ConsumedBlockIo, This->DriverBindingHandle, handle, mode);

	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"Unable to open BlockIO Protocol (%s): %r\n",
				(by_child ? L"by child" : L"parent"), error));
		if (!by_child) {
			context.ConsumedBlockIo = NULL;
		}
		return error;
	}
	if ((!by_child) || (context.ConsumedBlockIo2))
		{
			EFI_STATUS error2;
			/* BlockIo2Protocol is optional, no error if not available... */
			error2 = uefi_call_wrapper(BS->OpenProtocol, 6, ControllerHandle, &BlockIo2Protocol,
					(VOID **) &context.ConsumedBlockIo2, This->DriverBindingHandle, handle, mode);
			if (EFI_ERROR(error2)) {
				CS_DEBUG((D_WARN, L"Unable to open BlockIO2 Protocol (%s): %r\n",
						(by_child ? L"by child" : L"parent"), error2));
				if (!by_child) {
					context.ConsumedBlockIo2 = NULL;
				} else {
					error = error2;
				}
			}
		}

	return error;
}

/*
 * \brief	create a new child device handle and install BlockIO protocol at it.
 *
 * This function takes the given device handle and creates a child handle of it.
 * The device path of the given handle is extended by the string CS_CHILD_PATH_EXTENSION
 * which becomes the device path of the new child handle. The child handle is
 * stored in the system context.
 *
 * \param  ControllerHandle		parent controller handle
 *
 * \return	the success state of the function
 */
static EFI_STATUS add_new_child_handle(IN EFI_HANDLE ControllerHandle) {
	EFI_STATUS error;
	EFI_DEVICE_PATH *ParentDevicePath;
	struct _cs_device_path path;

	ASSERT(StrLen(CS_CHILD_PATH_EXTENSION) * sizeof(CHAR16) < sizeof(path.__cs_path));

	ZeroMem (&path, sizeof(path));
	path.value.Header.Type = MEDIA_DEVICE_PATH;
	path.value.Header.SubType = MEDIA_FILEPATH_DP;
	StrCpy(&path.value.PathName[0], CS_CHILD_PATH_EXTENSION);
	SetDevicePathNodeLength(&path.value.Header, sizeof(path.value) +
			(sizeof (CHAR16) * StrLen(CS_CHILD_PATH_EXTENSION)));

	/* get ParentDevicePath */
	error = uefi_call_wrapper(BS->HandleProtocol, 3, ControllerHandle, &DevicePathProtocol,
			  (VOID **) &ParentDevicePath);
	if (!EFI_ERROR(error)) {

#if 0	// seems not to work in QEMU environment
		CS_DEBUG((D_INFO, L"ParentDevicePath: %s\n", DevicePathToStr(ParentDevicePath)));
#endif

		context.ChildDevicePath = AppendDevicePathNode(ParentDevicePath, (EFI_DEVICE_PATH *) &path.value);
		if (context.ChildDevicePath == NULL) {
		    error = EFI_OUT_OF_RESOURCES;
		}
#if 0	// seems not to work in QEMU environment
		CS_DEBUG((D_INFO, L"ChildDevicePath: %s\n", DevicePathToStr(context.ChildDevicePath)));
#endif
		if (!EFI_ERROR(error)) {
			/* Add Device Path Protocol and Block I/O Protocol to a new handle */
			context.ChildHandle = NULL;
			/*******************************************************************/
			error = install_provided_protocols(&context.ChildHandle, context.ChildDevicePath);
			/*******************************************************************/
		}
	} else {
		CS_DEBUG((D_ERROR, L"Unable to create the child handle: %r\n", error));
	}

	return error;
}

/*
 * \brief	destroy the child handle that was created by add_new_child_handle().
 *
 * \return	the success state of the function
 */
static EFI_STATUS delete_child_handle(void) {

	if (context.ChildHandle) {
		uninstall_provided_protocols(context.ChildHandle);
		return EFI_SUCCESS;
	} else {
		return EFI_DEVICE_ERROR;
	}
}

/*
 * \brief	Start this driver on ControllerHandle.
 *
 * This service is called by the EFI boot service ConnectController(). In order
 * to make drivers as small as possible, there are a few calling restrictions for
 * this service. ConnectController() must follow these calling restrictions. If
 * any other agent wishes to call Start() it must also follow these calling
 * restrictions.
 *
 * \param  This                 Protocol instance pointer.
 * \param  ControllerHandle     Handle of device to bind driver to
 * \param  RemainingDevicePath  Optional parameter use to pick a specific child
 *                              device to start -> not used in this driver
 *
 * \return EFI_SUCCESS          This driver is added to ControllerHandle
 * \return EFI_ALREADY_STARTED  This driver is already running on ControllerHandle
 * \return other                This driver does not support this device
 */
EFI_STATUS EFIAPI CsDriverBindingStart (
		IN EFI_DRIVER_BINDING_PROTOCOL  *This,
		IN EFI_HANDLE                   ControllerHandle,
		IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
	) {

	EFI_STATUS error;

	CS_DEBUG((D_INFO, L"CsDriverBindingStart(0x%x) started.\n", ControllerHandle));

	/* exclusively open the protocols that are consumed by the driver: BlockIo Protocol */
	error = open_consumed_protocols(This, ControllerHandle, FALSE);
	if (EFI_ERROR(error)) {
		return error;
	}

	if (context.options.createChildDevice) {

		/* see Driver Writers Guide, example 31 */
		error = add_new_child_handle(ControllerHandle);
		if (EFI_ERROR(error)) {
			goto fail;
		}

		/* and now open again for the client handle to record the parent-child relationship */
		error = open_consumed_protocols(This, ControllerHandle, TRUE);
		if (EFI_ERROR(error)) {
			goto fail;
		}
	}

	/* initialize driver data, in case of error: return EFI_OUT_OF_RESOURCES */
	if (context.ConsumedBlockIo) {
		CsPatchCodeInit(CS_BLOCK_IO_READ);
		CsPatchCodeInit(CS_BLOCK_IO_WRITE);
		CsPatchCode(CS_BLOCK_IO_READ);
		CsPatchCode(CS_BLOCK_IO_WRITE);
		CS_DEBUG((D_INFO, L"CsDriverBindingStart() patch of BLOCK_IO code succeeded.\n"));
	}
	if (context.ConsumedBlockIo2) {
		CsPatchCodeInit(CS_BLOCK_IO2_READ);
		CsPatchCodeInit(CS_BLOCK_IO2_WRITE);
		CsPatchCode(CS_BLOCK_IO2_READ);
		CsPatchCode(CS_BLOCK_IO2_WRITE);
		CS_DEBUG((D_INFO, L"CsDriverBindingStart() patch of BLOCK_IO2 code succeeded.\n"));
	}

	CS_DEBUG((D_INFO, L"CsDriverBindingStart() succeeded.\n"));

	return EFI_SUCCESS;

fail:
	if (context.options.createChildDevice) {
		delete_child_handle();
	}
	close_consumed_protocols(This, ControllerHandle);

	return EFI_DEVICE_ERROR;
}

/*
 * \brief	Stop this driver on ControllerHandle.
 *
 * This service is called by the EFI boot service DisconnectController().
 * In order to make drivers as small as possible, there are a few calling
 * restrictions for this service. DisconnectController() must follow these
 * calling restrictions. If any other agent wishes to call Stop() it must
 * also follow these calling restrictions.
 *
 * \param  This              Protocol instance pointer.
 * \param  ControllerHandle  Handle of device to stop driver on
 * \param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
 *                           children is zero stop the entire bus driver.
 * \param  ChildHandleBuffer List of Child Handles to Stop.
 *
 * \return EFI_SUCCESS       This driver is removed ControllerHandle
 * \return other             This driver was not removed from this device
 */
EFI_STATUS EFIAPI CsDriverBindingStop (
		IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
		IN  EFI_HANDLE                      ControllerHandle,
		IN  UINTN                           NumberOfChildren,
		IN  EFI_HANDLE                      *ChildHandleBuffer   OPTIONAL
	) {

	EFI_STATUS error = EFI_SUCCESS;

	CS_DEBUG((D_INFO, L"CsDriverBindingStop() started.\n"));

	if (context.options.createChildDevice) {
		/* uninstall the provided protocol interfaces from child handle */
		delete_child_handle();
	}

	/* unpatch all patched code */
	if (context.ConsumedBlockIo) {
		CsUnpatchCode(CS_BLOCK_IO_READ);
		CsUnpatchCode(CS_BLOCK_IO_WRITE);
	}
	if (context.ConsumedBlockIo2) {
		CsUnpatchCode(CS_BLOCK_IO2_READ);
		CsUnpatchCode(CS_BLOCK_IO2_WRITE);
	}
	SetMem(&context.patchBytes, sizeof(context.patchBytes), 0);
	SetMem(&context.patchedBytes, sizeof(context.patchedBytes), 0);

	/* close the opened BlockIO protocols */
	close_consumed_protocols(This, ControllerHandle);

	return error;
}

/*
 * \brief	Retrieves a Unicode string that is the user-readable name of the EFI Driver.
 *
 * \param  This       A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
 * \param  Language   A pointer to a three-character ISO 639-2 language identifier.
 *                    This is the language of the driver name that that the caller
 *                    is requesting, and it must match one of the languages specified
 *                    in SupportedLanguages.  The number of languages supported by a
 *                    driver is up to the driver writer.
 * \param  DriverName A pointer to the Unicode string to return.  This Unicode string
 *                    is the name of the driver specified by This in the language
 *                    specified by Language.
 *
 * \return EFI_SUCCESS           The Unicode string for the Driver specified by This
 *                               and the language specified by Language was returned
 *                               in DriverName.
 * \return EFI_INVALID_PARAMETER Language is NULL.
 * \return EFI_INVALID_PARAMETER DriverName is NULL.
 * \return EFI_UNSUPPORTED       The driver specified by This does not support the
 *                               language specified by Language.
 */
EFI_STATUS EFIAPI CsGetDriverName(
		IN EFI_COMPONENT_NAME_PROTOCOL           *This,
		IN  CHAR8                                *Language,
		OUT CHAR16                               **DriverName
	) {

	CS_DEBUG((D_INFO, L"CsGetDriverName(0x%x, 0x%x, 0x%x) started.\n", This, Language, DriverName));

	if (DriverName) {
		*DriverName = DriverNameString;
	}
	else
		return EFI_INVALID_PARAMETER;

	return EFI_SUCCESS;
}

/*
 * Retrieves a Unicode string that is the user readable name of the controller
 * that is being managed by an EFI Driver.
 *
 * \param  This             A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
 * \param  ControllerHandle The handle of a controller that the driver specified by
 *                          This is managing.  This handle specifies the controller
 *                          whose name is to be returned.
 * \param  ChildHandle      The handle of the child controller to retrieve the name
 *                          of.  This is an optional parameter that may be NULL.  It
 *                          will be NULL for device drivers.  It will also be NULL
 *                          for a bus drivers that wish to retrieve the name of the
 *                          bus controller.  It will not be NULL for a bus driver
 *                          that wishes to retrieve the name of a child controller.
 * \param  Language         A pointer to a three character ISO 639-2 language
 *                          identifier.  This is the language of the controller name
 *                          that the caller is requesting, and it must match one
 *                          of the languages specified in SupportedLanguages.  The
 *                          number of languages supported by a driver is up to the
 *                          driver writer.
 * \param  ControllerName   A pointer to the Unicode string to return.  This Unicode
 *                          string is the name of the controller specified by
 *                          ControllerHandle and ChildHandle in the language specified
 *                          by Language, from the point of view of the driver specified
 *                          by This.
 *
 * \return EFI_SUCCESS           The Unicode string for the user-readable name in the
 *                               language specified by Language for the driver
 *                               specified by This was returned in DriverName.
 * \return EFI_INVALID_PARAMETER ControllerHandle is NULL.
 * \return EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid EFI_HANDLE.
 * \return EFI_INVALID_PARAMETER Language is NULL.
 * \return EFI_INVALID_PARAMETER ControllerName is NULL.
 * \return EFI_UNSUPPORTED       The driver specified by This is not currently managing
 *                               language specified by Language.
 */
EFI_STATUS EFIAPI CsGetControllerName(
		IN EFI_COMPONENT_NAME_PROTOCOL                              *This,
		IN  EFI_HANDLE                                              ControllerHandle,
		IN  EFI_HANDLE                                              ChildHandle        OPTIONAL,
		IN  CHAR8                                                   *Language,
		OUT CHAR16                                                  **ControllerName
	) {

	CS_DEBUG((D_INFO, L"CsGetControllerName(0x%x, 0x%x, 0x%x) started.\n", This, Language, ControllerName));

#if 1
	if ((ControllerName) && (ChildHandle)) {
		*ControllerName = ControllerNameString;
	}
	else
		return EFI_INVALID_PARAMETER;

	return EFI_SUCCESS;
#else
	return EFI_UNSUPPORTED;
#endif
}


/*
 * Retrieves a string that is the user readable name of
 * the EFI Driver.
 *
 * \param  This       A pointer to the
 *                    EFI_COMPONENT_NAME2_PROTOCOL instance.
 *
 * \param  Language   A pointer to a Null-terminated ASCII string
 *                    array indicating the language. This is the
 *                    language of the driver name that the caller
 *                    is requesting, and it must match one of the
 *                    languages specified in SupportedLanguages.
 *                    The number of languages supported by a
 *                    driver is up to the driver writer. Language
 *                    is specified in RFC 4646 language code
 *                    format.
 *
 * \param  DriverName A pointer to the string to return.
 *                    This string is the name of the
 *                    driver specified by This in the language
 *                    specified by Language.
 *
 * \return EFI_SUCCESS           The string for the
 *                               Driver specified by This and the
 *                               language specified by Language
 *                               was returned in DriverName.
 *
 * \return EFI_INVALID_PARAMETER Language is NULL.
 *
 * \return EFI_INVALID_PARAMETER DriverName is NULL.
 *
 * \return EFI_UNSUPPORTED       The driver specified by This
 *                               does not support the language
 *                               specified by Language.
 */
EFI_STATUS EFIAPI CsGetDriverName2(
		IN EFI_COMPONENT_NAME2_PROTOCOL          *This,
		IN  CHAR8                                *Language,
		OUT CHAR16                               **DriverName
	) {

	if (DriverName)
		*DriverName = DriverNameString;
	else
		return EFI_INVALID_PARAMETER;

	return EFI_SUCCESS;
}

/*
 * Retrieves a string that is the user readable name of
 * the controller that is being managed by an EFI Driver.
 *
 * \param  This             A pointer to the
 *                          EFI_COMPONENT_NAME2_PROTOCOL instance.
 *
 * \param  ControllerHandle The handle of a controller that the
 *                          driver specified by This is managing.
 *                          This handle specifies the controller
 *                          whose name is to be returned.
 *
 * \param  ChildHandle      The handle of the child controller to
 *                          retrieve the name of.  This is an
 *                          optional parameter that may be NULL.
 *                          It will be NULL for device drivers.
 *                          It will also be NULL for bus
 *                          drivers that wish to retrieve the
 *                          name of the bus controller.  It will
 *                          not be NULL for a bus driver that
 *                          wishes to retrieve the name of a
 *                          child controller.
 *
 * \param  Language         A pointer to a Null-terminated ASCII
 *                          string array indicating the language.
 *                          This is the language of the driver
 *                          name that the caller is requesting,
 *                          and it must match one of the
 *                          languages specified in
 *                          SupportedLanguages. The number of
 *                          languages supported by a driver is up
 *                          to the driver writer. Language is
 *                          specified in RFC 4646 language code
 *                          format.
 *
 * \param  ControllerName   A pointer to the string to return.
 *                          This string is the name of the controller
 *                          specified by ControllerHandle and ChildHandle
 *                          in the language specified by Language
 *                          from the point of view of the driver
 *                          specified by This.
 *
 * \return EFI_SUCCESS           The string for the user
 *                               readable name in the language
 *                               specified by Language for the
 *                               driver specified by This was
 *                               returned in DriverName.
 *
 * \return EFI_INVALID_PARAMETER ControllerHandle is NULL.
 *
 * \return EFI_INVALID_PARAMETER ChildHandle is not NULL and it
 *                               is not a valid EFI_HANDLE.
 *
 * \return EFI_INVALID_PARAMETER Language is NULL.
 *
 * \return EFI_INVALID_PARAMETER ControllerName is NULL.
 *
 * \return EFI_UNSUPPORTED       The driver specified by This is
 *                               not currently managing the
 *                               controller specified by
 *                               ControllerHandle and
 *                               ChildHandle.
 */
EFI_STATUS EFIAPI CsGetControllerName2(
		IN EFI_COMPONENT_NAME2_PROTOCOL *This,
		IN  EFI_HANDLE                  ControllerHandle,
		IN  EFI_HANDLE                  ChildHandle        OPTIONAL,
		IN  CHAR8                       *Language,
		OUT CHAR16                      **ControllerName
	) {

	CS_DEBUG((D_INFO, L"CsGetControllerName2(0x%x, 0x%x, 0x%x) started.\n", This, Language, ControllerName));

#if 1
	if ((ControllerName) && (ChildHandle)) {
		*ControllerName = ControllerNameString;
	}
	else
		return EFI_INVALID_PARAMETER;

	return EFI_SUCCESS;
#else
	return EFI_UNSUPPORTED;
#endif
}

/*
 * \brief	Read BufferSize bytes from Lba into Buffer.
 *
 * \param  This       Indicates a pointer to the calling context.
 * \param  MediaId    Id of the media, changes every time the media is replaced.
 * \param  Lba        The starting Logical Block Address to read from
 * \param  BufferSize Size of Buffer, must be a multiple of device block size.
 * \param  Buffer     A pointer to the destination buffer for the data. The caller is
 *                    responsible for either having implicit or explicit ownership of the buffer.
 *
 * \return EFI_SUCCESS           The data was read correctly from the device.
 * \return EFI_DEVICE_ERROR      The device reported an error while performing the read.
 * \return EFI_NO_MEDIA          There is no media in the device.
 * \return EFI_MEDIA_CHANGED     The MediaId does not matched the current device.
 * \return EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
 * \return EFI_INVALID_PARAMETER The read request contains LBAs that are not valid,
 *                               or the buffer is not on proper alignment.
 */
EFI_STATUS EFIAPI CsChildReadBlocks(
		IN EFI_BLOCK_IO		              *This,
		IN UINT32                         MediaId,
		IN EFI_LBA                        Lba,
		IN UINTN                          BufferSize,
		OUT VOID                          *Buffer
	) {

	CS_DEBUG((D_INFO, L"CsChildReadBlocks() started (LBA 0x%lx).\n", Lba));

	return CsParentReadWriteBlocks(CS_BLOCK_IO_READ, context.ConsumedBlockIo, MediaId, Lba, NULL, BufferSize, Buffer);
}

/*
 * \brief	Write BufferSize bytes from Lba into Buffer.
 *
 * \param  This       Indicates a pointer to the calling context.
 * \param  MediaId    The media ID that the write request is for.
 * \param  Lba        The starting logical block address to be written. The caller is
 *                    responsible for writing to only legitimate locations.
 * \param  BufferSize Size of Buffer, must be a multiple of device block size.
 * \param  Buffer     A pointer to the source buffer for the data.
 *
 * \return EFI_SUCCESS           The data was written correctly to the device.
 * \return EFI_WRITE_PROTECTED   The device can not be written to.
 * \return EFI_DEVICE_ERROR      The device reported an error while performing the write.
 * \return EFI_NO_MEDIA          There is no media in the device.
 * \return EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
 * \return EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
 * \return EFI_INVALID_PARAMETER The write request contains LBAs that are not valid,
 *                               or the buffer is not on proper alignment.
 */
EFI_STATUS EFIAPI CsChildWriteBlocks(
		IN EFI_BLOCK_IO		              *This,
		IN UINT32                         MediaId,
		IN EFI_LBA                        Lba,
		IN UINTN                          BufferSize,
		IN VOID                           *Buffer
	) {

	CS_DEBUG((D_INFO, L"CsChildWriteBlocks(ID=0x%x, LBA=0x%lx, Size=0x%x) called.\n", MediaId, Lba, BufferSize));

	return CsParentReadWriteBlocks(CS_BLOCK_IO_WRITE, context.ConsumedBlockIo, MediaId, Lba, NULL, BufferSize, Buffer);
}

/*
 * \brief	Flush the Block Device.
 *
 * \param  This              Indicates a pointer to the calling context.
 *
 * \return EFI_SUCCESS       All outstanding data was written to the device
 * \return EFI_DEVICE_ERROR  The device reported an error while writting back the data
 * \return EFI_NO_MEDIA      There is no media in the device.
 */
EFI_STATUS EFIAPI CsChildFlushBlocks(
		IN EFI_BLOCK_IO		*This
	) {

	if (context.ConsumedBlockIo == NULL) {
		CS_DEBUG((D_ERROR, L"CsFlushBlocks() called, but no consumed BlockIo interface exists.\n"));
		return EFI_NOT_FOUND;
	}

	return uefi_call_wrapper(context.ConsumedBlockIo->FlushBlocks, 1, context.ConsumedBlockIo);
}

/*
 *  \brief	Reset the Block Device.
 *
 *  \param  This                 Indicates a pointer to the calling context.
 *  \param  ExtendedVerification Driver may perform diagnostics on reset.
 *
 *  \return EFI_SUCCESS          The device was reset.
 *  \return EFI_DEVICE_ERROR     The device is not functioning properly and could
 *                               not be reset.
 */
EFI_STATUS EFIAPI CsReset(
		IN EFI_BLOCK_IO					*This,
		IN BOOLEAN                        ExtendedVerification
	) {

	if (context.ConsumedBlockIo == NULL) {
		CS_DEBUG((D_ERROR, L"CsReset() called, but no consumed BlockIo interface exists.\n"));
		return EFI_NOT_FOUND;
	}

	return uefi_call_wrapper(context.ConsumedBlockIo->Reset, 2, context.ConsumedBlockIo, ExtendedVerification);
}

/*
 * \brief	Uninstall EFI driver
 *
 * \param	ImageHandle Handle identifying the loaded image
 *
 * \return	Status EFI status code to return on exit
 */
EFI_STATUS EFIAPI CsDriverUninstall(IN EFI_HANDLE ImageHandle) {

	EFI_STATUS error;
	UINTN NumHandles;
	EFI_HANDLE *Handles;
	UINTN i;

	CS_DEBUG((D_INFO, L"CS driver uninstall started.\n"));

	/* Enumerate all handles */
	error = uefi_call_wrapper(BS->LocateHandleBuffer, 5, AllHandles, NULL, NULL, &NumHandles, &Handles);
	/* Disconnect controllers linked to our driver. This action will trigger a call to BindingStop */
	if (error == EFI_SUCCESS) {
		for (i=0; i < NumHandles; i++) {
			/* Make sure to filter on DriverBindingHandle, else EVERYTHING gets disconnected! */
			error = uefi_call_wrapper(BS->DisconnectController, 3,
					Handles[i], CsDriverBinding.DriverBindingHandle, NULL);
			if (error == EFI_SUCCESS) {
				CS_DEBUG((D_INFO, L"DisconnectController[0x%x]", i));
			}
		}
	} else {
		Print(L"Unable to enumerate handles (%r)", error);
	}

	uefi_call_wrapper(BS->FreePool, 1, Handles);

	/* Now that all controllers are disconnected, we can safely remove our protocols */
	LibUninstallProtocolInterfaces(ImageHandle,
			&DriverBindingProtocol, &CsDriverBinding,
			&ComponentNameProtocol, &CsComponentName,
			&ComponentName2Protocol, &CsComponentName2,
			NULL);

	SetMem(&context, sizeof(context), 0);	/* destroy sensitive data */

	CS_DEBUG((D_INFO, L"CS driver uninstalled.\n"));

	return EFI_SUCCESS;
}

/*
 * 	\brief	initialize the system context based on the driver startup parameters
 *
 * 	This function checks the start parameters of the drivers and fills the system context
 * 	with the corresponding values. Also, the required sector offsets in case of a hidden
 * 	volume are calculated and stored in the context.
 *
 *	\param LoadedImage		loaded image containing the startup parameters (options)
 *
 *	\return		the success state of the function
 */
static EFI_STATUS cs_init_crypto_options(EFI_LOADED_IMAGE *LoadedImage) {

	ASSERT(LoadedImage != NULL);

	aes_init();

	if (LoadedImage->LoadOptionsSize == sizeof(context.options)) {
		CopyMem(&context.options, LoadedImage->LoadOptions, sizeof(context.options));
		EFIDebug = context.options.debug;
	} else {
	    CS_DEBUG((D_WARN, L"Attention: unexpected Option size of provided data (0x%x/0x%x) -> ignoring.\n",
	    		LoadedImage->LoadOptionsSize, sizeof(context.options)));
		EFIDebug = D_ERROR | D_WARN | D_LOAD | D_BLKIO | D_INIT | D_INFO;
	    context.options.createChildDevice = TRUE;	/* enabled for test purposes */
	}

	context.EndSector = context.options.StartSector + context.options.SectorCount;
	CS_DEBUG((D_INFO, L"driver init: StartSector 0x%x, SectorCount 0x%x\n",
			context.options.StartSector, context.options.SectorCount));
	CS_DEBUG((D_INFO, L"driver init: hiddenVolumePresent %x, Encryption Algorithm ID/mode: 0x%x/0x%x\n",
			context.options.isHiddenVolume, context.options.cipher.algo, context.options.cipher.mode));
	CS_DEBUG((D_INFO, L"driver init: need to build child device: %x\n",
			context.options.createChildDevice));

	if (context.options.isHiddenVolume) {
		/* see TC: BootEncryptedio.cpp ReadEncryptedSectors(), WriteEncryptedSectors() */
		context.hiddenUnitOffset   = context.options.HiddenVolumeStartUnitNo - context.options.StartSector;
		context.hiddenSectorOffset = context.options.HiddenVolumeStartSector - context.options.StartSector;
	    CS_DEBUG((D_INFO, L"Hidden Volume: sector/unit offset: 0x%x/0x%x\n",
	    		context.hiddenSectorOffset, context.hiddenUnitOffset));
	} else {
		context.hiddenUnitOffset = context.hiddenSectorOffset = 0;
	}

	return EFI_SUCCESS;
}

/*
 * Install EFI driver - Will be the entrypoint for the driver executable
 * http://wiki.phoenix.com/wiki/index.php/EFI_IMAGE_ENTRY_POINT
 *
 * \param	ImageHandle Handle identifying the loaded image
 * \param	SystemTable Pointers to EFI system calls
 *
 * \return	Status EFI status code to return on exit
 */
EFI_STATUS EFIAPI CsDriverInstall(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE* SystemTable) {

	EFI_STATUS error;
	EFI_LOADED_IMAGE *LoadedImage = NULL;

	ASSERT(ImageHandle != NULL);
	ASSERT(SystemTable != NULL);

    InitializeLib(ImageHandle, SystemTable);
    SetMem(&context, sizeof(context), 0);
    context.factorMediaBlock = 1;

	// EFIDebug = D_INFO;

    CS_DEBUG((D_INFO, L"CS driver install started.\n"));
    CS_DEBUG((D_INFO, L"EFI system table revision 0x%x.\n", SystemTable->Hdr.Revision));

	/* Grab a handle to this image, so that we can add an unload to the driver */
	error = uefi_call_wrapper(BS->OpenProtocol, 6, ImageHandle, &LoadedImageProtocol,
			(VOID **)&LoadedImage, ImageHandle,	NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
	if (EFI_ERROR(error)) {
		Print(L"Could not open loaded image protocol (%r)", error);
		return error;
	}

	/* Configure driver binding protocol */
	CsDriverBinding.ImageHandle = ImageHandle;
	CsDriverBinding.DriverBindingHandle = ImageHandle;

	/* get the provided parameters (sector and cipher data) and store them in system context */
	error = cs_init_crypto_options(LoadedImage);
	if (EFI_ERROR(error)) {
		return error;
	}

	/* Install driver */
	error = LibInstallProtocolInterfaces(&CsDriverBinding.DriverBindingHandle,
			&DriverBindingProtocol, &CsDriverBinding,
			&ComponentNameProtocol, &CsComponentName,
			&ComponentName2Protocol, &CsComponentName2,
			NULL);
	if (EFI_ERROR(error)) {
		cs_print_msg(L"Could not install driver (%r)\n", error);
		return error;
	}

	/* Register the uninstall callback */
	LoadedImage->Unload = CsDriverUninstall;

	CS_DEBUG((D_INFO, L"CS driver installed.\n"));

	return EFI_SUCCESS;
}
