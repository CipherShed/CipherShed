#include "process.h"

#include "Tcdefs.h"

#ifndef CS_UNITTESTING
#include <windowsx.h>
#include <dbghelp.h>
#include <dbt.h>
#include <fcntl.h>
#include <io.h>
#include <math.h>
#include <shlobj.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>
#endif

#include "Resource.h"

#include "../Platform/Finally.h"
#include "../Platform/ForEach.h"
#include "Apidrvr.h"
#include "BootEncryption.h"
#include "Combo.h"
#include "Crc.h"
#include "Crypto.h"
#include "Dictionary.h"
#include "Dlgcode.h"
#include "EncryptionThreadPool.h"
#include "Endian.h"
#include "../Format/InPlace.h"
#include "Language.h"
#include "Keyfiles.h"
#include "Pkcs5.h"
#include "Random.h"
#include "Registry.h"
#include "SecurityToken.h"
using namespace std;
#include "Tests.h"
#include "Volumes.h"
#include "Wipe.h"
#include "Xml.h"
#include "Xts.h"
#include "../Boot/Windows/BootCommon.h"
#include "snprintf.h"

#ifdef TCMOUNT
#include "Mount/Mount.h"
#endif

#ifdef VOLFORMAT
#include "Format/Tcformat.h"
#endif

#ifdef SETUP
#include "Setup/Setup.h"
#endif


void AbortProcess (char *stringId)
{
	// Note that this function also causes localcleanup() to be called (see atexit())
	MessageBeep (MB_ICONEXCLAMATION);
	MessageBoxW (NULL, GetString (stringId), lpszTitle, ICON_HAND);
	exit (1);
}

void AbortProcessSilent (void)
{
	// Note that this function also causes localcleanup() to be called (see atexit())
	exit (1);
}
