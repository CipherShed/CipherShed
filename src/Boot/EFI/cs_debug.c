/*  cs_debug.c - CipherShed EFI boot loader
 *  implementation of the debug features of the application and the driver
 *
 *	Copyright (c) 2015-2016  Falk Nedwal
 *
 *	Governed by the Apache 2.0 License the full text of which is contained in
 *	the file License.txt included in CipherShed binary and source code distribution
 *	packages.
 */

#include <efi.h>
#include <efilib.h>

#include "cs_debug.h"

#if EFI_DEBUG
#include <cs_common.h>

#define CS_MAX_DEBUG_LINESIZE	500	/* max number characters per line */

EFI_FILE_HANDLE csLogfile = NULL;	/* file handle for (opened) logfile */

/*
 * \brief convert the given unicode string to an ascii string
 *
 * The unicode string is converted into an ascii string in the same buffer (!),
 * hence the content of the input buffer will be destroyed.
 * The function simply takes every second byte of the source buffer and puts
 * it into the result buffer.
 *
 *	\param	uString	buffer containing the unicode string
 *
 *	\return	pointer to the result buffer containing the ascii string,
 *			this is in fact the same address as the input buffer
 */
static CHAR8 *uString_to_aString(IN CHAR16 *uString) {
	if (uString) {
		CHAR8 *aString;
		UINTN len = StrLen(uString);
		UINTN i;

		aString = (CHAR8 *)uString;
		for (i = 0; i < len; ++i) {
			aString[i] = aString[2 * i];
		}
		for (i = len; i < len * sizeof(uString[0]); ++i) {
			aString[i] = 0;
		}
		return aString;
	}
	return NULL;
}

/*
 * \brief write the given data to the logfile
 *
 * The arguments are written to the logfile that is assumed to be opened (csLogfile).
 * The size of the text is limited by the size of the buffer as defined in the function.
 *
 *	\param	format	format string as required by VSPrint()
 *	\param	args	arguments as required by VSPrint()
 */
static void cs_debug_write_file(IN CHAR16 *format, IN va_list args) {
	if (csLogfile) {
		static CHAR16 buffer[CS_MAX_DEBUG_LINESIZE];
		CHAR8 *aString;
		UINTN outputSize;
		EFI_STATUS error;

		ASSERT(format != NULL);

		VSPrint (buffer, sizeof(buffer), format, args);
		aString = uString_to_aString(buffer);
		outputSize = strlena (aString);

		error = uefi_call_wrapper(csLogfile->Write, 3, csLogfile, &outputSize, aString);
		if (EFI_ERROR(error)) {
			CS_DEBUG((D_ERROR, L"Unable to write logfile \"%s\", size 0x&x (%r)\n", buffer, outputSize, error));
		}

		uefi_call_wrapper(csLogfile->Flush, 1, csLogfile);
	}
}

/*
 * \brief own DEBUG function since the provided DEBUG() macro seems not to work
 *
 * This is a replacement for the macro DEBUG: it simply calls Print()
 *
 */
void cs_debug(INTN mask, CHAR16 *format, ...) {

	va_list args;

	if (!(EFIDebug & mask)) {
		/* debugging is disabled per configuration */
		return;
	}

	va_start (args, format);
	if (((EFIDebug & D_FILE) == 0) && ((EFIDebug & D_FILE_APPEND) == 0)) {
		VPrint(format, args);
	} else {
		if ((EFIDebug & D_FILE) && (EFIDebug & D_FILE_APPEND)) {
			/* if both flags are set then output to STDOUT and to the logfile */
			VPrint(format, args);
		}
		cs_debug_write_file(format, args);
	}
	va_end (args);
}

/*
 * \brief initialize the debugging function, especially the logfile
 *
 * The function opens the log file for debugging, the logfile name and the current directory
 * are given as argument
 *
 *	\param	root_dir_handle	handle of the root directory
 *	\param	current_dir		string containing the current directory where the logfile will created
 *	\param	filename		name of the logfile
 */
void cs_debug_init(IN EFI_FILE_HANDLE root_dir_handle, IN CHAR16 *current_dir, IN CHAR16 *filename) {
	EFI_STATUS error;
    CHAR16 *buf;
    UINTN buflen;

	if (((EFIDebug & D_FILE) == 0) && ((EFIDebug & D_FILE_APPEND) == 0)) {
		csLogfile = NULL;
		return;	/* nothing to do */
	}

    ASSERT(root_dir_handle != NULL);
    ASSERT(current_dir != NULL);
    ASSERT(filename != NULL);

    buflen = StrLen(current_dir) + StrLen(filename) + 1;
    buf = AllocatePool(buflen);
    if (buf) {
    	StrCpy(buf, current_dir);
    	StrCat(buf, filename);
       	CS_DEBUG((D_INFO, L"Logfile name is: %s\n", buf));

		error = uefi_call_wrapper(root_dir_handle->Open, 5, root_dir_handle, &csLogfile, buf,
				EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
		if (EFI_ERROR(error)) {
	       	cs_print_msg(L"Unable to open logfile \"%s\" (%r)\n", buf, error);
			csLogfile = NULL;
		} else {
			if ((EFIDebug & D_FILE_APPEND) == 0) {
				/* need to write to new file -> delete file, then open again... */
				uefi_call_wrapper(root_dir_handle->Delete, 1, csLogfile);

				error = uefi_call_wrapper(root_dir_handle->Open, 5, root_dir_handle, &csLogfile, buf,
						EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);
				if (EFI_ERROR(error)) {
			       	cs_print_msg(L"Unable to open logfile \"%s\" (%r)\n", buf, error);
					csLogfile = NULL;
				} else {
					uefi_call_wrapper(csLogfile->SetPosition, 2, csLogfile, 0);
				}
			} else {
				uefi_call_wrapper(csLogfile->SetPosition, 2, csLogfile, 0xFFFFFFFFFFFFFFFF);
			}
		}

		FreePool(buf);
    }
}

/*
 * \brief close the debugging function, especially the logfile
 *
 * The function closes the logfile as defined in the (global) handle csLogfile
 *
 */
void cs_debug_exit() {
	if (csLogfile) {
		cs_debug(D_INFO, L"<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");	/* denote the end of the log */
		uefi_call_wrapper(csLogfile->Flush, 1, csLogfile);
		uefi_call_wrapper(csLogfile->Close, 1, csLogfile);
		csLogfile = NULL;
	}
}

#endif /* EFI_DEBUG */
