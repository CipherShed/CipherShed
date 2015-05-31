/*  cs_controller.c - CipherShed EFI boot loader
 *  main file for the CipherShed controller application
 *
 *
 */

#include <efi.h>
#include <efilib.h>
#include <efibind.h>

#include "cs_controller.h"
#include "cs_options.h"
#include "cs_crypto.h"
#include "cs_debug.h"
#include "cs_ui.h"

#define CS_VOLUME_HEADER_DIRECTORY	L"volume"	/* directory name containing the volume header files */
#ifdef X86_64
#define CS_CRYPTO_DRIVER_NAME		L"CsDrv_64.efi"	/* filename of the crypto driver (64 bit version) */
#else
#define CS_CRYPTO_DRIVER_NAME		L"CsDrv_32.efi"	/* filename of the crypto driver (32 bit version) */
#endif
#ifdef CS_FAT_SHORT_NAMES
#define CS_VH_INDEX_FILE			L"index"	/* filename for index for volume header file(s) */
#endif
#define CS_MAX_PASSWORD_LENGTH		64			/* taken from TrueCrypt for compatibility reason */
#define CS_BOOT_LOADER_ARGS_OFFSET  0x10		/* taken from TrueCrypt for compatibility reason */
#define CS_MAX_LOAD_OPTIONS			32			/* maximum number of command line arguments */


struct disk_info {
	UINT8 mbr_type;				/* see efidevp.h for encoding */
	UINT8 signature_type;		/* see efidevp.h for encoding */
	union {
		UINT32 mbr_id;
		EFI_GUID guid;
	} signature;
	EFI_HANDLE handle;			/* the corresponding disk handle */
};

/*
 * global system context containing all important settings and data for the application
 */
static struct cs_system_context {
	UINTN argc;				/* actual number of command line options */
	CHAR16 *argv[CS_MAX_LOAD_OPTIONS];	/* pointer array to the NULL terminated command line options */
	EFI_FILE_HANDLE root_dir;			/* file protocol interface for root filesystem of EFI system partition */
	CHAR16 *dest_uuid;		/* UUID of the device that the driver shall connect to */
	CHAR16 uuid_buffer[CS_LENGTH_FILENAME_VOLUMNE_HEADER + 1];	/* UUID of the encrypted HDD partition
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	   in case that none is given via command line */
	CHAR16 *os_loader_uuid;	/* String containing the UUID of the device that contains the OS loader file */
	CHAR16 *os_loader;		/* String containing the full path to the OS loader file */
	UINTN os_loader_option_number;	/* number of options for the OS loader */
	CHAR16 **os_loader_options; /* pointer into the array argv[] where the OS loader options start */
	CHAR16 cs_driver_path[CS_MAX_DRIVER_PATH_SIZE];
	CHAR16 vh_path[CS_MAX_DRIVER_PATH_SIZE];	/* path to volume header */
	struct cs_cipher_data volume_header_protection;	/* cipher data and key context for volume header encryption */
	struct cs_option_data user_defined_options;	/* options taken from options file */
	struct disk_info caller_disk;	/* the storage media information of the media containing this application */
	struct disk_info os_loader_disk;	/* the storage media information of the media containing the EFI OS loader */
	struct disk_info boot_partition;	/* the storage media information of the (encrypted) boot partition */
	struct cs_driver_data os_driver_data;	/* data to be handed over to the OS driver */
	struct cs_efi_option_data efi_driver_data;	/* data to be handed over to the CipherShed EFI driver */
} context;


/*
 *  \brief	Convert a String to GUID Value
 *
 *	\param	Str        Specifies the String to be converted
 *	\param	StrLen     Number of Unicode Characters of String (exclusive \0)
 *	\param	Guid       Return the result Guid value.
 *
 *	\return		the success state of the function
 *
 */
static EFI_STATUS StringToGuid (IN CHAR16 *Str, IN UINTN StrLen, OUT EFI_GUID *Guid) {
  CHAR16             *PtrBuffer;
  CHAR16             *PtrPosition;
  UINT16             *Buffer;
  UINTN              Data;
  UINTN              Index;
  UINT16             Digits[3];

  Buffer = (CHAR16 *) AllocateZeroPool (sizeof (CHAR16) * (StrLen + 1));
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  StrCpy (Buffer, Str);

  //
  // Data1
  //
  PtrBuffer       = Buffer;
  PtrPosition     = PtrBuffer;
  while (*PtrBuffer != L'\0') {
    if (*PtrBuffer == L'-') {
      break;
    }
    PtrBuffer++;
  }
  if (*PtrBuffer == L'\0') {
    FreePool (Buffer);
    return EFI_NOT_FOUND;
  }

  *PtrBuffer      = L'\0';
  Data            = xtoi (PtrPosition);
  Guid->Data1     = (UINT32)Data;

  //
  // Data2
  //
  PtrBuffer++;
  PtrPosition     = PtrBuffer;
  while (*PtrBuffer != L'\0') {
    if (*PtrBuffer == L'-') {
      break;
    }
    PtrBuffer++;
  }
  if (*PtrBuffer == L'\0') {
    FreePool (Buffer);
    return EFI_NOT_FOUND;
  }
  *PtrBuffer      = L'\0';
  Data            = xtoi (PtrPosition);
  Guid->Data2     = (UINT16)Data;

  //
  // Data3
  //
  PtrBuffer++;
  PtrPosition     = PtrBuffer;
  while (*PtrBuffer != L'\0') {
    if (*PtrBuffer == L'-') {
      break;
    }
    PtrBuffer++;
  }
  if (*PtrBuffer == L'\0') {
    FreePool (Buffer);
    return EFI_NOT_FOUND;
  }
  *PtrBuffer      = L'\0';
  Data            = xtoi (PtrPosition);
  Guid->Data3     = (UINT16)Data;

  //
  // Data4[0..1]
  //
  for ( Index = 0 ; Index < 2 ; Index++) {
    PtrBuffer++;
    if ((*PtrBuffer == L'\0') || ( *(PtrBuffer + 1) == L'\0')) {
      FreePool (Buffer);
      return EFI_NOT_FOUND;
    }
    Digits[0]     = *PtrBuffer;
    PtrBuffer++;
    Digits[1]     = *PtrBuffer;
    Digits[2]     = L'\0';
    Data          = xtoi (Digits);
    Guid->Data4[Index] = (UINT8)Data;
  }

  //
  // skip the '-'
  //
  PtrBuffer++;
  if ((*PtrBuffer != L'-' ) || ( *PtrBuffer == L'\0')) {
    return EFI_NOT_FOUND;
  }

  //
  // Data4[2..7]
  //
  for ( ; Index < 8; Index++) {
    PtrBuffer++;
    if ((*PtrBuffer == L'\0') || ( *(PtrBuffer + 1) == L'\0')) {
      FreePool (Buffer);
      return EFI_NOT_FOUND;
    }
    Digits[0]     = *PtrBuffer;
    PtrBuffer++;
    Digits[1]     = *PtrBuffer;
    Digits[2]     = L'\0';
    Data          = xtoi (Digits);
    Guid->Data4[Index] = (UINT8)Data;
  }

  FreePool (Buffer);

  return EFI_SUCCESS;
}

/*
 *	\brief	return the handle of the boot partition
 *
 *	The handle is taken from the system context and only available after calling
 *	connect_crypto_driver(). In case that the handle is not yet available, the
 *	function returns NULL.
 *
 *	\return		the requested handle (might be NULL)
 */
EFI_HANDLE get_boot_partition_handle() {
	return context.boot_partition.handle;
}

/*
 *	\brief	return the pointer to the media cipher information of the boot media
 *
 *	The handle is taken from the system context and only valid after calling
 *	decrypt_volume_header().
 *
 *	\return		the requested pointer
 */
CRYPTO_INFO *get_crypto_info() {
	return &context.os_driver_data.crypto_info;
}

/*
 *	\brief	store the volume header cipher data in the system context for later usage
 *
 *
 *	\return		the requested pointer
 */
void set_volume_header_cipher(IN PCRYPTO_INFO cryptoInfo) {

	ASSERT(cryptoInfo != NULL);
	context.volume_header_protection.algo = cryptoInfo->ea;
	context.volume_header_protection.mode = cryptoInfo->mode;
	CopyMem(&context.volume_header_protection.ks, &cryptoInfo->ks,
			sizeof(context.volume_header_protection.ks));
	CopyMem(&context.volume_header_protection.ks2, &cryptoInfo->ks2,
			sizeof(context.volume_header_protection.ks2));
}

/*
 *	\brief	read a file from the hard disk (FAT file system)
 *
 *	The file content is written into a returned buffer. The buffer is allocated
 *	inside this function, hence it must be freed using FreePool() by the caller!
 *
 *	\param	root_handle	opened handle of the file system
 *	\param	filename	string with the full path of the file from the root directory
 *	\param	content		pointer to the buffer for the file content
 *
 *	\return		size of the buffer (size of the copied file content)
 */
UINTN read_file(IN EFI_FILE_HANDLE root_handle, IN CHAR16 *filename, OUT CHAR8 **content) {
    EFI_FILE_HANDLE handle;
    EFI_STATUS error;
    EFI_FILE_INFO *info;
    CHAR8 *buf;
    UINTN buflen;
    UINTN len = 0;

    ASSERT(root_handle != NULL);
    ASSERT(filename != NULL);
    ASSERT(content != NULL);

    error = uefi_call_wrapper(root_handle->Open, 5, root_handle, &handle, filename, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"Unable to open file \"%s\" (%r)\n", filename, error));
            goto out;
    }

    info = LibFileInfo(handle);
    buflen = info->FileSize+1;
    buf = AllocatePool(buflen);

    if (buf) {
    	error = uefi_call_wrapper(handle->Read, 3, handle, &buflen, buf);
		if (EFI_ERROR(error) == EFI_SUCCESS) {
				buf[buflen] = '\0';
				*content = buf;
				len = buflen;
				CS_DEBUG((D_INFO, L"Read 0x%x byte from file %s\n", len, filename));
		} else {
				FreePool(buf);
				*content = NULL;
				CS_DEBUG((D_ERROR, L"Unable to read file %s\n", filename));
		}
	}
    else {
		CS_DEBUG((D_ERROR, L"Unable to allocate buffer (0x%x byte)\n", buflen));
		error = EFI_BUFFER_TOO_SMALL;
    }

    FreePool(info);
    uefi_call_wrapper(handle->Close, 1, handle);
out:
    return len;
}

/*
 *	\brief	write the given content to a file
 *
 *	The given content is written into a file defined by filename.
 *
 *	\param	root_handle	opened handle of the file system
 *	\param	filename	string with the full path of the file from the root directory
 *	\param	content		pointer to the buffer for the file content
 *	\param	size		size of the buffer to write
 *
 *	\return		the success state of the function
 */
static EFI_STATUS write_file(IN EFI_FILE_HANDLE root_handle, IN CHAR16 *filename,
		IN CHAR8 *content, IN UINTN size) {
    EFI_FILE_HANDLE handle;
    EFI_STATUS error;

    ASSERT(root_handle != NULL);
    ASSERT(filename != NULL);
    ASSERT(content != NULL);

    error = uefi_call_wrapper(root_handle->Open, 5, root_handle, &handle, filename,
#ifdef CS_TEST_CREATE_VOLUME_HEADER
    		EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
#else
    		EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
#endif
    		 0);
    if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"Unable to open file \"%s\" (%r)\n", filename, error));
            goto out;
    }

	error = uefi_call_wrapper(handle->Write, 3, handle, &size, content);
	if (EFI_ERROR(error) == EFI_SUCCESS) {
			CS_DEBUG((D_INFO, L"Wrote 0x%x byte to file %s\n", size, filename));
	} else {
			CS_DEBUG((D_ERROR, L"Unable to write file %s\n", filename));
	}

    uefi_call_wrapper(handle->Flush, 1, handle);
    uefi_call_wrapper(handle->Close, 1, handle);
out:
    return error;
}

/*
 *	initialize (global) system context with default values
 */
static void init_system_context() {
	SetMem(&context, sizeof(context), 0);
	context.user_defined_options.driverdebug = D_ERROR;
	context.user_defined_options.flags.silent = 1;
}

/*
 *	\brief	detects directory of the currently executed application in the file system
 *
 *	The buffer for the returned directory is allocated inside the function, hence the caller
 *	need to free this buffer using FreePool().
 *
 *	\param	loaded_image	opened image
 *	\param	current_dir		pointer to buffer for the returned directory name
 *
 *	\return		the success state of the function
 */
static EFI_STATUS get_current_directory(IN EFI_LOADED_IMAGE *loaded_image, OUT CHAR16** current_dir) {

	CHAR16 *current_file;
	EFI_STATUS error = EFI_SUCCESS;
	UINTN i;

    ASSERT(loaded_image != NULL);
    ASSERT(current_dir != NULL);

    current_file = DevicePathToStr(loaded_image->FilePath);
	if (current_file == NULL) {
		CS_DEBUG((D_ERROR, L"Unable to translate path to string\n"));
		return EFI_NO_MAPPING;
	}

	CS_DEBUG((D_INFO, L"Path to current Application: %s\n", current_file));

	for (i = StrLen(current_file); i >= 0; i--) {
		if (current_file[i] == '\\') {

			*current_dir = AllocatePool(i + 2);
			if (*current_dir) {
				UINTN j;

				for (j = 0; j <= i; j++) {
					(*current_dir)[j] = current_file[j];
				}
				(*current_dir)[i+1] = 0;

				CS_DEBUG((D_INFO, L"Current Directory: \"%s\"\n", *current_dir));
			} else {
				CS_DEBUG((D_ERROR, L"Unable to allocate directory buffer (0x%x)\n", i + 2));
				error = EFI_OUT_OF_RESOURCES;
			}
			break;
		}
	}

	return error;
}

/*
 *	\brief	initialize context.boot_partition
 *
 *	The function initializes the global variable context.boot_partition with default values. The only
 *	precondition is that context.dest_uuid is set.
 *
 *	\return		the success state of the function
 */
static EFI_STATUS init_boot_partition() {
	EFI_STATUS error;

	/* here only GPT partitions are supported (yet)... */
	context.boot_partition.mbr_type = MBR_TYPE_EFI_PARTITION_TABLE_HEADER; /* not sure whether this is correct */
	context.boot_partition.signature_type = SIGNATURE_TYPE_GUID;
	error = StringToGuid(context.dest_uuid, StrLen(context.dest_uuid),
			&context.boot_partition.signature.guid);
	if (EFI_ERROR(error))  {
		CS_DEBUG((D_ERROR, L"Unable to convert the string %s to a GUID: %r", context.dest_uuid, error));
		return error;
	}
	CS_DEBUG((D_INFO, L"BOOT PARTITION: %s\n", context.dest_uuid));

	return EFI_SUCCESS;
}

#ifdef CS_FAT_SHORT_NAMES
/*
 *	\brief	write the complete volume header path to context.vh_path
 *
 *
 *	\param	buffer	buffer containing the content of the index file
 *	\param	start_filename	position in buffer with the volume header filename
 *	\param	pathname	pathname to the file where the volume header is stored
 *
 *	\return		the success state of the function
 */
static EFI_STATUS _write_vh_path(IN CHAR8 *buffer, IN UINTN start_filename, IN CHAR16 *pathname) {
	EFI_STATUS error;
	UINTN needed_pathlen;

	needed_pathlen = StrLen(pathname) + StrLen((const CHAR16 *)L"\\") +
			strlena(&buffer[start_filename]) + 1;
	if ((needed_pathlen * sizeof(CHAR16)) <= sizeof(context.vh_path)) {
		int j, k;
		StrCpy(&context.vh_path[0], pathname);
		StrCat(&context.vh_path[0], (const CHAR16 *)L"\\");
		k = StrLen(&context.vh_path[0]);
		for (j = 0; j < strlena(&buffer[start_filename]); j++) {
			context.vh_path[k + j] = (CHAR16)buffer[start_filename + j];
		}
		context.vh_path[k + j] = (CHAR16)0;

		CS_DEBUG((D_INFO, L"pathname is: %s\n", pathname));
		CS_DEBUG((D_INFO, L"Volume Header Filename is: %s\n", &context.vh_path[0]));

		error = EFI_SUCCESS;
	} else {
		CS_DEBUG((D_ERROR, L"buffer too small for volume header path (0x%x/0x%x byte)\n",
				needed_pathlen * sizeof(CHAR16), sizeof(context.vh_path)));
		error = EFI_BUFFER_TOO_SMALL;
	}

	return error;
}

/*
 *	\brief	parse the content of the index file
 *
 *  The function parses the content of the file containing the assignment between
 *  the UUID of the encrypted partition and the filename of the volume header file.
 *  This index file content is given in the buffer with the given size.
 *  In case that context.dest_uuid is defined, the buffer is scanned for that given
 *  UUID, otherwise the first entry will be taken and the UUID of this entry will
 *  be written to context.dest_uuid. In case of success, the value context.vh_path
 *  will be filled with the full path to the file with the volume header.
 *
 *	\param	buffer	buffer containing the content of the index file
 *	\param	size	size of the buffer
 *	\param	pathname	pathname to the file where the volume header is stored
 *
 *	\return		the success state of the function
 */
static EFI_STATUS _parse_index_file(IN CHAR8 *buffer, IN UINTN size, IN CHAR16 *pathname) {
    EFI_STATUS error = EFI_NOT_READY;

	if (size) {
		UINTN next, index = 0;
		UINTN line_size;

		do {
			next = _get_next_line(&buffer[index], size, &line_size);
			line_size = _strip_line(&buffer[index], line_size);
			if (line_size) {
				if (context.dest_uuid != NULL) {
					if (StrLen(context.dest_uuid) < line_size) {
						int i;
						for (i = 0; i < StrLen(context.dest_uuid); i++) {
							if (context.dest_uuid[i] != buffer[index + i]) {
								break;
							}
						}
						if ((i >= StrLen(context.dest_uuid)) &&
								((buffer[index + i] == ' ') || (buffer[index + i] == '\t'))) {
							/* matching UUID found */
							int remaining_characters = line_size - i;
							CS_DEBUG((D_INFO, L"found line with matching UUID: \"%a\"\n", &buffer[index]));

							while ((remaining_characters > 0) &&
									((buffer[index + i] == ' ') || (buffer[index + i] == '\t'))) {
								i++;
								remaining_characters--;
							}
							CS_DEBUG((D_INFO, L"corresponding filename: \"%a\"\n", &buffer[index + i]));

							error = init_boot_partition();
							if (!EFI_ERROR(error))  {
								error = _write_vh_path(&buffer[index], i, pathname);
							}
							break;
						}
					}
				} else {
					/* no UUID given, so take the first entry from the file */
					int remaining_characters = line_size, i = 0;
					int end_uuid, start_filename;

					/* find the end of the UUID */
					while ((remaining_characters > 0) &&
							(buffer[index + i] != ' ') && (buffer[index + i] != '\t')) {
						i++;
						remaining_characters--;
					}
					end_uuid = i;
					/* find the begin of the filename */
					while ((remaining_characters > 0) &&
							((buffer[index + i] == ' ') || (buffer[index + i] == '\t'))) {
						i++;
						remaining_characters--;
					}
					start_filename = i;
					if (((end_uuid * sizeof(CHAR16)) <= sizeof(context.uuid_buffer)) &&
							(start_filename > end_uuid) &&
							(strlena(&buffer[index + start_filename]) > 0)) {

						for (i = 0; i < end_uuid; i++) {
							context.uuid_buffer[i] = (CHAR16)buffer[index + i];
						}
						context.uuid_buffer[i] = (CHAR16)0;
						context.dest_uuid = &context.uuid_buffer[0];
						CS_DEBUG((D_INFO, L"partition UUID: \"%s\"\n", context.dest_uuid));
						CS_DEBUG((D_INFO, L"vh filename: \"%a\"\n", &buffer[index + start_filename]));

						error = init_boot_partition();
						if (!EFI_ERROR(error))  {
							error = _write_vh_path(&buffer[index], start_filename, pathname);
						}
						break;
					}
				}
			}
			index += next;
			size -= next;
		} while (next > 0);

		FreePool(buffer);
	} else {
		/* this situation might be no error since the index file is not compulsory,
		 * in this case the volume header might be stored in the other way */
		CS_DEBUG((D_INFO, L"unable to read from index file \"%s\"\n", pathname));
		error = EFI_NO_MEDIA;
	}

    return error;
}

/*
 *	\brief	detect the name of the file containing the volume header using an index file
 *
 *	The function checks whether an index file is in the current directory. If not, the
 *	function returns with an error (EFI_NO_MEDIA), otherwise the file is opened
 *	and parsed. The file shall contain an assignment between an UUID
 *	(for the encrypted partition) and a filename (in the same directory as
 *	the index file) for the file containing the volume header.
 *	The structure of this assignment is: <UUID> <filename>
 *	Lines starting with "#" are ignored. The filename shall NOT contain a pathname.
 *	When a valid UUID is found, the corresponding filename will be stored
 *	together with its full path to context.vh_path.
 *
 *	\param	root_dir	opened handle to the file system root
 *	\param	current_dir	string containing the directory name of the volume header files
 *
 *	\return		the success state of the function
 */
static EFI_STATUS get_volume_header_file_short(IN EFI_FILE *root_dir, IN CHAR16 *current_dir) {
    EFI_STATUS error = EFI_NOT_READY;
    CHAR16 *indexfile_path;
    UINTN indexfile_path_length, len;
    CHAR8 *content_index_file;

    ASSERT(root_dir != NULL);
    ASSERT(current_dir != NULL);

    indexfile_path_length = StrLen(current_dir) + StrLen((const CHAR16 *)L"\\") +
    		StrLen((const CHAR16 *)CS_VH_INDEX_FILE) + 1;
    indexfile_path = AllocatePool(indexfile_path_length);

    if (indexfile_path == NULL) {
		CS_DEBUG((D_ERROR, L"Unable to allocate index path name buffer (0x%x byte)\n", indexfile_path_length));
		return EFI_BUFFER_TOO_SMALL;
    }
	StrCpy(indexfile_path, current_dir);
	StrCat(indexfile_path, (const CHAR16 *)L"\\");
	StrCat(indexfile_path, (const CHAR16 *)CS_VH_INDEX_FILE);

	len = read_file(root_dir, indexfile_path, &content_index_file);

    FreePool(indexfile_path);

    error = _parse_index_file(content_index_file, len, current_dir);

    return error;
}
#endif /* CS_FAT_SHORT_NAMES */

/*
 *	\brief	detect the name of the file containing the volume header
 *
 *	The function reads the filename containing the volume header. The file is expected to be
 *	in the directory CS_VOLUME_HEADER_DIRECTORY. This filename is expected to be the UUID
 *	of the intended (encrypted) hard disk partition (see context.dest_uuid).
 *	Its size is expected to be CS_LENGTH_FILENAME_VOLUMNE_HEADER. Other files are ignored.
 *
 *	\param	root_dir	opened handle to the file system root
 *	\param	current_dir	string containing the directory name of the executed application
 *
 *	\return		the success state of the function
 */
static EFI_STATUS get_volume_header_file(IN EFI_FILE *root_dir, IN CHAR16 *current_dir) {
    EFI_FILE_HANDLE entries_dir;
    EFI_STATUS error = EFI_NOT_READY;
    CHAR16 *directory_name;
    UINTN directory_length;

    ASSERT(root_dir != NULL);
    ASSERT(current_dir != NULL);

    directory_length = StrLen(current_dir) + StrLen((const CHAR16 *)CS_VOLUME_HEADER_DIRECTORY) + 1;
    directory_name = AllocatePool(directory_length);

    if (directory_name == NULL) {
		CS_DEBUG((D_ERROR, L"Unable to allocate directory name buffer (0x%x byte)\n", directory_length));
		return EFI_BUFFER_TOO_SMALL;
    }

	StrCpy(directory_name, current_dir);
	StrCat(directory_name, (const CHAR16 *)CS_VOLUME_HEADER_DIRECTORY);

#ifdef CS_FAT_SHORT_NAMES
	/* try alternative method to get the volume header file... */
	error = get_volume_header_file_short(root_dir, directory_name);
	if (EFI_ERROR(error) == EFI_SUCCESS) {
		/* header file found... */
	    FreePool(directory_name);
		return error;
	}
#endif

    error = uefi_call_wrapper(root_dir->Open, 5, root_dir, &entries_dir, directory_name, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(error) == EFI_SUCCESS) {
            for (;;) {
				CHAR16 directory_content[256];
				UINTN bufsize;
				EFI_FILE_INFO *f;
				UINTN len;

				bufsize = sizeof(directory_content);
				error = uefi_call_wrapper(entries_dir->Read, 3, entries_dir, &bufsize, directory_content);

				if (EFI_ERROR(error))  {
					CS_DEBUG((D_ERROR, L"Unable to Read Volume Directory %r", error));
					break;
				}
				if (bufsize == 0) {
					CS_DEBUG((D_ERROR, L"Unable to Read Volume Directory %s\n", directory_name));
					error = EFI_OUT_OF_RESOURCES;
					break;
				}

				f = (EFI_FILE_INFO *) directory_content;
				if (f->FileName[0] == '.')
						continue;
				if (f->Attribute & EFI_FILE_DIRECTORY)
				{
					CS_DEBUG((D_WARN, L"Found Directory in Volume Header directory %s\n", f->FileName));
					continue;
				}
				len = StrLen(f->FileName);

				if (context.dest_uuid != NULL) {
					if ((StrLen(context.dest_uuid) == len) && (StriCmp(context.dest_uuid, f->FileName) == 0)) {
						/* SUCCESS !! */
						CS_DEBUG((D_INFO, L"Success: required volume header %s found.\n", context.dest_uuid));

					} else {
						CS_DEBUG((D_WARN, L"No match: required UUID %s, found %s\n", context.dest_uuid, f->FileName));
						continue;
					}
				} else {
					/* no UUID given as command line option-> take the first volume header that seems correct... */

					if (len < CS_LENGTH_FILENAME_VOLUMNE_HEADER)
					{
						CS_DEBUG((D_WARN, L"Filename \"%s\" too short in Volume Header directory\n", f->FileName));
						continue;
					}
					if (len > CS_LENGTH_FILENAME_VOLUMNE_HEADER)
					{
						CS_DEBUG((D_WARN, L"Filename \"%s\" too long in Volume Header directory\n", f->FileName));
						continue;
					}
					if (f->FileSize != CS_VOLUME_HEADER_SIZE) {
						CS_DEBUG((D_WARN, L"Wrong File Size in Volume Header file %s\n", f->FileName));
						continue;
					}

					CS_DEBUG((D_INFO, L"Volume Header candidate found: %s\n", f->FileName));
					StrCpy(context.uuid_buffer, f->FileName);
					context.dest_uuid = &context.uuid_buffer[0];
				}

				error = init_boot_partition();
				if (EFI_ERROR(error)) {
					break;
				}

				len = StrLen(directory_name) + StrLen((const CHAR16 *)L"\\") + StrLen((const CHAR16 *)f->FileName) + 1;
				if ((len * sizeof(CHAR16)) <= sizeof(context.vh_path)) {
					StrCpy(&context.vh_path[0], directory_name);
					StrCat(&context.vh_path[0], (const CHAR16 *)L"\\");
					StrCat(&context.vh_path[0], (const CHAR16 *)f->FileName);

					CS_DEBUG((D_INFO, L"Volume Header Filename is: %s\n", &context.vh_path[0]));

				} else {
					CS_DEBUG((D_ERROR, L"buffer too small for volume header path (0x%x/0x%x byte)\n",
							len * sizeof(CHAR16), sizeof(context.vh_path)));
					error = EFI_BUFFER_TOO_SMALL;
					break;
				}
				break;	/* for now: stop here after the first volume header file */
            }
            uefi_call_wrapper(entries_dir->Close, 1, entries_dir);
    }
    FreePool(directory_name);

	return error;
}

/*
 *	\brief	read the content of the volume header from a file
 *
 *	The function reads the volume header file as defined by the given filename into the
 *	buffer in the (global) system context. The location of the volume header file is taken
 *	relative to the current directory, where the executed application is stored.
 *
 *	\param	root_dir	opened handle to the file system root
 *	\param	current_dir	string containing the directory name of the executed application
 *
 *	\return		the success state of the function
 */
static EFI_STATUS load_volume_header(IN EFI_FILE *root_dir, IN CHAR16 *current_dir) {
    EFI_STATUS error;
	CHAR8 *content_volume_header;
    UINTN len;

    ASSERT(root_dir != NULL);
    ASSERT(current_dir != NULL);

    error = get_volume_header_file(root_dir, current_dir);
    if (EFI_ERROR(error) == EFI_SUCCESS) {

		len = read_file(root_dir, &context.vh_path[0], &content_volume_header);
		if (len) {
			if (len == CS_VOLUME_HEADER_SIZE) {
				CS_DEBUG((D_INFO, L"Volume Header File %s correctly read\n", context.vh_path));

				CopyMem(&context.os_driver_data.volume_header,
						content_volume_header, sizeof(context.os_driver_data.volume_header));

				error = EFI_SUCCESS;
			} else {
				CS_DEBUG((D_ERROR, L"Invalid file size %s (0x%x)\n", context.vh_path, len));

				error = EFI_INVALID_PARAMETER;
			}
			FreePool(content_volume_header);
		} else {
			CS_DEBUG((D_ERROR, L"Unable to read file %s\n", context.vh_path));

			error = EFI_END_OF_FILE;
		}
    }

    return error;
}

/*
 *	\brief	store the path to the crypto driver in the system context
 *
 *	The function tries to get the full path to the crypto driver without verifying
 *	that the driver is really existing (this is done later when the driver is loaded).
 *	The result is stored for later usage in system context.
 *
 *	\param	current_dir	string containing the directory name of the executed application
 *
 *	\return		the success state of the function
 */
static EFI_STATUS get_driver_path(IN CHAR16 *current_dir) {
    EFI_STATUS error = EFI_BUFFER_TOO_SMALL;
    UINTN len;

    ASSERT(current_dir != NULL);

    len = StrLen(current_dir) + StrLen((const CHAR16 *)CS_CRYPTO_DRIVER_NAME);
    len *= sizeof(CHAR16);	 /* length in byte! */
	if (len <= sizeof(context.cs_driver_path)) {
    	StrCpy(context.cs_driver_path, current_dir);
    	StrCat(context.cs_driver_path, (const CHAR16 *)CS_CRYPTO_DRIVER_NAME);
		error = EFI_SUCCESS;
		CS_DEBUG((D_INFO, L"Driver path is: %s\n", context.cs_driver_path));
	}

	return error;
}

/*
 * \brief	check the device path for the given device handle
 *
 * This function scans the device path of the given device handle for storage device information
 * (hard disk device). If found, then the type of the MBR (MBR or GUID) is returned together with
 * the disk signature (MBR signature or GUID).
 *
 * \param	device_handle	the given device handle to check
 * \param	mbr_type		pointer to buffer for returned MBR type
 * \param	signature_type	pointer to buffer for returned MBR signature type
 * 							(either SIGNATURE_TYPE_MBR or SIGNATURE_TYPE_GUID)
 * \param	signature		pointer to buffer for returned signature value
 * 							(either type UINT32 or type EFI_GUID)
 *
 * \return		the success state of the function
 */
static EFI_STATUS get_disk_handle(IN EFI_HANDLE device_handle, OUT UINT8 *mbr_type,
		OUT UINT8 *signature_type, OUT void *signature) {

	EFI_STATUS error;
	EFI_DEVICE_PATH  *DevicePath;

    ASSERT(device_handle != NULL);
    ASSERT(mbr_type != NULL);
    ASSERT(signature_type != NULL);
    ASSERT(signature != NULL);

	error = uefi_call_wrapper(BS->HandleProtocol, 3, device_handle, &DevicePathProtocol, (void **)&DevicePath);
    if (EFI_ERROR(error) || (DevicePath == NULL)) {
		CS_DEBUG((D_ERROR, L"Error getting a DevicePathProtocol handle: %r\n", error));
    } else {
        /* the code is (partly) taken from EDK2 EFI shell source:
         *
         * Search DevicePath for a Hard Drive Media Device Path node.
         * If one is found, then see if it matches the signature that was
         * passed in.  If it does match, and the next node is the End of the
         * device path, and the previous node is not a Hard Drive Media Device
         * Path, then we have found a match.
         */

    	BOOLEAN PreviousNodeIsHardDriveDevicePath = FALSE;
        EFI_DEVICE_PATH *DevPath = DevicePath;
        EFI_DEVICE_PATH *Next;
        error = EFI_NOT_FOUND;

        /*
         * Check for end of device path type
         */
        for (;;) {

//			CS_DEBUG((D_INFO, L"device path type/subtype: 0x%x/0x%x\n",
//					DevicePathType(DevPath), DevicePathSubType(DevPath)));

            if ((DevicePathType(DevPath) == MEDIA_DEVICE_PATH) &&
            		(DevicePathSubType(DevPath) == MEDIA_HARDDRIVE_DP)) {

            	HARDDRIVE_DEVICE_PATH *HardDriveDevicePath = (HARDDRIVE_DEVICE_PATH *) (DevPath);

            	if (PreviousNodeIsHardDriveDevicePath == FALSE) {

					Next = NextDevicePathNode(DevPath);
					if (IsDevicePathEndType(Next)) {
						error = EFI_SUCCESS;
						*mbr_type = HardDriveDevicePath->MBRType;
						*signature_type = HardDriveDevicePath->SignatureType;

						CS_DEBUG((D_INFO, L"found storage media, MBR type 0x%x, signature type 0x%x, ",
								*mbr_type, *signature_type));

						switch (*signature_type) {
						case SIGNATURE_TYPE_MBR:
							CopyMem(signature, &(HardDriveDevicePath->Signature[0]), sizeof(UINT32));
							CS_DEBUG((D_INFO, L"signature 0x%x\n", *(UINT32 *)(signature)));
							break;
						case SIGNATURE_TYPE_GUID:
							CopyMem(signature, &(HardDriveDevicePath->Signature[0]), sizeof(EFI_GUID));
							CS_DEBUG((D_INFO, L"signature %g\n", (EFI_GUID *)(signature)));
							break;
						}
					}
            	}
                PreviousNodeIsHardDriveDevicePath = TRUE;
              } else {
                PreviousNodeIsHardDriveDevicePath = FALSE;
              }

            if (!EFI_ERROR(error)) {
            	break;
            }

            if (IsDevicePathEnd(DevPath)) {
              break;
            }

            DevPath = NextDevicePathNode(DevPath);
        }
    }

	return error;
}

/*
 *	\brief	find a disk handle by the given UUID
 *
 *	This function searches in all BLOCK_IO devices and requests the UUID. If the UUID is
 *	available and the UUID corresponds to the given UUID (in requested_disk structure),
 *	then the handle is returned.
 *
 *	\param	requested_disk	structure containing the UUID (and partition type) of the requested device
 *	\param	handle	pointer to the buffer for the resulting device handle
 *
 *	\return		the success state of the function
 */
static EFI_STATUS get_handle_by_uuid(IN struct disk_info *requested_disk, OUT EFI_HANDLE *handle) {
	EFI_STATUS error;
	UINTN number_handles = 0;
	EFI_HANDLE *HandleBuffer = NULL;

    ASSERT(requested_disk != NULL);
    ASSERT(handle != NULL);

	/* get a list of BLOCK_IO device handles */
	/* LibLocateHandleByDiskSignature() should also be possible */
	error = LibLocateHandle(ByProtocol, &BlockIoProtocol, NULL, &number_handles, &HandleBuffer);
	if (!EFI_ERROR(error)) {
		BOOLEAN success = FALSE;
		UINTN i;
		struct disk_info disk;

		for (i = 0; i < number_handles; i++) {
			error = get_disk_handle(HandleBuffer[i], &disk.mbr_type, &disk.signature_type, &disk.signature);
			if (EFI_ERROR(error)) {
				CS_DEBUG((D_INFO, L"get_disk_handle() failed: %r\n", error));
				continue;
			}
			if (disk.signature_type == requested_disk->signature_type) {
				switch (disk.signature_type) {
				case SIGNATURE_TYPE_MBR:
					if (disk.signature.mbr_id == requested_disk->signature.mbr_id) {
						success = TRUE;
						CS_DEBUG((D_INFO, L"matching MBR disk handle found 0x%x\n", disk.signature.mbr_id));
					} else {
						CS_DEBUG((D_INFO, L"no match: MBR 0x%x/0x%x\n",
								disk.signature.mbr_id, requested_disk->signature.mbr_id));
					}
					break;
				case SIGNATURE_TYPE_GUID:
					if (CompareMem(&disk.signature.guid, &requested_disk->signature.guid,
							sizeof(disk.signature.guid)) == 0) {
						success = TRUE;
						CS_DEBUG((D_INFO, L"matching GUID disk handle found %g\n", (EFI_GUID *)&disk.signature.guid));
					} else {
						CS_DEBUG((D_INFO, L"no match: GUID %g/%g\n",
								(EFI_GUID *)&disk.signature.guid, (EFI_GUID *)&requested_disk->signature.guid));
					}
					break;
				}
			} else {
				CS_DEBUG((D_INFO, L"no match: signature type: 0x%x/0x%x/0x%x\n",
						disk.signature_type, requested_disk->signature_type, context.caller_disk.signature_type));
#if 0
				Print(L"TEST MODE with MBR disk !!\n");
				success = TRUE;
#endif
			}
			if (success) {
				break;	/* the matching handle of the block device for booting is now in HandleBuffer[i] */
			}
		}
		if (success) {
			*handle = HandleBuffer[i];
		} else {
			CS_DEBUG((D_WARN, L"no matching boot device found in the device tree\n"));
			*handle = NULL;
			error = EFI_NOT_FOUND;
		}
	}

	return error;
}

/*
 * \brief	transform the command line options into C style ARGC/ARGV data
 *
 * The function transforms the given buffer into an array of pointers to the
 * starting positions of the NULL terminated strings. The number of dedicated
 * strings is returned
 *
 *	\param buf	pointer to the buffer containing the command line options as taken from
 *	 			loadedImage->LoadOptions
 *	\param len	size of buffer in byte (not the number of CHAR16's)
 *	\param argv	pointer to the array of pointers for the resulting strings
 *
 *	\return		number of valid pointers in argv
 */
static UINTN transform_args(IN OUT CHAR16 *buf, IN UINTN len, OUT CHAR16 **argv) {
	UINTN i = 0;
	UINTN j = 0;
	CHAR16 *p = buf;

#define CHAR_SPACE L' '
    ASSERT(buf != NULL);
    ASSERT(argv != NULL);

	if (buf == 0) {
		argv[0] = NULL;
		return 0;
	}
	len = len / sizeof(CHAR16);	/* now len is the number of CHAR16 characters */

	for(;;) {
		while (buf[i] == CHAR_SPACE && buf[i] != CHAR_NULL && i < len) {
			i++;
		}
		if (buf[i] == CHAR_NULL || i == len) {
			goto end;
		}

		p = buf+i;
		i++;

		while (buf[i] != CHAR_SPACE && buf[i] != CHAR_NULL && i < len) {
			i++;
		}

		argv[j++] = p;

		if (buf[i] == CHAR_NULL) {
			goto end;
		}

		buf[i] = CHAR_NULL;
		if (i == len) {
			goto end;
		}

		i++;
		if (j == (CS_MAX_LOAD_OPTIONS - 1)) {
			CS_DEBUG((D_ERROR, L"%s: too many arguments (%d), truncating\n", argv[0], j));
			goto end;
		}
	}

end:

	argv[j] = NULL;
	return j;
}

/*
 * \brief	read the command line arguments and save them in the system context
 *
 * The EFI application expects the following command line arguments:
 * 1. the UUID of the partition where the crypto driver shall be connected
 * 2. the UUID of the partition where the EFI OS loader is stored
 * 3. the full path to the EFI OS loader file that shall be started (usually the Windows loader)
 * 4. (and more) options that shall be passed to the EFI loader (as given as option 3) at start
 * If only one option is given, the driver is connected to that partition, but no further loader
 * will be started. If no option is given, the the first partition will be connected with the driver
 * that is found by the available volume header file(s), no further loader will be started.
 *
 * TODO: check this behavior
 *
 *	\param loaded_image		opened image
 *
 */
static void get_cmdline_args(IN EFI_LOADED_IMAGE *loaded_image) {

    ASSERT(loaded_image != NULL);

	context.argc = transform_args(loaded_image->LoadOptions, loaded_image->LoadOptionsSize,
			context.argv);

	if (context.argc > 1) {
		context.dest_uuid = context.argv[1];
		CS_DEBUG((D_INFO, L"argc: 0x%x\nargument 1: %s\n", context.argc, context.argv[1]));
	}
	if (context.argc > 2) {
		context.os_loader_uuid = context.argv[2];
		CS_DEBUG((D_INFO, L"argument 2: %s\n", context.argv[2]));
	}
	if (context.argc > 3) {
		context.os_loader = context.argv[3];
		CS_DEBUG((D_INFO, L"argument 3: %s\n", context.argv[3]));
	}
	if (context.argc > 4) {
		context.os_loader_option_number = context.argc - 4;
		context.os_loader_options = &context.argv[4];
		CS_DEBUG((D_INFO, L"argument 4: %s\n", context.argv[4]));
	}
}

/*
 *	\brief	load and start a given EFI application
 *
 *	This function loads and starts an EFI application identified by a given filename on a given
 *	device handle. Also, options mey be handed over to the application. The handle to the started
 *	application is returned.
 *
 *	\param	ImageHandle
 *	\param handle_to_load	handle of the block device that contains the EFI application file
 *	\param file_to_load		full file path of the EFI application to start at the given device
 *	\param load_option_size	size of the option line in byte (may be 0)
 *	\param load_options		pointer to the option buffer to be handed over to the application
 *	\param loaded_handle	pointer to buffer for the returned handle of the loaded application
 *
 *	\return		the success state of the function
 */
static EFI_STATUS load_start_image(IN EFI_HANDLE ImageHandle, IN EFI_HANDLE handle_to_load, IN CHAR16 *file_to_load,
		IN UINTN load_option_size, IN void *load_options, OUT EFI_HANDLE *loaded_handle) {
	EFI_STATUS error;
	EFI_DEVICE_PATH *path_to_load;

    ASSERT(ImageHandle != NULL);
    ASSERT(handle_to_load != NULL);
    ASSERT(file_to_load != NULL);
    ASSERT(loaded_handle != NULL);

	path_to_load = FileDevicePath(handle_to_load, file_to_load);

	if (path_to_load) {
		error = uefi_call_wrapper(BS->LoadImage, 6,	FALSE, ImageHandle, path_to_load,
				NULL, 0, loaded_handle);

		if (!EFI_ERROR (error)) {
			EFI_LOADED_IMAGE *ImageInfo;

			/* set the calling options for the driver to be used at StartImage() */
			error = uefi_call_wrapper(BS->HandleProtocol, 3,
					*loaded_handle, &LoadedImageProtocol, (VOID *) &ImageInfo);

			if (!EFI_ERROR (error)) {
				/* pass the required option data to the driver... */
				if ((load_option_size > 0) && (load_options != NULL)) {
					if (ImageInfo->LoadOptions) {
					  FreePool (ImageInfo->LoadOptions);
					}
					ImageInfo->LoadOptions = load_options;
					ImageInfo->LoadOptionsSize  = (UINT32)load_option_size;
				} else {
					ImageInfo->LoadOptions = NULL;
					ImageInfo->LoadOptionsSize  = 0;
				}

				/* now start the image... */
				error = uefi_call_wrapper(BS->StartImage, 3, *loaded_handle, NULL, NULL);
				if (EFI_ERROR (error)) {
					uefi_call_wrapper(BS->UnloadImage, 1, *loaded_handle);
					CS_DEBUG((D_ERROR, L"StartImage(%s) failed: %r\n", file_to_load, error));
				}
			}
		} else if (error == EFI_SECURITY_VIOLATION) {
			uefi_call_wrapper(BS->UnloadImage, 1, *loaded_handle);
			CS_DEBUG((D_ERROR, L"LoadImage(%s) failed: %r\n", file_to_load, error));
		} else {
			CS_DEBUG((D_ERROR, L"LoadImage(%s) failed: %r\n", file_to_load, error));
		}
	} else {
		error = EFI_UNSUPPORTED;
		CS_DEBUG((D_ERROR, L"FileDevicePath(%s) failed\n", file_to_load));
	}

	return error;
}

/*
 * \brief	main initialization function
 *
 * The function initializes the application. The function returns the current directory as string,
 * the corresponding buffer must be freed by the caller using FreePool().
 *
 *	\param	ImageHandle
 *	\param	SystemTable
 *	\param	root_dir	pointer to a buffer for the opened handle of the root directory
 *	\param	current_directory	pointer to the buffer for the current directory (string)
 *
 *	\return		the success state of the function
 */
static EFI_STATUS initialize(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable,
		OUT EFI_FILE **root_dir, OUT CHAR16 **current_directory) {

	EFI_STATUS error;
	EFI_LOADED_IMAGE *loaded_image;

    ASSERT(ImageHandle != NULL);
    ASSERT(SystemTable != NULL);
    ASSERT(root_dir != NULL);
    ASSERT(current_directory != NULL);

    init_system_context();	/* initialize global system context (context) */

    InitializeLib(ImageHandle, SystemTable);

    EFIDebug = D_ERROR | D_WARN | D_LOAD | D_BLKIO | D_INIT | D_INFO; // remove this later...

    error = uefi_call_wrapper(BS->OpenProtocol, 6, ImageHandle, &LoadedImageProtocol, (void **)&loaded_image,
		   ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(error)) {
    	cs_print_msg(L"Error getting a LoadedImageProtocol handle: %r", error);
        return error;
    }

    CS_DEBUG((D_INFO, L"LoadedImageProtocol returned %s\n", DevicePathToStr(loaded_image->FilePath)));
    context.caller_disk.handle = loaded_image->DeviceHandle;

    get_cmdline_args(loaded_image);

    *root_dir = LibOpenRoot(loaded_image->DeviceHandle);
    if (!root_dir) {
    	cs_print_msg(L"Unable to open root directory: %s", DevicePathToStr(loaded_image->FilePath));
        error = EFI_LOAD_ERROR;
    } else {
		error = get_current_directory(loaded_image, current_directory);
		if (EFI_ERROR(error)) {
			cs_print_msg(L"Unable to get current directory: %s ", DevicePathToStr(loaded_image->FilePath));
		}
    }

    /*
     * Read information about the storage media where this application was started.
     * This information may become important when it should be handed over to the OS driver
     * in order to access the volume header file in case of password change.
     */
	error = get_disk_handle(loaded_image->DeviceHandle, &context.caller_disk.mbr_type,
			&context.caller_disk.signature_type, &context.caller_disk.signature);
	if (EFI_ERROR(error)) {
		SetMem(&context.caller_disk, 0, sizeof(context.caller_disk));
		cs_print_msg(L"Error getting disk signature: %r ", error);
	}
#if 0
	else {
		Print(L"caller disk: MBR Type: 0x%x, SignatureType: 0x%x \n",
				context.caller_disk.mbr_type, context.caller_disk.signature_type);
	}
#endif

	uefi_call_wrapper(BS->CloseProtocol, 4, ImageHandle, &LoadedImageProtocol, ImageHandle, NULL);

	return error;
}


/*
 * 	\brief	update the volume header with disk encryption data or change user password
 *
 * 	This function updates the volume header from system context with the given data.
 * 	If the given password is valid, the user password is changed instead. Then the
 * 	volume header is stored as file back to disk.
 *
 *	\param	cryptoInfo	data to be updated in volume header
 *	\param	password	new password
 *
 *	\return		the success state of the function
 */
static EFI_STATUS _update_volume_header(IN CRYPTO_INFO *cryptoInfo, OPTIONAL Password *password) {
	EFI_STATUS error = EFI_SUCCESS;
	CRYPTO_INFO vh_cipher_data;

	ASSERT(cryptoInfo != NULL);

	CopyMem(&vh_cipher_data, cryptoInfo, sizeof(vh_cipher_data));

	/* take cipher data from system context: volume header encryption */
	vh_cipher_data.ea = context.volume_header_protection.algo;
	vh_cipher_data.mode = context.volume_header_protection.mode;
	CopyMem(&vh_cipher_data.ks, &context.volume_header_protection.ks,
			sizeof(vh_cipher_data.ks));
	CopyMem(&vh_cipher_data.ks2, &context.volume_header_protection.ks2,
			sizeof(vh_cipher_data.ks2));

	if (cs_update_volume_header((char *)&context.os_driver_data.volume_header[0] /* encrypted volume header */,
			&vh_cipher_data, password)) {
		error = EFI_NOT_READY;
	}

#if 1
	/* write volume header back to file on disk */
	if (!EFI_ERROR (error)) {
		error = write_file(context.root_dir, context.vh_path,
				context.os_driver_data.volume_header, sizeof(context.os_driver_data.volume_header));
	}
#endif

	return error;
}

/*
 * 	\brief	update the volume header with disk encryption data
 *
 * 	This function updates the volume header from system context with the given data
 * 	and stores the volume header file back to the disk.
 *
 *	\param	cryptoInfo	data to be updated in volume header
 *
 *	\return		the success state of the function
 */
EFI_STATUS update_volume_header(IN CRYPTO_INFO *newCryptoInfo) {

	return _update_volume_header(newCryptoInfo, NULL);
}

/*
 * 	\brief	change password
 *
 * 	This function updates the volume header and modifies the user password.
 * 	and stores the volume header file back to the disk.
 *
 *	\param	password	new password
 *
 *	\return		the success state of the function
 */
EFI_STATUS change_password(IN Password *password) {
	CRYPTO_INFO vh_cipher_data;

	SetMem(&vh_cipher_data, sizeof(vh_cipher_data), 0);

	return _update_volume_header(&vh_cipher_data, password);
}

#ifdef CS_TEST_CREATE_VOLUME_HEADER
/* only for test purposes: allow to build arbitrary volume header */
extern int cs_write_volume_header(IN OUT char *header, IN PCRYPTO_INFO cryptoInfo, IN char *password);
EFI_STATUS create_new_volume_header() {
	CRYPTO_INFO vh_cipher_data;
	EFI_STATUS error;

	/********************************************************************************/
	/* fill the content fields of the volume header here... */
	char password[] = "bla";
	vh_cipher_data.VolumeSize.Value = (uint64)103774208;
	vh_cipher_data.EncryptedAreaStart.Value = (uint64)0;
	vh_cipher_data.EncryptedAreaLength.Value = (uint64)0;
	vh_cipher_data.HeaderFlags = 1;
	vh_cipher_data.hiddenVolume = 0;
	vh_cipher_data.ea = 1; 		/* AES */
	vh_cipher_data.mode = XTS;	/* the only valid mode */
	/********************************************************************************/

	error = cs_write_volume_header((char *)&context.os_driver_data.volume_header[0],
			&vh_cipher_data, password);

	if (!EFI_ERROR (error)) {
		CHAR16 new_vh_path[CS_MAX_DRIVER_PATH_SIZE];
		int i;
		StrCpy(new_vh_path, context.vh_path);

		/* set filename of volume header to "xxx...x" */
		for (i = StrLen(new_vh_path) - 1; (i > 0) && (new_vh_path[i] != L'\\'); i--) {
			new_vh_path[i] = 'x';
		}
		CS_DEBUG((D_INFO, L"filename of new volume header: \"%s\"\n", new_vh_path));

		error = write_file(context.root_dir, new_vh_path,
				context.os_driver_data.volume_header, sizeof(context.os_driver_data.volume_header));
		if (EFI_ERROR (error)) {
			CS_DEBUG((D_INFO, L"write_file() failed (%r)\n", error));
		}
	} else {
		CS_DEBUG((D_INFO, L"cs_write_volume_header() returned 0x%x (%r)\n", error, error));
	}
	return error;
}
#endif

/*
 * \brief	decrypt the loaded volume header with the given user password
 *
 * 	this function tries to decrypt the volume header (stored in system context) using the
 * 	user password (stored in system context). If successful, the values from the decrypted
 * 	volume header are stored in the corresponding buffers. Especially the required data
 * 	for the crypto driver (sector numbers and cipher contexts) is extracted.
 * 	In unsuccessful case, the error return value mostly indicates a wrong user password.
 *
 *	\return		the success state of the function (EFI_SUCCESS or EFI_ACCESS_DENIED)
 */
EFI_STATUS decrypt_volume_header() {

	int retvalue;

	ASSERT(sizeof(context.efi_driver_data.cipher.ks) == sizeof(context.os_driver_data.crypto_info.ks));
	ASSERT(sizeof(context.efi_driver_data.cipher.ks2) == sizeof(context.os_driver_data.crypto_info.ks2));

    retvalue = cs_read_volume_header(TRUE /* not a hidden volume */,
    		(char *)&context.os_driver_data.volume_header[0] /* encrypted volume header */,
    		&context.os_driver_data.boot_arguments.BootPassword,
    		&context.os_driver_data.crypto_info);

    if (retvalue != ERR_SUCCESS) {
		CS_DEBUG((D_INFO, L"error while decryption of volume header: 0x%x\n", retvalue));
    	return EFI_ACCESS_DENIED;	/* probably: wrong password */
    }

    /* set some fields in boot_arguments, taken from TrueCrypt, BootMain.cpp, MountVolume() */
    context.os_driver_data.boot_arguments.BootLoaderVersion = VERSION_NUM;
    context.os_driver_data.boot_arguments.CryptoInfoOffset = 0; /* unsupported in EFI version */
    context.os_driver_data.boot_arguments.CryptoInfoLength = 0; /* unsupported in EFI version */

    /* indicate that context.os_driver_data.volume_header contains a volume header */
    context.os_driver_data.boot_arguments.Flags |= TC_BOOT_ARGS_FLAG_BOOT_VOLUME_HEADER_PRESENT;
    context.os_driver_data.boot_arguments.HiddenSystemPartitionStart = 0; /* not unsupported yet */

    TC_SET_BOOT_ARGUMENTS_SIGNATURE(context.os_driver_data.boot_arguments.Signature);

	/* read data from the parsed volume header to send them to the EFI crypto driver:
	 * first the sector numbers of the encrypted area...  */
	context.efi_driver_data.StartSector =
			context.os_driver_data.crypto_info.EncryptedAreaStart.Value  >> TC_LB_SIZE_BIT_SHIFT_DIVISOR;
	context.efi_driver_data.SectorCount =
			context.os_driver_data.crypto_info.EncryptedAreaLength.Value >> TC_LB_SIZE_BIT_SHIFT_DIVISOR;
	context.efi_driver_data.EndSector = context.efi_driver_data.SectorCount - 1;
	context.efi_driver_data.EndSector += context.efi_driver_data.StartSector;

	/* now extract some information regarding hidden volume... */
	context.efi_driver_data.isHiddenVolume = context.os_driver_data.crypto_info.hiddenVolume;
	context.efi_driver_data.HiddenVolumeStartSector = 0;	/* not supported yet */
	context.efi_driver_data.HiddenVolumeStartUnitNo = 0;	/* not supported yet */

	/* now copy the cipher data to the hand-over buffer for the EFI crypto driver */
	context.efi_driver_data.cipher.algo = context.os_driver_data.crypto_info.ea;
	context.efi_driver_data.cipher.mode = context.os_driver_data.crypto_info.mode;
	CopyMem(&context.efi_driver_data.cipher.ks[0], &context.os_driver_data.crypto_info.ks[0],
			sizeof(context.efi_driver_data.cipher.ks));
	CopyMem(&context.efi_driver_data.cipher.ks2[0], &context.os_driver_data.crypto_info.ks2[0],
			sizeof(context.efi_driver_data.cipher.ks2));

	return EFI_SUCCESS;
}

/*
 *	\brief	load and start the crypto driver
 *
 *	The function loads the crypto driver into memory, allocates an option buffer
 *	of this driver and copies the required data (including the decrypted volume header)
 *	into this buffer. Then the driver is started.
 *
 *	\param	ImageHandle
 *	\param	pCryptoDriverHandle	pointer to buffer for the returned handle of the crypto driver
 *
 *	\return		the success state of the function
 */
static EFI_STATUS start_crypto_driver(IN EFI_HANDLE ImageHandle, OUT EFI_HANDLE *pCryptoDriverHandle) {
	EFI_STATUS error;
	UINTN LoadOptionsSize = sizeof(struct cs_efi_option_data);
	struct cs_efi_option_data *LoadOptions = (struct cs_efi_option_data *)AllocatePool(LoadOptionsSize);

	if (LoadOptions != NULL) {
		context.efi_driver_data.debug = context.user_defined_options.driverdebug;
		CopyMem(LoadOptions, &context.efi_driver_data, sizeof(*LoadOptions));

		CS_DEBUG((D_INFO, L"StartSector 0x%x, SectorCount 0x%x\n",
				LoadOptions->StartSector, LoadOptions->SectorCount));
		CS_DEBUG((D_INFO, L"hiddenVolumePresent %x, Algo/Mode 0x%x/0x%x\n",
				LoadOptions->isHiddenVolume, LoadOptions->cipher.algo, LoadOptions->cipher.mode));

		error = load_start_image(ImageHandle, context.caller_disk.handle, &context.cs_driver_path[0],
				LoadOptionsSize, LoadOptions, pCryptoDriverHandle);
	} else {
		error = EFI_OUT_OF_RESOURCES;
	}

	return error;
}

/*
 *	\brief	connect the given crypto driver with the intended block device
 *
 *	This function connects the crypto driver as given by the handle with the intended
 *	boot device. This information is taken from the (global) system context.
 *
 *	\param	CryptoDriverHandle	handle of the crypto driver to connect
 *
 *	\return		the success state of the function
 */
static EFI_STATUS connect_crypto_driver(IN EFI_HANDLE CryptoDriverHandle) {
	EFI_STATUS error;

    ASSERT(CryptoDriverHandle != NULL);

	error = get_handle_by_uuid(&context.boot_partition, &context.boot_partition.handle);
	if ((!EFI_ERROR(error)) && (context.boot_partition.handle)) {
		/* now bind the driver to the boot device... */
		EFI_HANDLE driver_list[2];

	    //CS_DEBUG((D_INFO, L"now starting ConnectController(), handle=0x%x/0x%x\n", handle, CryptoDriverHandle));

		driver_list[0] = CryptoDriverHandle;
		driver_list[1] = NULL;
		error = uefi_call_wrapper(BS->ConnectController, 4,
				context.boot_partition.handle, driver_list, NULL, FALSE /* recursive */);
	}

	return error;
}

/*
 *	\brief	load and start the crypto driver and connect it with the intended block device
 *
 *	This function loads and initializes the crypto driver. Then the driver is connected to the
 *	intended boot device. This information is taken from the (global) system context.
 *
 *	\param	ImageHandle
 *
 *	\return		the success state of the function
 */
EFI_STATUS start_connect_crypto_driver(IN EFI_HANDLE ImageHandle) {

	EFI_STATUS error;
	EFI_HANDLE CryptoDriverHandle;

	error = start_crypto_driver(ImageHandle, &CryptoDriverHandle);

	/* now the decrypted volume header data is not longer needed... */
	SetMem(&context.efi_driver_data, sizeof(context.efi_driver_data), 0);

	if (!EFI_ERROR(error)) {
		error = connect_crypto_driver(CryptoDriverHandle);
	}

	return error;
}

#if 0
/*
 *	\brief	allocate and fill memory for hand over of data to the OS driver
 *
 *	This function allocates a memory pool at a defined physical location that is accessed later
 *	by the driver of the OS. For that reason the physical location must not be changed. Also,
 *	the function fills the data fields in this memory area with the correct values taken from the
 *	decrypted volume header.
 *
 *	Remark: The hard coded memory address for data hand-over probably will not work, because the
 *	        AllocatePages() call might fail when the address is not accessible by the EFI application.
 *	        Instead, an EFI runtime service (driver) might be needed in order to provide the
 *	        service to hand-over the data to the OS, then clean the data area (stop the service,
 *	        if not needed anymore)
 *
 *
 *	\return		the success state of the function
 */
static EFI_STATUS prepare_handover_memory() {
	EFI_STATUS error = EFI_SUCCESS;
	BootArguments *memory_location;

    /* the structure BootArguments needs 118 Byte, further memory is required by BootArguments.CryptoInfoOffset
     * -> 1 (4K) page is big enough */
	const UINTN number_pages = 1;

	EFI_PHYSICAL_ADDRESS physical_address = CS_BOOT_LOADER_ARGS_OFFSET;

	CS_DEBUG((D_INFO, L"prepare_handover_memory(0x%x)\n", physical_address));

	/*
	 * The following memory types were tested: AllocatePages() always returns "Not found":
	 *   EfiLoaderData EfiBootServicesData EfiConventionalMemory EfiBootServicesCode EfiReservedMemoryType
	 * in the EFI Shell, use memmap to show the mapping
	 * */
	error = uefi_call_wrapper(BS->AllocatePages, 4,
			AllocateAddress, EfiRuntimeServicesData,
			number_pages, &physical_address);

	if (!EFI_ERROR(error)) {
		memory_location = (BootArguments *)(UINTN)physical_address;
		SetMem(memory_location, 0, number_pages * 4 * 1024);
		physical_address += sizeof(BootArguments);
		memory_location->CryptoInfoOffset = (UINT16)physical_address;
		memory_location->CryptoInfoLength = 0; /* need to be adjusted later */

		/* TODO: fill the BootArguments structure with correct values */
	} else {
	    CS_DEBUG((D_ERROR, L"unable to allocate memory pages: %r\n", error));
	}

	return error;
}
#endif

/*
 *	\brief	initialize the runtime service to hand over the crypto information to the OS driver
 *
 *	This function loads and starts a runtime service driver in order to provide some information
 *	required for device encryption/decryption to the CipherShed OS driver.
 *	The data to hand over to the OS driver is handed over to the runtime service driver as
 *	start options.
 *
 *	\return		the success state of the function
 */
static EFI_STATUS init_runtime_service() {
	EFI_STATUS error = EFI_SUCCESS;
	EFI_GUID variable_guid = CS_HANDOVER_VARIABLE_GUID;

	/* set a UEFI variable with attributes to be accessible at runtime
	 * -> if the OS driver is able to read this variable _BEFORE_ the call of ExitBootServices(),
	 *    then the attributes should be EFI_VARIABLE_BOOTSERVICE_ACCESS
	 * -> this needs to be tested!
	 * -> this decision has security implications */
	error = uefi_call_wrapper(RT->SetVariable, 5, CS_HANDOVER_VARIABLE_NAME, &variable_guid,
			EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
			sizeof(context.os_driver_data), &context.os_driver_data);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"unable to set EFI variable %s: %r\n", CS_HANDOVER_VARIABLE_NAME, error));
	}

	/* the sensitive data in the system context are no longer needed... */
	SetMem(&context.os_driver_data, sizeof(context.os_driver_data), 0);

	return error;
}

/*
 *	\brief	load and start the OS loader
 *
 *	This function first initializes the memory area for the OS driver where the required crypto data
 *	is handed over. This includes the preparation of this memory area with the crypto data.
 *	Then the function loads and starts the OS loader application.
 *
 *	\param	ImageHandle
 *
 *	\return		the success state of the function
 */
static EFI_STATUS boot_os(IN EFI_HANDLE ImageHandle) {

	EFI_STATUS error;
	EFI_HANDLE loaded_handle;

#if 0
	/* this is the traditional hand over method to the OS cipher driver via a fixed memory location
	 * (backward compatible to TrueCrypt)
	 * _BUT_ it does not (yet) work: the required physical memory address cannot be accessed...
	 * hence the more strait forward solution using the EFI concept is to use an EFI runtime service
	 * to hand over the data to the OS crypto driver */
	error = prepare_handover_memory();
#endif
	error = init_runtime_service();

	if (context.os_loader == NULL) {
		CS_DEBUG((D_INFO, L"No OS loader given... nothing more to do...\n"));
		return error;
	}

	if (!EFI_ERROR(error)) {
		UINTN os_loader_option_size = 0;
		void *os_loader_options = NULL;

		if (context.os_loader_option_number > 0) {
			CHAR16 *ptr = context.os_loader_options[context.os_loader_option_number - 1];
			os_loader_option_size = (UINTN)(ptr - context.os_loader_options[0]);
			os_loader_option_size +=
					StrLen(context.os_loader_options[context.os_loader_option_number - 1])
					* sizeof(*context.os_loader_options[0]);
			os_loader_options = context.os_loader_options[0];
		}

		context.os_loader_disk.mbr_type = MBR_TYPE_EFI_PARTITION_TABLE_HEADER; /* not sure whether this is correct */
		context.os_loader_disk.signature_type = SIGNATURE_TYPE_GUID;
        error = StringToGuid(context.os_loader_uuid, StrLen(context.os_loader_uuid),
        		&context.os_loader_disk.signature.guid);

        if (!EFI_ERROR(error)) {
			error = get_handle_by_uuid(&context.os_loader_disk, &context.os_loader_disk.handle);
			if (!EFI_ERROR(error)) {

				error = load_start_image(ImageHandle, context.os_loader_disk.handle, context.os_loader,
						os_loader_option_size, os_loader_options, &loaded_handle);

			} else {
				CS_DEBUG((D_ERROR, L"get_handle_by_uuid(%s) failed: %r\n", context.os_loader_uuid, error));
			}
        } else {
			CS_DEBUG((D_ERROR, L"StringToGuid(%s) failed: %r\n", context.os_loader_uuid, error));
        }
	}

	return error;
}

/*
 *	\brief	cleanup of sensitive data before exit
 *
 *	This function cleans sensitive (global) variables before the application terminates
 *
 */
static void cs_cleanup() {
	SetMem(&context.volume_header_protection, sizeof(context.volume_header_protection), 0);
}

/*
 *	\brief	the EFI main function
 *
 *	This is the main function that is executed when the application is started
 *
 *	\param	ImageHandle
 *	\param	SystemTable
 *
 *	\return		the success state of the function
 */
EFI_STATUS efi_main (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable) {
	EFI_STATUS error;
    CHAR16 *current_directory;
    enum cs_enum_user_decision user_decision;

    ASSERT(ImageHandle != NULL);
    ASSERT(SystemTable != NULL);

    error = initialize(ImageHandle, SystemTable, &context.root_dir, &current_directory);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_ERROR, L"Unable to initialize the application: %r\n", error));
		goto exit;
	}

	/* load the loader settings from options file, on error, use default values */
	error = load_settings(context.root_dir, current_directory, &context.user_defined_options);
	if (EFI_ERROR(error)) {
		CS_DEBUG((D_WARN, L"Unable to load options from file: %r\n", error));
	    FreePool(current_directory);
	}

	/* load the volume header from the corresponding file */
	error = load_volume_header(context.root_dir, current_directory);
	if (EFI_ERROR(error)) {
		if (!context.user_defined_options.flags.silent)
			cs_print_msg(L"Unable to load Volume Header from file: %r\n", error);
	    FreePool(current_directory);
	    goto exit;
	}

	/* initialize the system context with the crypto driver name */
	error = get_driver_path(current_directory);
	if (EFI_ERROR(error)) {
		if (!context.user_defined_options.flags.silent)
			cs_print_msg(L"Unable to Initialize Crypto Driver Name: %r\n", error);
	    FreePool(current_directory);
	    goto exit;
	}

	FreePool(current_directory); /* no more file system access needed from here... */

	do {
		/* call user dialog to get the password or any other decision */
		error = user_dialog(ImageHandle, SystemTable, &context.user_defined_options,
				&user_decision, &context.os_driver_data.boot_arguments.BootPassword);
		if (EFI_ERROR(error)) {
			if (!context.user_defined_options.flags.silent)
				cs_print_msg(L"User dialog call failed: %r\n", error);
		    goto exit;
		}

		switch (user_decision) {
		case CS_UI_PASSWORD:
			/* this is the default case: the password was typed in */
			error = decrypt_volume_header();
			break;

		case CS_UI_MAIN_MENU:
			break;

		case CS_UI_EXIT_APP:
		    goto exit;

		case CS_UI_REBOOT:
			cs_cleanup();
			return uefi_call_wrapper(RT->ResetSystem, 4, EfiResetCold, EFI_SUCCESS, 0, NULL);

		case CS_UI_SHUTDOWN:
			cs_cleanup();
			return uefi_call_wrapper(RT->ResetSystem, 4, EfiResetShutdown, EFI_SUCCESS, 0, NULL);

		default:
			if (!context.user_defined_options.flags.silent)
				cs_print_msg(L"User dialog returned invalid code: %x\n", user_decision);
		    error = EFI_INVALID_PARAMETER;
		    goto exit;
		}
	} while ((error == EFI_ACCESS_DENIED)		/* wrong password (or corrupted volume header) */
		|| (user_decision == CS_UI_MAIN_MENU));	/* the main menu is required */

	/* for all other error types... close application */
	if (EFI_ERROR(error)) {
		if (!context.user_defined_options.flags.silent)
			cs_print_msg(L"Unable to parse the volume header: %r\n", error);
	    goto exit;
	}

#if 1
	/* load, initialize and start the crypto driver */
	error = start_connect_crypto_driver(ImageHandle);
	if (EFI_ERROR(error)) {
		if (!context.user_defined_options.flags.silent)
			cs_print_msg(L"Unable to start the crypto driver: %r\n", error);
	    goto exit;
	}
#endif
#if 1
	/* start the boot loader of the OS */
	error = boot_os(ImageHandle);
	if (EFI_ERROR(error)) {
		if (!context.user_defined_options.flags.silent)
			cs_print_msg(L"Unable to boot the OS: %r\n", error);
	    goto exit;
	}
#endif

exit:
	cs_cleanup();
	return error;
}
