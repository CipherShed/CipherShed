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
	VPrint(format, args);
	va_end (args);
}
#endif /* EFI_DEBUG */
