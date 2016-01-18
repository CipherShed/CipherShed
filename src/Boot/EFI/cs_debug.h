/*  cs_debug.h - CipherShed EFI boot loader
 *
 *
 *
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
