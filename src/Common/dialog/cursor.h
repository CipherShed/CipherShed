#ifndef CS_Common_dialog_cursor_h_
#define CS_Common_dialog_cursor_h_

#ifndef CS_UNITTESTING
#include <windows.h>
#else
#include "../../unit-tests/faux/windows/HCURSOR.h"
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

extern HCURSOR hCursor;
void WaitCursor ( void );
void NormalCursor ( void );
void ArrowWaitCursor ( void );
void HandCursor ();

#ifdef __cplusplus
}
#endif


#endif