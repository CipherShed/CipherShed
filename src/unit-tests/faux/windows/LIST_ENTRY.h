#ifndef _faux_windows_LIST_ENTRY_h_
#define _faux_windows_LIST_ENTRY_h_

#include "RESTRICTED_POINTER.h"

typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY *Flink;
   struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY, *RESTRICTED_POINTER PRLIST_ENTRY;

#endif