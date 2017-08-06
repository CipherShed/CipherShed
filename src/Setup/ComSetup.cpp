/*
 Copyright (c) 2007-2010 TrueCrypt Developers Association. All rights reserved.

 Governed by the TrueCrypt License 3.0 the full text of which is contained in
 the file License.txt included in TrueCrypt binary and source code distribution
 packages.
*/

#define TC_MAIN_COM_VERSION_MAJOR 2
#define TC_MAIN_COM_VERSION_MINOR 4

#define TC_FORMAT_COM_VERSION_MAJOR 2
#define TC_FORMAT_COM_VERSION_MINOR 4

#ifndef CS_UNITTESTING
#include <atlbase.h>
#include <comdef.h>
#include <statreg.h>
#endif
#include <windows.h>
#include "ComSetup.h"
#include "../Common/Dlgcode.h"
#include "Resource.h"
#include "../Mount/MainCom_i.c"
#include "../Format/FormatCom_i.c"


extern "C" BOOL RegisterComServers (char *modulePath)
{
	BOOL ret = TRUE;
	wchar_t mainModule[1024], formatModule[1024];
	CComPtr<ITypeLib> tl, tl2;

	wsprintfW (mainModule, L"%hsCipherShed.exe", modulePath);
	wsprintfW (formatModule, L"%hsCipherShed-Format.exe", modulePath);

	UnRegisterTypeLib (LIBID_CipherShedMainCom, TC_MAIN_COM_VERSION_MAJOR, TC_MAIN_COM_VERSION_MINOR, 0, SYS_WIN32);
	UnRegisterTypeLib (LIBID_CipherShedFormatCom, TC_FORMAT_COM_VERSION_MAJOR, TC_FORMAT_COM_VERSION_MINOR, 0, SYS_WIN32);

	wchar_t setupModule[MAX_PATH];
	GetModuleFileNameW (NULL, setupModule, sizeof (setupModule) / sizeof (setupModule[0]));

	CRegObject ro;
	HRESULT r;

	if (!SUCCEEDED (r = ro.FinalConstruct ())	/* Init. */
		|| !SUCCEEDED (r = ro.AddReplacement (L"MAIN_MODULE", mainModule))	/* Replaceable parameters. */
		|| !SUCCEEDED (r = ro.AddReplacement (L"FORMAT_MODULE", formatModule))
		|| !SUCCEEDED (r = ro.ResourceRegister (setupModule, IDR_COMREG, L"REGISTRY"))	/* Register ComSetup.rgs */
		|| !SUCCEEDED (r = LoadTypeLib (mainModule, &tl))	/* CipherShed type class. */
		|| !SUCCEEDED (r = RegisterTypeLib (tl, mainModule, 0))
		|| !SUCCEEDED (r = LoadTypeLib (formatModule, &tl2))	/* CipherShedFormat type class. */
		|| !SUCCEEDED (r = RegisterTypeLib (tl2, formatModule, 0)))
	{
		MessageBox (MainDlg, _com_error (r).ErrorMessage(), TC_APP_NAME, MB_ICONERROR);
		ret = FALSE;
	}

	ro.FinalRelease ();
	return ret;
}


extern "C" BOOL UnregisterComServers (char *modulePath)
{
	BOOL ret;

	if (UnRegisterTypeLib (LIBID_CipherShedMainCom, TC_MAIN_COM_VERSION_MAJOR, TC_MAIN_COM_VERSION_MINOR, 0, SYS_WIN32) != S_OK)
		return FALSE;
	if (UnRegisterTypeLib (LIBID_CipherShedFormatCom, TC_FORMAT_COM_VERSION_MAJOR, TC_FORMAT_COM_VERSION_MINOR, 0, SYS_WIN32) != S_OK)
		return FALSE;

	wchar_t module[1024];
	CRegObject ro;

	/* Init. */
	ro.FinalConstruct ();

	/* Replaceable parameters. */
	wsprintfW (module, L"%hsCipherShed.exe", modulePath);
	ro.AddReplacement (L"MAIN_MODULE", module);

	wsprintfW (module, L"%hsCipherShed-Format.exe", modulePath);
	ro.AddReplacement (L"FORMAT_MODULE", module);

	wchar_t setupModule[MAX_PATH];
	GetModuleFileNameW (NULL, setupModule, sizeof (setupModule) / sizeof (setupModule[0]));

	/* Unregister ComSetup.rgs */
	ret = ro.ResourceUnregister (setupModule, IDR_COMREG, L"REGISTRY") == S_OK;

	ro.FinalRelease ();
	return ret;
}
