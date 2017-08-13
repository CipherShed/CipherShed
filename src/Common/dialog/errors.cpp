#include "errors.h"

#include "../Dlgcode.h"
#include "../Language.h"
#ifdef TCMOUNT 
#include "../../Mount/Mount.h"
#endif
#include "../snprintf.h"
#include "../Resource.h"

#ifndef CS_UNITTESTING
#else
#include "../../unit-tests/faux/windows/ERROR.h"
#include "../../unit-tests/faux/windows/FormatMessage.h"
#include "../../unit-tests/faux/windows/lang.h"
#include "../../unit-tests/faux/windows/WORD.h"
#include "../../unit-tests/faux/windows/MessageBox.h"
#include "../../unit-tests/faux/windows/LocalFree.h"
#include "../../unit-tests/faux/windows/NOTIFYICON.h"
#include "../../unit-tests/faux/windows/ZeroMemory.h"
#include "../../unit-tests/faux/windows/NIF.h"
#include "../../unit-tests/faux/windows/NIIF.h"
#include "../../unit-tests/faux/windows/ARRAYSIZE.h"
#include "../../unit-tests/faux/windows/NIM.h"
#include "../../unit-tests/faux/windows/ShellNotifyIcon.h"
#endif


BOOL Silent = FALSE;

DWORD handleWin32Error (HWND hwndDlg)
{
	PWSTR lpMsgBuf;
	DWORD dwError = GetLastError ();

	if (Silent || dwError == 0 || dwError == ERROR_INVALID_WINDOW_HANDLE)
		return dwError;

	// Access denied
	if (dwError == ERROR_ACCESS_DENIED && !IsAdmin ())
	{
		Error ("ERR_ACCESS_DENIED");
		SetLastError (dwError);		// Preserve the original error code
		return dwError;
	}

	FormatMessageW (
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			      NULL,
			      dwError,
			      MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),	/* Default language */
			      (PWSTR) &lpMsgBuf,
			      0,
			      NULL
	    );

	MessageBoxW (hwndDlg, lpMsgBuf, (LPCWSTR)lpszTitle, ICON_HAND);
	LocalFree (lpMsgBuf);

	// User-friendly hardware error explanation
	if (IsDiskError (dwError))
		Error ("ERR_HARDWARE_ERROR");

	// Device not ready
	if (dwError == ERROR_NOT_READY)
		HandleDriveNotReadyError();

	SetLastError (dwError);		// Preserve the original error code

	return dwError;
}

BOOL translateWin32Error (wchar_t *lpszMsgBuf, int nWSizeOfBuf)
{
	DWORD dwError = GetLastError ();

	if (FormatMessageW (FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError,
			   MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),	/* Default language */
			   (LPWSTR)lpszMsgBuf, nWSizeOfBuf, NULL))
	{
		SetLastError (dwError);		// Preserve the original error code
		return TRUE;
	}

	SetLastError (dwError);			// Preserve the original error code
	return FALSE;
}

std::wstring GetWrongPasswordErrorMessage (HWND hwndDlg)
{
	WCHAR szTmp[8192];

	swprintf ((wchar_t*)szTmp, ARRAYSIZE(szTmp), L"%s", GetString (KeyFilesEnable ? "PASSWORD_OR_KEYFILE_WRONG" : "PASSWORD_WRONG"));
	if (CheckCapsLock (hwndDlg, TRUE))
		wcscat ((wchar_t*)szTmp, GetString ("PASSWORD_WRONG_CAPSLOCK_ON"));

#ifdef TCMOUNT 
	if (TCBootLoaderOnInactiveSysEncDrive ())
	{
		swprintf (szTmp, GetString (KeyFilesEnable ? "PASSWORD_OR_KEYFILE_OR_MODE_WRONG" : "PASSWORD_OR_MODE_WRONG"));

		if (CheckCapsLock (hwndDlg, TRUE))
			wcscat (szTmp, GetString ("PASSWORD_WRONG_CAPSLOCK_ON"));

		wcscat (szTmp, GetString ("SYSENC_MOUNT_WITHOUT_PBA_NOTE"));
	}
#endif

	std::wstring msg = (wchar_t*)szTmp;

#ifdef TCMOUNT
	if (KeyFilesEnable && HiddenFilesPresentInKeyfilePath)
	{
		msg += GetString ("HIDDEN_FILES_PRESENT_IN_KEYFILE_PATH");
		HiddenFilesPresentInKeyfilePath = FALSE;
	}
#endif

	return msg;
}

void handleError (HWND hwndDlg, int code)
{
	WCHAR szTmp[4096];

	if (Silent) return;

	switch (code)
	{
	case ERR_OS_ERROR:
		handleWin32Error (hwndDlg);
		break;
	case ERR_OUTOFMEMORY:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("OUTOFMEMORY"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;

	case ERR_PASSWORD_WRONG:
		MessageBoxW (hwndDlg, (LPCWSTR)GetWrongPasswordErrorMessage (hwndDlg).c_str(), (LPCWSTR)lpszTitle, MB_ICONWARNING);
		break;

	case ERR_DRIVE_NOT_FOUND:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("NOT_FOUND"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_FILES_OPEN:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("OPENFILES_DRIVER"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_FILES_OPEN_LOCK:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("OPENFILES_LOCK"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_VOL_SIZE_WRONG:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("VOL_SIZE_WRONG"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_COMPRESSION_NOT_SUPPORTED:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("COMPRESSION_NOT_SUPPORTED"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_PASSWORD_CHANGE_VOL_TYPE:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("WRONG_VOL_TYPE"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_VOL_SEEKING:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("VOL_SEEKING"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_CIPHER_INIT_FAILURE:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("ERR_CIPHER_INIT_FAILURE"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_CIPHER_INIT_WEAK_KEY:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("ERR_CIPHER_INIT_WEAK_KEY"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_VOL_ALREADY_MOUNTED:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("VOL_ALREADY_MOUNTED"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_FILE_OPEN_FAILED:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("FILE_OPEN_FAILED"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_VOL_MOUNT_FAILED:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString  ("VOL_MOUNT_FAILED"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_NO_FREE_DRIVES:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("NO_FREE_DRIVES"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;
	case ERR_ACCESS_DENIED:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("ACCESS_DENIED"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;

	case ERR_DRIVER_VERSION:
		Error ("DRIVER_VERSION");
		break;

	case ERR_NEW_VERSION_REQUIRED:
		MessageBoxW (hwndDlg, (LPCWSTR)GetString ("NEW_VERSION_REQUIRED"), (LPCWSTR)lpszTitle, ICON_HAND);
		break;

	case ERR_SELF_TESTS_FAILED:
		Error ("ERR_SELF_TESTS_FAILED");
		break;

	case ERR_VOL_FORMAT_BAD:
		Error ("ERR_VOL_FORMAT_BAD");
		break;

	case ERR_ENCRYPTION_NOT_COMPLETED:
		Error ("ERR_ENCRYPTION_NOT_COMPLETED");
		break;

	case ERR_NONSYS_INPLACE_ENC_INCOMPLETE:
		Error ("ERR_NONSYS_INPLACE_ENC_INCOMPLETE");
		break;

	case ERR_SYS_HIDVOL_HEAD_REENC_MODE_WRONG:
		Error ("ERR_SYS_HIDVOL_HEAD_REENC_MODE_WRONG");
		break;

	case ERR_PARAMETER_INCORRECT:
		Error ("ERR_PARAMETER_INCORRECT");
		break;

	case ERR_USER_ABORT:
	case ERR_DONT_REPORT:
		// A non-error
		break;

	default:
		swprintf ((wchar_t*)szTmp, ARRAYSIZE(szTmp), GetString ("ERR_UNKNOWN"), code);
		MessageBoxW (hwndDlg, (LPCWSTR)szTmp, (LPCWSTR)lpszTitle, ICON_HAND);
	}
}

void TaskBarIconDisplayBalloonTooltip (HWND hwnd, const wchar_t *headline, const wchar_t *text, BOOL warning)
{
	if (nCurrentOS == WIN_2000)
	{
		MessageBoxW (MainDlg, (LPCWSTR)text, (LPCWSTR)headline, warning ? MB_ICONWARNING : MB_ICONINFORMATION);
		return;
	}

	NOTIFYICONDATAW tnid; 

	ZeroMemory (&tnid, sizeof (tnid));

	tnid.cbSize = sizeof (tnid); 
	tnid.hWnd = hwnd; 
	tnid.uID = IDI_CIPHERSHED_ICON; 
	//tnid.uVersion = (IsOSAtLeast (WIN_VISTA) ? NOTIFYICON_VERSION_4 : NOTIFYICON_VERSION);

	//Shell_NotifyIconW (NIM_SETVERSION, &tnid);

	tnid.uFlags = NIF_INFO; 
	tnid.dwInfoFlags = (warning ? NIIF_WARNING : NIIF_INFO);
	tnid.uTimeout = (IsOSAtLeast (WIN_VISTA) ? 1000 : 5000); // in ms

	wcsncpy ((wchar_t*)tnid.szInfoTitle, headline, ARRAYSIZE (tnid.szInfoTitle) - 1);
	wcsncpy ((wchar_t*)tnid.szInfo, text, ARRAYSIZE (tnid.szInfo) - 1);

	// Display the balloon tooltip quickly twice in a row to avoid the slow and unwanted "fade-in" phase
	Shell_NotifyIconW (NIM_MODIFY, &tnid);
	Shell_NotifyIconW (NIM_MODIFY, &tnid);
}


// Either of the pointers may be NULL
void InfoBalloon (const char *headingStringId, const char *textStringId)
{
	if (Silent) 
		return;

	TaskBarIconDisplayBalloonTooltip (MainDlg,
		headingStringId == NULL ? L"CipherShed" : GetString (headingStringId), 
		textStringId == NULL ? L" " : GetString (textStringId), 
		FALSE);
}


// Either of the pointers may be NULL
void InfoBalloonDirect (const wchar_t *headingString, const wchar_t *textString)
{
	if (Silent) 
		return;

	TaskBarIconDisplayBalloonTooltip (MainDlg,
		headingString == NULL ? L"CipherShed" : headingString, 
		textString == NULL ? L" " : textString, 
		FALSE);
}


// Either of the pointers may be NULL
void WarningBalloon (const char *headingStringId, const char *textStringId)
{
	if (Silent) 
		return;

	TaskBarIconDisplayBalloonTooltip (MainDlg,
		headingStringId == NULL ? L"CipherShed" : GetString (headingStringId), 
		textStringId == NULL ? L" " : GetString (textStringId), 
		TRUE);
}


// Either of the pointers may be NULL
void WarningBalloonDirect (const wchar_t *headingString, const wchar_t *textString)
{
	if (Silent) 
		return;

	TaskBarIconDisplayBalloonTooltip (MainDlg,
		headingString == NULL ? L"CipherShed" : headingString, 
		textString == NULL ? L" " : textString, 
		TRUE);
}


int Info (char *stringId)
{
	if (Silent) return 0;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONINFORMATION);
}


int InfoTopMost (char *stringId)
{
	if (Silent) return 0;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONINFORMATION | MB_SETFOREGROUND | MB_TOPMOST);
}


int InfoDirect (const wchar_t *msg)
{
	if (Silent) return 0;
	return MessageBoxW (MainDlg, (LPCWSTR)msg, (LPCWSTR)lpszTitle, MB_ICONINFORMATION);
}


int Warning (char *stringId)
{
	if (Silent) return 0;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONWARNING);
}


int WarningTopMost (char *stringId)
{
	if (Silent) return 0;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
}


int WarningDirect (const wchar_t *warnMsg)
{
	if (Silent) return 0;
	return MessageBoxW (MainDlg, (LPCWSTR)warnMsg, (LPCWSTR)lpszTitle, MB_ICONWARNING);
}


int Error (const char *stringId)
{
	if (Silent) return 0;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONERROR);
}


int ErrorTopMost (char *stringId)
{
	if (Silent) return 0;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
}


int ErrorDirect (const wchar_t *errMsg)
{
	return ErrorDirectTitle(errMsg, lpszTitle);
}

int ErrorDirectTitle (const wchar_t* errMsg, const wchar_t* title)
{
	if (Silent) return 0;
	return MessageBoxW (MainDlg, (LPCWSTR)errMsg, (LPCWSTR)title, MB_ICONERROR);
}


int AskYesNo (char *stringId)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
}


int AskYesNoString (const wchar_t *str)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)str, (LPCWSTR)lpszTitle, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
}


int AskYesNoTopmost (char *stringId)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1 | MB_SETFOREGROUND | MB_TOPMOST);
}


int AskNoYes (char *stringId)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2);
}


int AskOkCancel (char *stringId)
{
	if (Silent) return IDCANCEL;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON1);
}


int AskWarnYesNo (char *stringId)
{
	return AskWarnYesNoTitle(stringId, lpszTitle);
}

int AskWarnYesNoTitle (char* stringId, wchar_t* title)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)title, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1);
}


int AskWarnYesNoString (const wchar_t *string)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)string, (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1);
}


int AskWarnYesNoTopmost (char *stringId)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1 | MB_SETFOREGROUND | MB_TOPMOST);
}


int AskWarnYesNoStringTopmost (const wchar_t *string)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)string, (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON1 | MB_SETFOREGROUND | MB_TOPMOST);
}


int AskWarnNoYes (char *stringId)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
}


int AskWarnNoYesString (const wchar_t *string)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)string, (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
}


int AskWarnNoYesTopmost (char *stringId)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2 | MB_SETFOREGROUND | MB_TOPMOST);
}


int AskWarnOkCancel (char *stringId)
{
	if (Silent) return IDCANCEL;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON1);
}


int AskWarnCancelOk (char *stringId)
{
	if (Silent) return IDCANCEL;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2);
}


int AskErrYesNo (char *stringId)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONERROR | MB_YESNO | MB_DEFBUTTON1);
}


int AskErrNoYes (char *stringId)
{
	if (Silent) return IDNO;
	return MessageBoxW (MainDlg, (LPCWSTR)GetString (stringId), (LPCWSTR)lpszTitle, MB_ICONERROR | MB_YESNO | MB_DEFBUTTON2);
}

void InconsistencyResolved (char *techInfo)
{
	wchar_t finalMsg[8024];

	swprintf (finalMsg, ARRAYSIZE(finalMsg), GetString ("INCONSISTENCY_RESOLVED"), techInfo);
	MessageBoxW (MainDlg, (LPCWSTR)finalMsg, (LPCWSTR)lpszTitle, MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
}


void ReportUnexpectedState (char *techInfo)
{
	wchar_t finalMsg[8024];

	swprintf (finalMsg, ARRAYSIZE(finalMsg), GetString ("UNEXPECTED_STATE"), techInfo);
	MessageBoxW (MainDlg, (LPCWSTR)finalMsg, (LPCWSTR)lpszTitle, MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
}

