/*  cs_debug.h - CipherShed EFI boot loader
 *
 *	Copyright (c) 2015-2016  Falk Nedwal
 *
 *	Governed by the Apache 2.0 License the full text of which is contained in
 *	the file License.txt included in CipherShed binary and source code distribution
 *	packages.
 */

#ifndef _CS_DEBUG_H_
#define _CS_DEBUG_H_

/* extend the definitions D_* from efidebug.h with values for debug output to a file: */
#define D_FILE			0x00100000	/* debug output to new file */
#define D_FILE_APPEND	0x00200000	/* append debug output to file */

/* need to define own DEBUG function since the DEBUG() macro seems not to work */
#if EFI_DEBUG

void cs_debug(INTN mask, CHAR16 *format, ...);
void cs_debug_init(IN EFI_FILE_HANDLE root_dir_handle, IN CHAR16 *current_dir, IN CHAR16 *filename);
void cs_debug_exit();
#define CS_DEBUG_INIT(a)	cs_debug_init a
#define CS_DEBUG_EXIT(a)	cs_debug_exit a
#define CS_DEBUG(a)			cs_debug a
#define CS_DEBUG_SLEEP(a)	if (EFIDebug) { cs_sleep(a); }

#else
#define CS_DEBUG_INIT(a)
#define CS_DEBUG_EXIT(a)
#define CS_DEBUG(a)
#define CS_DEBUG_SLEEP(a)
#endif /* EFI_DEBUG */

#endif /* _CS_DEBUG_H_ */
