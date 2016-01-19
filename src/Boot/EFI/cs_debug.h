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

/* need to define own DEBUG function since the DEBUG() macro seems not to work */
#if EFI_DEBUG

void cs_debug(INTN mask, CHAR16 *format, ...);
#define CS_DEBUG(a)	cs_debug a
#define CS_DEBUG_SLEEP(a)	if (EFIDebug) { cs_sleep(a); }

#else
#define CS_DEBUG(a)
#define CS_DEBUG_SLEEP(a)
#endif /* EFI_DEBUG */


#endif /* _CS_DEBUG_H_ */
