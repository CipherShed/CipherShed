/*
 *
 *
 *
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
