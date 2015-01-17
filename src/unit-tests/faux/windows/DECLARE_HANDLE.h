#ifndef _faux_windows_declare_handle_h_
#define _faux_windows_declare_handle_h_

#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name

#endif