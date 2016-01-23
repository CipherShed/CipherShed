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

EFI_FILE_HANDLE csLogfile = NULL;	/* file handle for (opened) logfile */

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
		static CHAR16 buffer[500];
		UINTN outputSize;
		EFI_STATUS error;

		ASSERT(format != NULL);

		outputSize = VSPrint (buffer, sizeof(buffer), format, args);
		outputSize *= sizeof(buffer[0]);

		error = uefi_call_wrapper(csLogfile->Write, 3, csLogfile, &outputSize, buffer);
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
			return;
	}

	va_start (args, format);
	if (((EFIDebug & D_FILE) == 0) && ((EFIDebug & D_FILE_APPEND) == 0)) {
		VPrint(format, args);
	} else {
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
			uefi_call_wrapper(csLogfile->SetPosition, 2, csLogfile,
					(EFIDebug & D_FILE_APPEND) ? 0xFFFFFFFFFFFFFFFF : 0);
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
		uefi_call_wrapper(csLogfile->Flush, 1, csLogfile);
		uefi_call_wrapper(csLogfile->Close, 1, csLogfile);
		csLogfile = NULL;
	}
}

#endif /* EFI_DEBUG */
