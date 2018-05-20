#include "userperms.h"

#include "../Tcdefs.h"
#include "../../Platform/Finally.h"

#ifndef CS_UNITTESTING
#include <Shlobj.h>
#else
#include "../../unit-tests/faux/windows/WCHAR.h"
#include "../../unit-tests/faux/windows/HKEY.h"
#include "../../unit-tests/faux/windows/ERROR.h"
#include "../../unit-tests/faux/windows/RegOpenKeyEx.h"
#include "../../unit-tests/faux/windows/BYTE.h"
#include "../../unit-tests/faux/windows/stdcall.h"
#include "../../unit-tests/faux/windows/GetProcAddress.h"
#include "../../unit-tests/faux/windows/GetModuleHandle.h"
#include "../../unit-tests/faux/windows/GetCurrentProcess.h"
#include "../../unit-tests/faux/windows/OSVERSIONINFOEX.h"
#include "../../unit-tests/faux/windows/OSVERSIONINFO.h"
#include "../../unit-tests/faux/windows/GetVersionEx.h"
#include "../../unit-tests/faux/windows/DefWindowProc.h"
#include "../../unit-tests/faux/windows/WNDCLASSEX.h"
#include "../../unit-tests/faux/windows/RegisterClassEx.h"
#include "../../unit-tests/faux/windows/CreateWindowEx.h"
#include "../../unit-tests/faux/windows/GetSystemMetrics.h"
#include "../../unit-tests/faux/windows/SetLayeredWindowAttributes.h"
#include "../../unit-tests/faux/windows/ShowWindow.h"
#include "../../unit-tests/faux/windows/DestroyWindow.h"
#include "../../unit-tests/faux/windows/UnregisterClass.h"
#include "../../unit-tests/faux/windows/IsUserAnAdmin.h"
#include "../../unit-tests/faux/windows/OpenProcessToken.h"
#include "../../unit-tests/faux/windows/TOKEN_INFORMATION_CLASS.h"
#include "../../unit-tests/faux/windows/GetTokenInformation.h"
#include "../../unit-tests/faux/windows/TOKEN_USER.h"
#include "../../unit-tests/faux/windows/IsWellKnownSid.h"
#include <stdlib.h>
#endif

#include "../util/dll.h"

BOOL UacElevated = FALSE;

BOOL IsUacSupported ()
{
	HKEY hkey;
	DWORD value = 1, size = sizeof (DWORD);

	if (!IsOSAtLeast (WIN_VISTA))
		return FALSE;

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		if (RegQueryValueExA(hkey, "EnableLUA", 0, 0, (LPBYTE) &value, &size) != ERROR_SUCCESS)
			value = 1;

		RegCloseKey (hkey);
	}

	return value != 0;
}

BOOL IsOSAtLeast (OSVersionEnum reqMinOS)
{
	return IsOSVersionAtLeast (reqMinOS, 0);
}

// Returns TRUE if the operating system is at least reqMinOS and service pack at least reqMinServicePack.
// Example 1: IsOSVersionAtLeast (WIN_VISTA, 1) called under Windows 2008, returns TRUE.
// Example 2: IsOSVersionAtLeast (WIN_XP, 3) called under Windows XP SP1, returns FALSE.
// Example 3: IsOSVersionAtLeast (WIN_XP, 3) called under Windows Vista SP1, returns TRUE.
BOOL IsOSVersionAtLeast (OSVersionEnum reqMinOS, int reqMinServicePack)
{
	/* When updating this function, update IsOSAtLeast() in Ntdriver.c too. */

	int major = 0, minor = 0;

	if (CurrentOSMajor <= 0)
		TC_THROW_FATAL_EXCEPTION;

	switch (reqMinOS)
	{
	case WIN_2000:			major = 5; minor = 0; break;
	case WIN_XP:			major = 5; minor = 1; break;
	case WIN_SERVER_2003:	major = 5; minor = 2; break;
	case WIN_VISTA:			major = 6; minor = 0; break;
	case WIN_7:				major = 6; minor = 1; break;
	case WIN_8:				major = 6; minor = 2; break;
	case WIN_10:			major = 6; minor = 4; break;

	default:
		TC_THROW_FATAL_EXCEPTION;
		break;
	}

	return ((CurrentOSMajor << 16 | CurrentOSMinor << 8 | CurrentOSServicePack)
		>= (major << 16 | minor << 8 | reqMinServicePack));
}


typedef BOOL (__stdcall *LPFN_ISWOW64PROCESS ) (HANDLE hProcess,PBOOL Wow64Process);


BOOL Is64BitOs ()
{
    static BOOL isWow64 = FALSE;
	static BOOL valid = FALSE;
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	if (valid)
		return isWow64;
	//this can not ever be not loaded so it is safe...
	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress (GetModuleHandleA("kernel32"), "IsWow64Process");

    if (fnIsWow64Process != NULL)
        if (!fnIsWow64Process (GetCurrentProcess(), &isWow64))
			isWow64 = FALSE;

	valid = TRUE;
    return isWow64;
}


BOOL IsServerOS ()
{
	OSVERSIONINFOEXA osVer;
	osVer.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEXA);
	GetVersionExA ((LPOSVERSIONINFOA) &osVer);

	return (osVer.wProductType == VER_NT_SERVER || osVer.wProductType == VER_NT_DOMAIN_CONTROLLER);
}

static LRESULT CALLBACK EnableElevatedCursorChangeWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc (hWnd, message, wParam, lParam);
}

void EnableElevatedCursorChange (HWND parent)
{
	// Create a transparent window to work around a UAC issue preventing change of the cursor
	if (UacElevated)
	{
		const char *className = "CipherShedEnableElevatedCursorChange";
		WNDCLASSEXA winClass;
		HWND hWnd;

		memset (&winClass, 0, sizeof (winClass));
		winClass.cbSize = sizeof (WNDCLASSEX); 
		winClass.lpfnWndProc = (WNDPROC) EnableElevatedCursorChangeWndProc;
		winClass.hInstance = hInst;
		winClass.lpszClassName = className;
		RegisterClassExA(&winClass);

		hWnd = CreateWindowExA(WS_EX_TOOLWINDOW | WS_EX_LAYERED, className, "CipherShed UAC", 0, 0, 0, GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN), parent, NULL, hInst, NULL);
		SetLayeredWindowAttributes (hWnd, 0, 1, LWA_ALPHA);
		ShowWindow (hWnd, SW_SHOWNORMAL);

		DestroyWindow (hWnd);
		UnregisterClassA(className, hInst);
	}
}

BOOL IsAdmin (void)
{
	return IsUserAnAdmin ();
}


BOOL IsBuiltInAdmin ()
{
	HANDLE procToken;
	DWORD size;

	if (!IsAdmin() || !OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &procToken))
		return FALSE;

	finally_do_arg (HANDLE, procToken, { CloseHandle (finally_arg); });

	if (GetTokenInformation (procToken, TokenUser, NULL, 0, &size) || GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return FALSE;

	TOKEN_USER *tokenUser = (TOKEN_USER *) malloc (size);
	if (!tokenUser)
		return FALSE;

	finally_do_arg (void *, tokenUser, { free (finally_arg); });

	if (!GetTokenInformation (procToken, TokenUser, tokenUser, size, &size))
		return FALSE;

	return IsWellKnownSid (tokenUser->User.Sid, WinAccountAdministratorSid);
}


