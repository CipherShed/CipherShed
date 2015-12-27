/*  cs_driver.h - CipherShed EFI boot loader
 *
 *
 *
 */

#ifndef _CS_DRIVER_H_
#define _CS_DRIVER_H_

#include "cs_common.h"

/**
  Install EFI driver - Will be the entrypoint for the driver executable
  http://wiki.phoenix.com/wiki/index.php/EFI_IMAGE_ENTRY_POINT

  \param	ImageHandle Handle identifying the loaded image
  \param	SystemTable Pointers to EFI system calls

  \return	Status EFI status code to return on exit
**/
EFI_STATUS EFIAPI CsDriverInstall(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

/**
	\brief	Uninstall EFI driver

	\param	ImageHandle Handle identifying the loaded image

	\return	Status EFI status code to return on exit
**/
EFI_STATUS EFIAPI CsDriverUninstall(EFI_HANDLE ImageHandle);

/**
  \brief	Test to see if this driver supports ControllerHandle.

  This service is called by the EFI boot service ConnectController(). In order
  to make drivers as small as possible, there are a few calling restrictions for
  this service. ConnectController() must follow these calling restrictions.
  If any other agent wishes to call Supported() it must also follow these
  calling restrictions.

  \param  This                Protocol instance pointer.
  \param  ControllerHandle    Handle of device to test
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
  );

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
                               device to start.

  \return EFI_SUCCESS          This driver is added to ControllerHandle
  \return EFI_ALREADY_STARTED  This driver is already running on ControllerHandle
  \return other                This driver does not support this device

**/
EFI_STATUS EFIAPI CsDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
  );

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
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer   OPTIONAL
  );

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
  );

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
  );

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
  );

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
  );

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
  IN EFI_BLOCK_IO		            *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  OUT VOID                          *Buffer
  );

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
  IN EFI_BLOCK_IO		            *This,
  IN UINT32                         MediaId,
  IN EFI_LBA                        Lba,
  IN UINTN                          BufferSize,
  IN VOID                           *Buffer
  );

/**
  \brief	Flush the Block Device.

  \param  This              Indicates a pointer to the calling context.

  \return EFI_SUCCESS       All outstanding data was written to the device
  \return EFI_DEVICE_ERROR  The device reported an error while writting back the data
  \return EFI_NO_MEDIA      There is no media in the device.

**/
EFI_STATUS EFIAPI CsFlushBlocks(
  IN EFI_BLOCK_IO		*This
  );

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
  );

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
  );

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
  IN     EFI_LBA                LBA,
  IN OUT EFI_BLOCK_IO2_TOKEN    *Token,
  IN     UINTN                  BufferSize,
     OUT VOID                  *Buffer
  );

/**
  \brief	Write BufferSize bytes from Lba into Buffer.

  This function writes the requested number of blocks to the device. All blocks
  are written, or an error is returned.If EFI_DEVICE_ERROR, EFI_NO_MEDIA,
  EFI_WRITE_PROTECTED or EFI_MEDIA_CHANGED is returned and non-blocking I/O is
  being used, the Event associated with this request will not be signaled.

  \param[in]       This       Indicates a pointer to the calling context.
  \param[in]       MediaId    The media ID that the write request is for.
  \param[in]       Lba        The starting logical block address to be written. The
                              caller is responsible for writing to only legitimate
                              locations.
  \param[in, out]  Token      A pointer to the token associated with the transaction.
  \param[in]       BufferSize Size of Buffer, must be a multiple of device block size.
  \param[in]       Buffer     A pointer to the source buffer for the data.

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
  IN     EFI_LBA                LBA,
  IN OUT EFI_BLOCK_IO2_TOKEN    *Token,
  IN     UINTN                  BufferSize,
  IN     VOID                   *Buffer
  );

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
  );




#endif /* _CS_DRIVER_H_ */
