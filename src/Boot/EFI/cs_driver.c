/*  cs_driver.c - CipherShed EFI boot loader
 *  main file for the CipherShed EFI driver providing access to the encrypted partition
 *
 *
 *
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

typedef struct {
	LIST_ENTRY List;
	UINT32 MediaId;
	UINTN BufferSize;
	VOID *Buffer;
	EFI_LBA Lba;
	EFI_EVENT CallerEvent;
	EFI_BLOCK_IO2_TOKEN Token;
} CS_READ_BLOCKS_SUBTASK; /* task to non-blocking read */

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

EFI_BLOCK_IO CsBlockIo = {
		.Revision = EFI_BLOCK_IO_INTERFACE_REVISION3,
		.Media = NULL,
		.Reset = CsReset,
		.ReadBlocks = CsReadBlocks,
		.WriteBlocks = CsWriteBlocks,
		.FlushBlocks = CsFlushBlocks
};

EFI_BLOCK_IO2_PROTOCOL CsBlockIo2 = {
		.Media = NULL,
		.Reset = CsResetEx,
		.ReadBlocksEx = CsReadBlocksEx,
		.WriteBlocksEx = CsWriteBlocksEx,
		.FlushBlocksEx = CsFlushBlocksEx
};

CHAR16 *DriverNameString = CS_DRIVER_NAME L" Version " WIDEN(STRINGIFY(CS_DRIVER_VERSION));
CHAR16 *ControllerNameString = CS_CONTROLLER_NAME L" Version " WIDEN(STRINGIFY(CS_DRIVER_VERSION));

static struct cs_driver_context {
	UINT16 factorMediaBlock;	/* ratio between media block size and crypto unit size
	 	 	 	 	 	 	 	   (the crypto unit size is always ENCRYPTION_DATA_UNIT_SIZE);
	 	 	 	 	 	 	 	   this shall support media block sizes bigger than
	 	 	 	 	 	 	 	   ENCRYPTION_DATA_UNIT_SIZE */
	struct {
		UINT32 blockIoInstalled:1;
		UINT32 blockIo2Installed:1;
	} status;
	EFI_BLOCK_IO *ConsumedBlockIo;
	EFI_BLOCK_IO2 *ConsumedBlockIo2;
	LIST_ENTRY TaskListReadEx;
	FLOCK TaskListReadExLock;
	EFI_HANDLE ChildHandle;
	EFI_DEVICE_PATH *ChildDevicePath;
	UINT64 hiddenSectorOffset;		/* only used for hidden volume */
	UINT64 hiddenUnitOffset;		/* only used for hidden volume */
	struct cs_efi_option_data options;
	UINT64 EndSector;				/* options.StartSector + options.SectorCount */
} context;

#if 0
/*
 *
 * \brief	Convert one Null-terminated ASCII string to a Null-terminated
 * 	  	  	Unicode string and returns the Unicode string.
 *
 * This function converts the contents of the ASCII string Source to the Unicode
 * string Destination, and returns Destination.  The function terminates the
 * Unicode string Destination by appending a Null-terminator character at the end.
 * The caller is responsible to make sure Destination points to a buffer with size
 * equal or greater than ((AsciiStrLen (Source) + 1) * sizeof (CHAR16)) in bytes.
 *
 *  \param  Source        A pointer to a Null-terminated ASCII string.
 *  \param  Destination   A pointer to a Null-terminated Unicode string.
 *
 *  \return Destination.
 */
static CHAR16 EFIAPI *AsciiStrToUnicodeStr (
  IN      CONST CHAR8               *Source,
  OUT     CHAR16                    *Destination  ) {

  CHAR16                            *ReturnValue;

  ASSERT (Destination != NULL);

  //
  // ASSERT Source is less long than PcdMaximumAsciiStringLength
  //
  ASSERT (strlena (Source) != 0);

  //
  // Source and Destination should not overlap
  //
  ASSERT ((UINTN) ((CHAR8 *) Destination - Source) > strlena (Source));
  ASSERT ((UINTN) (Source - (CHAR8 *) Destination) > (strlena (Source) * sizeof (CHAR16)));


  ReturnValue = Destination;

//  *(Destination++) = 0xfeff;
  while (*Source != '\0') {
//	  CHAR16 tmp = (CHAR16) *(Source++);
//	    *(Destination++) = tmp<<8;
    *(Destination++) = (CHAR16) *(Source++);
  }
  //
  // End the Destination with a NULL.
  //
  *Destination = '\0';

  //
  // ASSERT Original Destination is less long than PcdMaximumUnicodeStringLength
  //
  ASSERT (StrSize (ReturnValue) != 0);

  return ReturnValue;
}
#endif

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
 * \brief	destroy single subtask to read blocks (non-blocking)
 *
 * The function removes the given subtask from the list, removes the corresponding event,
 * and signals the event of the caller of ReadBlocksEx() to inform about the result.
 * Then the memory of subtask is unallocated.
 *
 * \param	Subtask		pointer to the subtask to destroy
 */
static VOID EFIAPI _csDestroySubtask(IN CS_READ_BLOCKS_SUBTASK *Subtask) {

    ASSERT(Subtask != NULL);

	if (Subtask == NULL) {
		CS_DEBUG((D_ERROR, L"csDestroySubtask(): invalid parameter\n"));
		return;
	}
	AcquireLock(&context.TaskListReadExLock);
	RemoveEntryList(&Subtask->List);
	ReleaseLock(&context.TaskListReadExLock);

	if (Subtask->Token.Event != NULL) {
		uefi_call_wrapper(BS->CloseEvent, 1, Subtask->Token.Event);
	}

	/* signal the event provided by the caller of ReadBlocksEx()
	 * together with the status of the performed action: */
    Subtask->Token.Event = Subtask->CallerEvent;
    uefi_call_wrapper(BS->SignalEvent, 1, Subtask->Token.Event);

	FreePool(Subtask);
}

/*
 * \brief	destroy all subtasks known by the driver for non-blocking read requests
 *
 * The function removes all subtasks from the linked list of the driver
 *
 */
static VOID EFIAPI csDestroyAllSubtasks(VOID) /* attention: untested! */ {

	while (!IsListEmpty(&context.TaskListReadEx)) {
		CS_READ_BLOCKS_SUBTASK *Subtask = _CR(&context.TaskListReadEx, CS_READ_BLOCKS_SUBTASK, List);
		_csDestroySubtask(Subtask);
	}
}

/*
 * \brief	callback function for the non-blocking read function
 *
 * This callback function will be called when a non-blocking disk read (called
 * by ReadBlocksEx) is finished. In case of successful disk read, the buffer
 * is decrypted and the caller is signaled about the result.
 *
 * \param	Event	Event whose notification function is being invoked.
 * \param	Context		The pointer to the notification function's context,
 *                      which points to the CS_READ_BLOCKS_SUBTASK instance.
 */
VOID EFIAPI csCallbackReadComplete(IN EFI_EVENT Event, IN VOID *Context) {

	EFI_STATUS error;
	CS_READ_BLOCKS_SUBTASK *Subtask = (CS_READ_BLOCKS_SUBTASK *)Context;

	if (Subtask == NULL) {
		CS_DEBUG((D_ERROR, L"csCallbackReadComplete() with invalid parameter\n"));
		return;
	}

	CS_DEBUG((D_INFO, L"csCallbackReadComplete(ID=0x%x, LBA=0x%x, Size=0x%x, Status=%r) called.\n",
			Subtask->MediaId, Subtask->Lba, Subtask->BufferSize, Subtask->Token.TransactionStatus));

	if (EFI_ERROR(Subtask->Token.TransactionStatus)) {
		CS_DEBUG((D_ERROR, L"ReadBlocksEx() failed (%r)\n", Subtask->Token.TransactionStatus));
	} else {
		error = decryptBlocks(Subtask->Lba, Subtask->BufferSize, Subtask->Buffer);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"csCallbackReadComplete() failed (%r)\n", error));
			Subtask->Token.TransactionStatus = error; /* used in _csDestroySubtask() */
		}
	}

	_csDestroySubtask(Subtask);	/* contains the signaling to the caller */

	FreePool(Subtask);
}

/*
 * \brief	creates a subtask for non-blocking read
 *
 * The function defines and initializes a subtask which is used to asynchronously read from
 * a block device. A callback function is defined that is called when the read operation
 * is finished.
 *
 * \param	BufferSize	Size of Buffer, must be a multiple of device block size.
 * \param	Buffer		A pointer to the destination buffer for the data. The caller is
 *						responsible for either having implicit or explicit ownership of the buffer.
 * \param 	MediaId  	Id of the media, changes every time the media is replaced.
 * \param 	Lba         The starting Logical Block Address to read from
 * \param	CallerEvent	The event of the caller of ReadBlocksEx()
 *
 * \return	pointer to the creates subtask structure or NULL on error
 */
static CS_READ_BLOCKS_SUBTASK EFIAPI *csCreateSubtask(UINTN BufferSize, VOID *Buffer,
		UINT32 MediaId, EFI_LBA Lba, EFI_EVENT CallerEvent) /* attention: untested! */ {

	CS_READ_BLOCKS_SUBTASK *Subtask;
	EFI_STATUS error;

    ASSERT(Buffer != NULL);

	Subtask = AllocateZeroPool(sizeof(CS_READ_BLOCKS_SUBTASK));
	if (Subtask == NULL) {
		CS_DEBUG((D_ERROR, L"unable to allocate subtask\n"));
		return NULL;
	}
	Subtask->BufferSize = BufferSize;
	Subtask->Buffer = Buffer;
	Subtask->MediaId = MediaId;
	Subtask->Lba = Lba;
	Subtask->CallerEvent = CallerEvent;

	error = uefi_call_wrapper(BS->CreateEvent, 5, EVT_NOTIFY_SIGNAL, TPL_NOTIFY, csCallbackReadComplete,
             Subtask, &Subtask->Token.Event);
	if (EFI_ERROR(error)) {
		FreePool (Subtask);
		CS_DEBUG((D_ERROR, L"CreateEvent() returned %r\n", error));
		return NULL;
	}

	AcquireLock(&context.TaskListReadExLock);
	InsertTailList(&context.TaskListReadEx, &Subtask->List);
	ReleaseLock(&context.TaskListReadExLock);

	return Subtask;
}

/**
  \brief	Test to see if this driver supports ControllerHandle.

  This service is called by the EFI boot service ConnectController(). In order
  to make drivers as small as possible, there are a few calling restrictions for
  this service. ConnectController() must follow these calling restrictions.
  If any other agent wishes to call Supported() it must also follow these
  calling restrictions.

  \param  This                Protocol instance pointer.
  \param  Controller          Handle of device to test
  \param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  \return EFI_SUCCESS         This driver supports this device
  \return EFI_ALREADY_STARTED This driver is already running on this device
  \return other               This driver does not support this device

**/
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

	return error;
}

/**
  \brief	uninstall the provided protocol interfaces to the given controller handle

  This function tries to uninstall the BLOCK_IO protocol and the BLOCK_IO2 protocol
  (if this protocol is provided) from the given controller handle.

  \param  ControllerHandle		controller handle
**/
static void uninstall_provided_protocols(IN EFI_HANDLE ControllerHandle) {

	ASSERT(ControllerHandle != NULL);

	/* uninstall the provided protocol interfaces */
	if (context.status.blockIoInstalled) {
		LibUninstallProtocolInterfaces(ControllerHandle,
				&BlockIoProtocol, &CsBlockIo,
				&CsCallerIdGuid, NULL /* no data required */,
				NULL);
		context.status.blockIoInstalled = 0;
	}
	if (context.status.blockIo2Installed) {
		LibUninstallProtocolInterfaces(ControllerHandle,
				&BlockIo2Protocol, &CsBlockIo2,
				&CsCallerIdGuid, NULL /* no data required */,
				NULL);
		context.status.blockIo2Installed = 0;
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

/**
  \brief	install the provided protocol interfaces to the given controller handle

  This function tries to install the BLOCK_IO protocol and the BLOCK_IO2 protocol
  (if this protocol is consumed) to the given controller handle. Also, the function
  installs the devicePathProtocol interface, if DevicePath is not NULL.

  \param  ControllerHandle		controller handle
  \param  DevicePath			device path of the handle

  \return	the success state of the function
**/
static EFI_STATUS install_provided_protocols(IN EFI_HANDLE *pControllerHandle, IN EFI_DEVICE_PATH *DevicePath) {
	EFI_STATUS error;

	CsBlockIo.Media = context.ConsumedBlockIo->Media;
	if (context.ConsumedBlockIo2) {
		CsBlockIo2.Media = context.ConsumedBlockIo2->Media;
		error = LibInstallProtocolInterfaces(pControllerHandle,
				&BlockIoProtocol, &CsBlockIo,
				&BlockIo2Protocol, &CsBlockIo2,
				&ComponentNameProtocol, &CsComponentName,
				&ComponentName2Protocol, &CsComponentName2,
				&CsCallerIdGuid, NULL /* no data required */,
				(DevicePath ? &DevicePathProtocol : NULL), DevicePath,
				NULL);
		if (!EFI_ERROR(error)) {
			context.status.blockIoInstalled = 1;
			context.status.blockIo2Installed = 1;
		}
	} else {
		error = LibInstallProtocolInterfaces(pControllerHandle,
				&BlockIoProtocol, &CsBlockIo,
				&ComponentNameProtocol, &CsComponentName,
				&ComponentName2Protocol, &CsComponentName2,
				&CsCallerIdGuid, NULL /* no data required */,
				(DevicePath ? &DevicePathProtocol : NULL), DevicePath,
				NULL);
		if (!EFI_ERROR(error)) {
			context.status.blockIoInstalled = 1;
			context.status.blockIo2Installed = 0;
		}
	}
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"error while installation of provided protocols: %r\n", error));
	} else {
		context.factorMediaBlock = CsBlockIo.Media->BlockSize / ENCRYPTION_DATA_UNIT_SIZE;

		CS_DEBUG((D_INFO, L"media block size is 0x%x\n", CsBlockIo.Media->BlockSize));
		CS_DEBUG((D_INFO, L"removable media/logical partition: %x/%x\n",
				CsBlockIo.Media->RemovableMedia, CsBlockIo.Media->LogicalPartition));
		CS_DEBUG((D_INFO, L"first/last LBA 0x%lx/0x%lx\n",
				CsBlockIo.Media->LowestAlignedLba, CsBlockIo.Media->LastBlock));
		ASSERT(CsBlockIo.Media->BlockSize >= ENCRYPTION_DATA_UNIT_SIZE);
		ASSERT(CsBlockIo.Media->BlockSize % ENCRYPTION_DATA_UNIT_SIZE == 0);
	}

	return error;
}

/**
  \brief	close the consumed protocols (BlockIO and BlockIO2) at the given controller

  This function close the protocols that are consumed by the driver at the given controller.

  \param  This              Protocol instance pointer
  \param  ControllerHandle	parent controller handle

  \return	the success state of the function
**/
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

/**
  \brief	open the consumed protocols (BlockIO and BlockIO2) at the given controller

  This function opens the protocols that are consumed by the driver at the given controller.
  Depending on the given flag, the open mode changes: when by_child is set, then the protocols
  are opened with the child flag and the child handle is given as argument instead of the parent
  handle. This is used to create the link between the parent and the child handle (see
  Driver Writers Guide, example 31).

  \param  This              Protocol instance pointer
  \param  ControllerHandle	parent controller handle
  \param  by_client			switch whether the protocol is opened in child mode

  \return	the success state of the function
**/
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

/**
  \brief	create a new child device handle and install BlockIO/BlockIO2 protocol at it.

  This function takes the given device handle and creates a child handle of it.
  The device path of the given handle is extended by the string CS_CHILD_PATH_EXTENSION
  which becomes the device path of the new child handle. The child handle is
  stored in the system context.

  \param  ControllerHandle		parent controller handle

  \return	the success state of the function
**/
#if 1
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
#else
static EFI_STATUS add_new_child_handle(IN EFI_HANDLE ControllerHandle) {
	EFI_STATUS error;
	EFI_DEVICE_PATH *ParentDevicePath;
	VENDOR_DEVICE_PATH path;
	const EFI_GUID tmp_GUID = CS_CALLER_ID_GUID;

	ZeroMem (&path, sizeof(path));
	path.Header.Type = HARDWARE_DEVICE_PATH;
	path.Header.SubType = HW_VENDOR_DP;
	path.Guid = tmp_GUID;
	SetDevicePathNodeLength(&path.Header, sizeof(path) + sizeof(path.Guid));

	/* get ParentDevicePath */
	error = uefi_call_wrapper(BS->HandleProtocol, 3, ControllerHandle, &DevicePathProtocol,
					  (VOID **) &ParentDevicePath);
	if (!EFI_ERROR(error)) {

		CS_DEBUG((D_INFO, L"ParentDevicePath type/subtype 0x%x/0x%x\n",
							ParentDevicePath->Type, ParentDevicePath->SubType));

		context.ChildDevicePath = AppendDevicePathNode (ParentDevicePath, (EFI_DEVICE_PATH *) &path);
		if (context.ChildDevicePath == NULL) {
			error = EFI_OUT_OF_RESOURCES;
		}
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
#endif

/**
  \brief	destroy the child handle that was created by add_new_child_handle().

  \return	the success state of the function
**/
static EFI_STATUS delete_child_handle(void) {

	if (context.ChildHandle) {
		uninstall_provided_protocols(context.ChildHandle);
		return EFI_SUCCESS;
	} else {
		return EFI_DEVICE_ERROR;
	}
}

/**
  \brief	Start this driver on ControllerHandle.

  This service is called by the EFI boot service ConnectController(). In order
  to make drivers as small as possible, there are a few calling restrictions for
  this service. ConnectController() must follow these calling restrictions. If
  any other agent wishes to call Start() it must also follow these calling
  restrictions.

  \param  This                 Protocol instance pointer.
  \param  ControllerHandle     Handle of device to bind driver to
  \param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start -> not used in this driver

  \return EFI_SUCCESS          This driver is added to ControllerHandle
  \return EFI_ALREADY_STARTED  This driver is already running on ControllerHandle
  \return other                This driver does not support this device

**/
EFI_STATUS EFIAPI CsDriverBindingStart (
		IN EFI_DRIVER_BINDING_PROTOCOL  *This,
		IN EFI_HANDLE                   ControllerHandle,
		IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
	) {

	EFI_STATUS error;

	CS_DEBUG((D_INFO, L"CsDriverBindingStart(0x%x) started.\n", ControllerHandle));

	/* exclusively open the protocols that are consumed by the driver: BlockIoProtocol and BlockIo2Protocol */
	error = open_consumed_protocols(This, ControllerHandle, FALSE);
	if (EFI_ERROR(error)) {
		return error;
	}

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

	/* initialize driver data, in case of error: return EFI_OUT_OF_RESOURCES */
	InitializeListHead(&context.TaskListReadEx);
	InitializeLock(&context.TaskListReadExLock, TPL_NOTIFY);

	CS_DEBUG((D_INFO, L"CsDriverBindingStart() succeeded.\n", ControllerHandle));

	return EFI_SUCCESS;

fail:
	delete_child_handle();
	close_consumed_protocols(This, ControllerHandle);

	return EFI_DEVICE_ERROR;
}

/**
  \brief	Stop this driver on ControllerHandle.

  This service is called by the EFI boot service DisconnectController().
  In order to make drivers as small as possible, there are a few calling
  restrictions for this service. DisconnectController() must follow these
  calling restrictions. If any other agent wishes to call Stop() it must
  also follow these calling restrictions.

  \param  This              Protocol instance pointer.
  \param  ControllerHandle  Handle of device to stop driver on
  \param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  \param  ChildHandleBuffer List of Child Handles to Stop.

  \return EFI_SUCCESS       This driver is removed ControllerHandle
  \return other             This driver was not removed from this device

**/
EFI_STATUS EFIAPI CsDriverBindingStop (
		IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
		IN  EFI_HANDLE                      ControllerHandle,
		IN  UINTN                           NumberOfChildren,
		IN  EFI_HANDLE                      *ChildHandleBuffer   OPTIONAL
	) {

	EFI_STATUS error = EFI_SUCCESS;

	CS_DEBUG((D_INFO, L"CsDriverBindingStop() started.\n"));

	/* uninstall the provided protocol interfaces from child handle */
	delete_child_handle();

	/* close the opened BlockIO/BlockIO2 protocols */
	close_consumed_protocols(This, ControllerHandle);

	/* release buffers of the driver, if applicable, erase sensitive data */
	csDestroyAllSubtasks();

	return error;
}

/**
  \brief	Retrieves a Unicode string that is the user-readable name of the EFI Driver.

  \param  This       A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  \param  Language   A pointer to a three-character ISO 639-2 language identifier.
                     This is the language of the driver name that that the caller
                     is requesting, and it must match one of the languages specified
                     in SupportedLanguages.  The number of languages supported by a
                     driver is up to the driver writer.
  \param  DriverName A pointer to the Unicode string to return.  This Unicode string
                     is the name of the driver specified by This in the language
                     specified by Language.

  \return EFI_SUCCESS           The Unicode string for the Driver specified by This
                                and the language specified by Language was returned
                                in DriverName.
  \return EFI_INVALID_PARAMETER Language is NULL.
  \return EFI_INVALID_PARAMETER DriverName is NULL.
  \return EFI_UNSUPPORTED       The driver specified by This does not support the
                                language specified by Language.

**/
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

/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by an EFI Driver.

  \param  This             A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
  \param  ControllerHandle The handle of a controller that the driver specified by
                           This is managing.  This handle specifies the controller
                           whose name is to be returned.
  \param  ChildHandle      The handle of the child controller to retrieve the name
                           of.  This is an optional parameter that may be NULL.  It
                           will be NULL for device drivers.  It will also be NULL
                           for a bus drivers that wish to retrieve the name of the
                           bus controller.  It will not be NULL for a bus driver
                           that wishes to retrieve the name of a child controller.
  \param  Language         A pointer to a three character ISO 639-2 language
                           identifier.  This is the language of the controller name
                           that the caller is requesting, and it must match one
                           of the languages specified in SupportedLanguages.  The
                           number of languages supported by a driver is up to the
                           driver writer.
  \param  ControllerName   A pointer to the Unicode string to return.  This Unicode
                           string is the name of the controller specified by
                           ControllerHandle and ChildHandle in the language specified
                           by Language, from the point of view of the driver specified
                           by This.

  \return EFI_SUCCESS           The Unicode string for the user-readable name in the
                                language specified by Language for the driver
                                specified by This was returned in DriverName.
  \return EFI_INVALID_PARAMETER ControllerHandle is NULL.
  \return EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid EFI_HANDLE.
  \return EFI_INVALID_PARAMETER Language is NULL.
  \return EFI_INVALID_PARAMETER ControllerName is NULL.
  \return EFI_UNSUPPORTED       The driver specified by This is not currently managing
                                language specified by Language.

**/
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


/**
  Retrieves a string that is the user readable name of
  the EFI Driver.

  \param  This       A pointer to the
                     EFI_COMPONENT_NAME2_PROTOCOL instance.

  \param  Language   A pointer to a Null-terminated ASCII string
                     array indicating the language. This is the
                     language of the driver name that the caller
                     is requesting, and it must match one of the
                     languages specified in SupportedLanguages.
                     The number of languages supported by a
                     driver is up to the driver writer. Language
                     is specified in RFC 4646 language code
                     format.

  \param  DriverName A pointer to the string to return.
                     This string is the name of the
                     driver specified by This in the language
                     specified by Language.

  \return EFI_SUCCESS           The string for the
                                Driver specified by This and the
                                language specified by Language
                                was returned in DriverName.

  \return EFI_INVALID_PARAMETER Language is NULL.

  \return EFI_INVALID_PARAMETER DriverName is NULL.

  \return EFI_UNSUPPORTED       The driver specified by This
                                does not support the language
                                specified by Language.
**/
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

/**
  Retrieves a string that is the user readable name of
  the controller that is being managed by an EFI Driver.

  \param  This             A pointer to the
                           EFI_COMPONENT_NAME2_PROTOCOL instance.

  \param  ControllerHandle The handle of a controller that the
                           driver specified by This is managing.
                           This handle specifies the controller
                           whose name is to be returned.

  \param  ChildHandle      The handle of the child controller to
                           retrieve the name of.  This is an
                           optional parameter that may be NULL.
                           It will be NULL for device drivers.
                           It will also be NULL for bus
                           drivers that wish to retrieve the
                           name of the bus controller.  It will
                           not be NULL for a bus driver that
                           wishes to retrieve the name of a
                           child controller.

  \param  Language         A pointer to a Null-terminated ASCII
                           string array indicating the language.
                           This is the language of the driver
                           name that the caller is requesting,
                           and it must match one of the
                           languages specified in
                           SupportedLanguages. The number of
                           languages supported by a driver is up
                           to the driver writer. Language is
                           specified in RFC 4646 language code
                           format.

  \param  ControllerName   A pointer to the string to return.
                           This string is the name of the controller
                           specified by ControllerHandle and ChildHandle
                           in the language specified by Language
                           from the point of view of the driver
                           specified by This.

  \return EFI_SUCCESS           The string for the user
                                readable name in the language
                                specified by Language for the
                                driver specified by This was
                                returned in DriverName.

  \return EFI_INVALID_PARAMETER ControllerHandle is NULL.

  \return EFI_INVALID_PARAMETER ChildHandle is not NULL and it
                                is not a valid EFI_HANDLE.

  \return EFI_INVALID_PARAMETER Language is NULL.

  \return EFI_INVALID_PARAMETER ControllerName is NULL.

  \return EFI_UNSUPPORTED       The driver specified by This is
                                not currently managing the
                                controller specified by
                                ControllerHandle and
                                ChildHandle.
**/
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

/**
  \brief	Read BufferSize bytes from Lba into Buffer.

  \param  This       Indicates a pointer to the calling context.
  \param  MediaId    Id of the media, changes every time the media is replaced.
  \param  Lba        The starting Logical Block Address to read from
  \param  BufferSize Size of Buffer, must be a multiple of device block size.
  \param  Buffer     A pointer to the destination buffer for the data. The caller is
                     responsible for either having implicit or explicit ownership of the buffer.

  \return EFI_SUCCESS           The data was read correctly from the device.
  \return EFI_DEVICE_ERROR      The device reported an error while performing the read.
  \return EFI_NO_MEDIA          There is no media in the device.
  \return EFI_MEDIA_CHANGED     The MediaId does not matched the current device.
  \return EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  \return EFI_INVALID_PARAMETER The read request contains LBAs that are not valid,
                                or the buffer is not on proper alignment.

**/
EFI_STATUS EFIAPI CsReadBlocks(
		IN EFI_BLOCK_IO		              *This,
		IN UINT32                         MediaId,
		IN EFI_LBA                        Lba,
		IN UINTN                          BufferSize,
		OUT VOID                          *Buffer
	) {

	EFI_STATUS error = EFI_SUCCESS;
	EFI_LBA block = Lba * context.factorMediaBlock;	/* block regards to the encryption/decryption unit,
													   Lba regards to the media unit */

	CS_DEBUG((D_INFO, L"CsReadBlocks() started (LBA 0x%lx).\n", Lba));

	if (context.ConsumedBlockIo == NULL) {
		CS_DEBUG((D_ERROR, L"CsReadBlocks() called, but no consumed BlockIo interface exists.\n"));
		return EFI_NOT_FOUND;
	}
	if (context.options.isHiddenVolume) {
		Lba += context.hiddenSectorOffset;
	}

	error = uefi_call_wrapper(context.ConsumedBlockIo->ReadBlocks, 5,
			context.ConsumedBlockIo, MediaId, Lba, BufferSize, Buffer);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"ConsumedBlockIo->ReadBlocks() failed (%r)\n", error));
		CS_DEBUG((D_ERROR, L"#Media %x, LBA %lx, buffersize %x, buffer %lx\n", MediaId, Lba, BufferSize, Buffer));
		return error;
	} else {

		CS_DEBUG((D_INFO, L"CsReadBlocks(ID=0x%x, LBA=0x%x, Size=0x%x) called.\n", MediaId, Lba, BufferSize));

		error = decryptBlocks(block, BufferSize, Buffer);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"decryptBlocks() failed (%r)\n", error));
		}
	}

	return error;
}

/**
  \brief	Write BufferSize bytes from Lba into Buffer.

  \param  This       Indicates a pointer to the calling context.
  \param  MediaId    The media ID that the write request is for.
  \param  Lba        The starting logical block address to be written. The caller is
                     responsible for writing to only legitimate locations.
  \param  BufferSize Size of Buffer, must be a multiple of device block size.
  \param  Buffer     A pointer to the source buffer for the data.

  \return EFI_SUCCESS           The data was written correctly to the device.
  \return EFI_WRITE_PROTECTED   The device can not be written to.
  \return EFI_DEVICE_ERROR      The device reported an error while performing the write.
  \return EFI_NO_MEDIA          There is no media in the device.
  \return EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
  \return EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  \return EFI_INVALID_PARAMETER The write request contains LBAs that are not valid,
                                or the buffer is not on proper alignment.

**/
EFI_STATUS EFIAPI CsWriteBlocks(
		IN EFI_BLOCK_IO		              *This,
		IN UINT32                         MediaId,
		IN EFI_LBA                        Lba,
		IN UINTN                          BufferSize,
		IN VOID                           *Buffer
	) {

	EFI_STATUS error;
	EFI_LBA block = Lba * context.factorMediaBlock;	/* block regards to the encryption/decryption unit,
													   Lba regards to the media unit */

	CS_DEBUG((D_INFO, L"CsWriteBlocks(ID=0x%x, LBA=0x%lx, Size=0x%x) called.\n", MediaId, Lba, BufferSize));

	if (context.ConsumedBlockIo == NULL) {
		CS_DEBUG((D_ERROR, L"CsWriteBlocks() called, but no consumed BlockIo interface exists.\n"));
		return EFI_NOT_FOUND;
	}

	error = encryptBlocks(block, BufferSize, Buffer);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"encryptBlocks() failed (%r)\n", error));
		return error;
	}
	if (context.options.isHiddenVolume) {
		Lba += context.hiddenSectorOffset;
	}

	error = uefi_call_wrapper(context.ConsumedBlockIo->WriteBlocks, 5,
			context.ConsumedBlockIo, MediaId, Lba, BufferSize, Buffer);

	/* to recover the original buffer content: */
	error = decryptBlocks(block, BufferSize, Buffer);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"decryptBlocks() failed (%r)\n", error));
	}

	return error;
}

/**
  \brief	Flush the Block Device.

  \param  This              Indicates a pointer to the calling context.

  \return EFI_SUCCESS       All outstanding data was written to the device
  \return EFI_DEVICE_ERROR  The device reported an error while writting back the data
  \return EFI_NO_MEDIA      There is no media in the device.

**/
EFI_STATUS EFIAPI CsFlushBlocks(
		IN EFI_BLOCK_IO		*This
	) {

	if (context.ConsumedBlockIo == NULL) {
		CS_DEBUG((D_ERROR, L"CsFlushBlocks() called, but no consumed BlockIo interface exists.\n"));
		return EFI_NOT_FOUND;
	}

	return uefi_call_wrapper(context.ConsumedBlockIo->FlushBlocks, 1, context.ConsumedBlockIo);
}

/**
  \brief	Reset the Block Device.

  \param  This                 Indicates a pointer to the calling context.
  \param  ExtendedVerification Driver may perform diagnostics on reset.

  \return EFI_SUCCESS          The device was reset.
  \return EFI_DEVICE_ERROR     The device is not functioning properly and could
                               not be reset.

**/
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

/**
  \brief	Reset the block device hardware.

  \param[in]  This                 Indicates a pointer to the calling context.
  \param[in]  ExtendedVerification Indicates that the driver may perform a more
                                   exhausive verfication operation of the device
                                   during reset.

  \return EFI_SUCCESS          The device was reset.
  \return EFI_DEVICE_ERROR     The device is not functioning properly and could
                               not be reset.

**/
EFI_STATUS EFIAPI CsResetEx(
		IN EFI_BLOCK_IO2_PROTOCOL  *This,
		IN BOOLEAN                 ExtendedVerification
	) {

	if (context.ConsumedBlockIo2 == NULL) {
		CS_DEBUG((D_ERROR, L"CsResetEx() called, but no consumed BlockIo2 interface exists.\n"));
		return EFI_NOT_FOUND;
	}

	return uefi_call_wrapper(context.ConsumedBlockIo2->Reset, 2, context.ConsumedBlockIo2, ExtendedVerification);
}


/**
  \brief	Read BufferSize bytes from Lba into Buffer.

  This function reads the requested number of blocks from the device. All the
  blocks are read, or an error is returned.
  If EFI_DEVICE_ERROR, EFI_NO_MEDIA,_or EFI_MEDIA_CHANGED is returned and
  non-blocking I/O is being used, the Event associated with this request will
  not be signaled.

  \param[in]       This       Indicates a pointer to the calling context.
  \param[in]       MediaId    Id of the media, changes every time the media is
                              replaced.
  \param[in]       Lba        The starting Logical Block Address to read from.
  \param[in, out]  Token            A pointer to the token associated with the transaction.
  \param[in]       BufferSize Size of Buffer, must be a multiple of device block size.
  \param[out]      Buffer     A pointer to the destination buffer for the data. The
                              caller is responsible for either having implicit or
                              explicit ownership of the buffer.

  \return EFI_SUCCESS           The read request was queued if Token->Event is
                                not NULL.The data was read correctly from the
                                device if the Token->Event is NULL.
  \return EFI_DEVICE_ERROR      The device reported an error while performing
                                the read.
  \return EFI_NO_MEDIA          There is no media in the device.
  \return EFI_MEDIA_CHANGED     The MediaId is not for the current media.
  \return EFI_BAD_BUFFER_SIZE   The BufferSize parameter is not a multiple of the
                                intrinsic block size of the device.
  \return EFI_INVALID_PARAMETER The read request contains LBAs that are not valid,
                                or the buffer is not on proper alignment.
  \return EFI_OUT_OF_RESOURCES  The request could not be completed due to a lack
                                of resources.
**/
EFI_STATUS EFIAPI CsReadBlocksEx(
		IN     EFI_BLOCK_IO2_PROTOCOL *This,
		IN     UINT32                 MediaId,
		IN     EFI_LBA                Lba,
		IN OUT EFI_BLOCK_IO2_TOKEN    *Token,
		IN     UINTN                  BufferSize,
		OUT VOID                      *Buffer
	) {

	EFI_STATUS error;
	EFI_LBA block = Lba * context.factorMediaBlock;	/* block regards to the encryption/decryption unit,
													   Lba regards to the media unit */

    ASSERT(Buffer != NULL);

	CS_DEBUG((D_INFO, L"CsReadBlocksEx() started (LBA 0x%lx).\n", Lba));

	if (context.ConsumedBlockIo2 == NULL) {
		CS_DEBUG((D_ERROR, L"CsReadBlocksEx() called, but no consumed BlockIo2 interface exists.\n"));
		return EFI_NOT_FOUND;
	}

	if ((Token == NULL) || (Token->Event == NULL)) /* Blocking */ {

		if (context.options.isHiddenVolume) {
			Lba += context.hiddenSectorOffset;
		}

		error = uefi_call_wrapper(context.ConsumedBlockIo2->ReadBlocksEx, 6,
				context.ConsumedBlockIo2, MediaId, Lba, Token, BufferSize, Buffer);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"ConsumedBlockIo2->ReadBlocksEx() failed (%r)\n", error));
			return error;
		} else {
			CS_DEBUG((D_INFO, L"CsReadBlocksEx(ID=0x%x, Lba=0x%x, Size=0x%x, Blocking) called.\n",
					MediaId, Lba, BufferSize));

			error = decryptBlocks(block, BufferSize, Buffer);
			if (EFI_ERROR(error)) {
				CS_DEBUG((D_ERROR, L"decryptBlocks() failed (%r)\n", error));
			}
		}
	} else /* Non-Blocking */ {
		CS_READ_BLOCKS_SUBTASK *Subtask;

		Subtask = csCreateSubtask(BufferSize, Buffer, MediaId, block, Token->Event);
		if (Subtask == NULL) {
			return EFI_OUT_OF_RESOURCES;
		}

		if (context.options.isHiddenVolume) {
			Lba = Subtask->Lba + context.hiddenSectorOffset;
		}

		error = uefi_call_wrapper(context.ConsumedBlockIo2->ReadBlocksEx, 6,
				context.ConsumedBlockIo2, MediaId, Lba, &Subtask->Token, Subtask->BufferSize, Subtask->Buffer);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"ReadBlocksEx(non-blocking) failed (%r)\n", error));
			_csDestroySubtask(Subtask);
			return error;
		}
		/*
		 * das ist nicht so trivial:
		 * - subtask definieren
		 * - event vom Aufrufer merken
		 * - subtask in quere hängen
		 * - neues event definieren, callback funktion setzen, parameter der callback definieren
		 * - in callback: subtask aus liste löschen, puffer entschlüsseln, callback löschen
		 *   -> und alten callback/event signalisieren
		 * - beim stop des Treibers: löschen/freigeben aller noch laufenden subtasks
		 * - beispiel-implementierung siehe MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIo.c
		 */
	}

	return error;
}

/**
  \brief	Write BufferSize bytes from Lba into Buffer.

  This function writes the requested number of blocks to the device. All blocks
  are written, or an error is returned. If EFI_DEVICE_ERROR, EFI_NO_MEDIA,
  EFI_WRITE_PROTECTED or EFI_MEDIA_CHANGED is returned and non-blocking I/O is
  being used, the Event associated with this request will not be signaled.
  Attention: this function does not recover the buffer content: it will be overwritten
  with the encrypted content!

  \param[in]       This       Indicates a pointer to the calling context.
  \param[in]       MediaId    The media ID that the write request is for.
  \param[in]       Lba        The starting logical block address to be written. The
                              caller is responsible for writing to only legitimate
                              locations.
  \param[in, out]  Token      A pointer to the token associated with the transaction.
  \param[in]       BufferSize Size of Buffer, must be a multiple of device block size.
  \param[in]       Buffer     A pointer to the source buffer for the data (content may be modified!).

  \return EFI_SUCCESS           The write request was queued if Event is not NULL.
                                The data was written correctly to the device if
                                the Event is NULL.
  \return EFI_WRITE_PROTECTED   The device can not be written to.
  \return EFI_NO_MEDIA          There is no media in the device.
  \return EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
  \return EFI_DEVICE_ERROR      The device reported an error while performing the write.
  \return EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  \return EFI_INVALID_PARAMETER The write request contains LBAs that are not valid,
                                or the buffer is not on proper alignment.
  \return EFI_OUT_OF_RESOURCES  The request could not be completed due to a lack
                                of resources.

**/
EFI_STATUS EFIAPI CsWriteBlocksEx(
		IN     EFI_BLOCK_IO2_PROTOCOL  *This,
		IN     UINT32                 MediaId,
		IN     EFI_LBA                Lba,
		IN OUT EFI_BLOCK_IO2_TOKEN    *Token,
		IN     UINTN                  BufferSize,
		IN OUT VOID                   *Buffer /* attention: buffer content is changed to encrypted content! */
	) {

	EFI_STATUS error;
	EFI_LBA block = Lba * context.factorMediaBlock;	/* block regards to the encryption/decryption unit,
													   Lba regards to the media unit */
	ASSERT(Buffer != NULL);

	CS_DEBUG((D_INFO, L"CsWriteBlocksEx() started (LBA 0x%lx).\n", Lba));

	if (context.ConsumedBlockIo2 == NULL) {
		CS_DEBUG((D_ERROR, L"CsWriteBlocksEx() called, but no consumed BlockIo2 interface exists.\n"));
		return EFI_NOT_FOUND;
	}
	/* EFI does not support multithreading, hence the encryption cannot be sent to the background... */

	error = encryptBlocks(block, BufferSize, Buffer);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"encryptBlocks() failed (%r)\n", error));
		return error;
	}

	if (context.options.isHiddenVolume) {
		Lba += context.hiddenSectorOffset;
	}

	return uefi_call_wrapper(context.ConsumedBlockIo2->WriteBlocksEx, 6,
			context.ConsumedBlockIo2, MediaId, Lba, Token, BufferSize, Buffer);
}

/**
  \brief	Flush the Block Device.

  If EFI_DEVICE_ERROR, EFI_NO_MEDIA,_EFI_WRITE_PROTECTED or EFI_MEDIA_CHANGED
  is returned and non-blocking I/O is being used, the Event associated with
  this request will not be signaled.

  \param[in]      This     Indicates a pointer to the calling context.
  \param[in,out]  Token    A pointer to the token associated with the transaction

  \return EFI_SUCCESS          The flush request was queued if Event is not NULL.
                               All outstanding data was written correctly to the
                               device if the Event is NULL.
  \return EFI_DEVICE_ERROR     The device reported an error while writting back
                               the data.
  \return EFI_WRITE_PROTECTED  The device cannot be written to.
  \return EFI_NO_MEDIA         There is no media in the device.
  \return EFI_MEDIA_CHANGED    The MediaId is not for the current media.
  \return EFI_OUT_OF_RESOURCES The request could not be completed due to a lack
                               of resources.

**/
EFI_STATUS EFIAPI CsFlushBlocksEx(
		IN     EFI_BLOCK_IO2_PROTOCOL   *This,
		IN OUT EFI_BLOCK_IO2_TOKEN      *Token
	) {

	if (context.ConsumedBlockIo2 == NULL) {
		CS_DEBUG((D_ERROR, L"CsFlushBlocksEx() called, but no consumed BlockIo2 interface exists.\n"));
		return EFI_NOT_FOUND;
	}

	return uefi_call_wrapper(context.ConsumedBlockIo2->FlushBlocksEx, 2, context.ConsumedBlockIo2, Token);
}

/**
	\brief	Uninstall EFI driver

	\param	ImageHandle Handle identifying the loaded image

	\return	Status EFI status code to return on exit
**/
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
	} else if (LoadedImage->LoadOptionsSize < sizeof(context.options)) {
	    CS_DEBUG((D_WARN, L"Attention: Option size of provided data too small.\n"));
		CopyMem(&context.options, LoadedImage->LoadOptions, LoadedImage->LoadOptionsSize);
	} else {
		cs_print_msg(L"Attention: option size of provided data too big (got 0x%x, max 0x%x)!\n",
				LoadedImage->LoadOptionsSize, sizeof(context.options));
		return EFI_INVALID_PARAMETER;
	}
	EFIDebug = context.options.debug;
	context.EndSector = context.options.StartSector + context.options.SectorCount;

	CS_DEBUG((D_INFO, L"driver init: StartSector 0x%x, SectorCount 0x%x\n",
			context.options.StartSector, context.options.SectorCount));
	CS_DEBUG((D_INFO, L"driver init: hiddenVolumePresent %x, Encryption Algorithm ID/mode: 0x%x/0x%x\n",
			context.options.isHiddenVolume, context.options.cipher.algo, context.options.cipher.mode));

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

/**
  Install EFI driver - Will be the entrypoint for the driver executable
  http://wiki.phoenix.com/wiki/index.php/EFI_IMAGE_ENTRY_POINT

  \param	ImageHandle Handle identifying the loaded image
  \param	SystemTable Pointers to EFI system calls

  \return	Status EFI status code to return on exit
**/
EFI_STATUS EFIAPI CsDriverInstall(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE* SystemTable) {

	EFI_STATUS error;
	EFI_LOADED_IMAGE *LoadedImage = NULL;

	ASSERT(ImageHandle != NULL);
	ASSERT(SystemTable != NULL);

    InitializeLib(ImageHandle, SystemTable);
    //InitializeUnicodeSupport((CHAR8 *)"eng");
    SetMem(&context, sizeof(context), 0);
    context.factorMediaBlock = 1;

	//EFIDebug = D_ERROR | D_WARN | D_LOAD | D_BLKIO | D_INIT | D_INFO;

    CS_DEBUG((D_INFO, L"CS driver install started.\n"));

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
