/*
 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of Encryption for the Masses 2.02a, which is
 Copyright (c) 1998-2000 Paul Le Roux and which is governed by the 'License
 Agreement for Encryption for the Masses'. Modifications and additions to
 the original source code (contained in this file) and all other portions
 of this file are Copyright (c) 2003-2012 TrueCrypt Developers Association
 and are governed by the TrueCrypt License 3.0 the full text of which is
 contained in the file License.txt included in TrueCrypt binary and source
 code distribution packages. */

#include "../Common/Tcdefs.h"
#include <SrRestorePtApi.h>
#include <io.h>
#include <propkey.h>
#include <propvarutil.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../Common/Apidrvr.h"
#include "../Common/BootEncryption.h"
#include "../Boot/Windows/BootCommon.h"
#include "../Common/Combo.h"
#include "ComSetup.h"
#include "../Common/Dlgcode.h"
#include "../Common/Language.h"
#include "../Common/Registry.h"
#include "Resource.h"

#include "Dir.h"
#include "Setup.h"
#include "SelfExtract.h"
#include "Wizard.h"

#include "../Common/Resource.h"
#include "../Common/snprintf.h"

using namespace CipherShed;

#pragma warning( disable : 4201 )
#pragma warning( disable : 4115 )

#include <shlobj.h>

#pragma warning( default : 4201 )
#pragma warning( default : 4115 )

char UninstallationPath[TC_MAX_PATH];
char InstallationPath[TC_MAX_PATH];
char SetupFilesDir[TC_MAX_PATH];
char UninstallBatch[MAX_PATH];

LONG InstalledVersion = 0;
BOOL bUninstall = FALSE;
BOOL bRestartRequired = FALSE;
BOOL bMakePackage = FALSE;
BOOL bDone = FALSE;
BOOL Rollback = FALSE;
BOOL bCipherShedMigration = FALSE;
BOOL bUpgrade = FALSE;
BOOL bDowngrade = FALSE;
BOOL SystemEncryptionUpdate = FALSE;
BOOL PortableMode = FALSE;
BOOL bRepairMode = FALSE;
BOOL bChangeMode = FALSE;
BOOL bDevm = FALSE;
BOOL bPossiblyFirstTimeInstall = FALSE;
BOOL bUninstallInProgress = FALSE;
BOOL UnloadDriver = TRUE;

BOOL bSystemRestore = TRUE;
BOOL bDisableSwapFiles = FALSE;
BOOL bForAllUsers = TRUE;
BOOL bRegisterFileExt = TRUE;
BOOL bAddToStartMenu = TRUE;
BOOL bDesktopIcon = TRUE;

BOOL bDesktopIconStatusDetermined = FALSE;

HMODULE volatile SystemRestoreDll = 0;

BOOL quiet = FALSE;

void localcleanup (void)
{
	localcleanupwiz ();
	cleanup ();

	CloseAppSetupMutex ();
}

BOOL StatDeleteFile (char *lpszFile)
{
	struct __stat64 st;

	if (_stat64 (lpszFile, &st) == 0)
		return DeleteFile (lpszFile);
	else
		return TRUE;
}

BOOL StatRemoveDirectory (char *lpszDir)
{
	struct __stat64 st;

	if (_stat64 (lpszDir, &st) == 0)
		return RemoveDirectory (lpszDir);
	else
		return TRUE;
}

HRESULT CreateLink (char *lpszPathObj, char *lpszArguments,
	    char *lpszPathLink)
{
	HRESULT hres;
	IShellLink *psl;

	/* Get a pointer to the IShellLink interface.  */
	hres = CoCreateInstance (CLSID_ShellLink, NULL,
			       CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl);
	if (SUCCEEDED (hres))
	{
		IPersistFile *ppf;

		/* Set the path to the shortcut target, and add the
		   description.  */
		psl->SetPath (lpszPathObj);
		psl->SetArguments (lpszArguments);

		// Application ID
		if (strstr (lpszPathObj, TC_APP_NAME ".exe"))
		{
			IPropertyStore *propStore;

			if (SUCCEEDED (psl->QueryInterface (IID_PPV_ARGS (&propStore))))
			{
				PROPVARIANT propVariant;
				if (SUCCEEDED (InitPropVariantFromString (TC_APPLICATION_ID, &propVariant)))
				{
					if (SUCCEEDED (propStore->SetValue (PKEY_AppUserModel_ID, propVariant)))
						propStore->Commit();

					PropVariantClear (&propVariant);
				}

				propStore->Release();
			}
		}

		/* Query IShellLink for the IPersistFile interface for saving
		   the shortcut in persistent storage.  */
		hres = psl->QueryInterface (IID_IPersistFile,
						    (void **) &ppf);

		if (SUCCEEDED (hres))
		{
			wchar_t wsz[TC_MAX_PATH];

			/* Ensure that the string is ANSI.  */
			MultiByteToWideChar (CP_ACP, 0, lpszPathLink, -1,
					     wsz, sizeof(wsz) / sizeof(wsz[0]));

			/* Save the link by calling IPersistFile::Save.  */
			hres = ppf->Save (wsz, TRUE);
			ppf->Release ();
		}
		psl->Release ();
	}
	return hres;
}

void GetProgramPath (HWND hwndDlg, char *path)
{
	ITEMIDLIST *i;
	HRESULT res;

	if (bForAllUsers)
        res = SHGetSpecialFolderLocation (hwndDlg, CSIDL_COMMON_PROGRAMS, &i);
	else
        res = SHGetSpecialFolderLocation (hwndDlg, CSIDL_PROGRAMS, &i);

	SHGetPathFromIDList (i, path);
}

void StatusMessage (HWND hwndDlg, char *stringId)
{
	if (Rollback)
		return;

	SendMessageW (GetDlgItem (hwndDlg, IDC_LOG_WINDOW), LB_ADDSTRING, 0, (LPARAM) GetString (stringId));

	SendDlgItemMessage (hwndDlg, IDC_LOG_WINDOW, LB_SETTOPINDEX, 
		SendDlgItemMessage (hwndDlg, IDC_LOG_WINDOW, LB_GETCOUNT, 0, 0) - 1, 0);
}

void StatusMessageParam (HWND hwndDlg, char *stringId, char *param)
{
	wchar_t szTmp[1024];

	if (Rollback)
		return;

	wsprintfW (szTmp, L"%s %hs", GetString (stringId), param);
	SendMessageW (GetDlgItem (hwndDlg, IDC_LOG_WINDOW), LB_ADDSTRING, 0, (LPARAM) szTmp);
		
	SendDlgItemMessage (hwndDlg, IDC_LOG_WINDOW, LB_SETTOPINDEX, 
		SendDlgItemMessage (hwndDlg, IDC_LOG_WINDOW, LB_GETCOUNT, 0, 0) - 1, 0);
}

void ClearLogWindow (HWND hwndDlg)
{
	SendMessage (GetDlgItem (hwndDlg, IDC_LOG_WINDOW), LB_RESETCONTENT, 0, 0);
}

void RegMessage (HWND hwndDlg, char *txt)
{
	StatusMessageParam (hwndDlg, "ADDING_REG", txt);
}

void CopyMessage (HWND hwndDlg, char *txt)
{
	StatusMessageParam (hwndDlg, "INSTALLING", txt);
}

void RemoveMessage (HWND hwndDlg, char *txt)
{
	if (!Rollback)
		StatusMessageParam (hwndDlg, "REMOVING", txt);
}

void IconMessage (HWND hwndDlg, char *txt)
{
	StatusMessageParam (hwndDlg, "ADDING_ICON", txt);
}

/**
 * Determines the installed kernel driver version and sets InstalledVersion / bUpgrade / bDowngrade globals accordingly.
 * @param	[in] BOOL bCloseDriverHandle	When set to true the driver handle will be closed afterwards.
 * @param	[out] LONG *driverVersionPtr	Pointer to a 32bit integer describing the installed kernel driver version.
 * @return	void.
 */
void DetermineUpgradeDowngradeStatus (BOOL bCloseDriverHandle, LONG *driverVersionPtr)
{
	/* Defaults to the callers version number if no installed driver is found. */
	LONG driverVersion = VERSION_NUM;

	/* Singleton pattern. */
	if (hDriver == INVALID_HANDLE_VALUE)
		DriverAttach();

	/* Check if the driver was opened successfully. */
	if (hDriver != INVALID_HANDLE_VALUE)
	{
		DWORD dwResult;

		/* Send a ioctl to determine the driver version. */
		BOOL bResult = DeviceIoControl (hDriver, TC_IOCTL_GET_DRIVER_VERSION, NULL, 0, &driverVersion, sizeof (driverVersion), &dwResult, NULL);

		if (!bResult)
			bResult = DeviceIoControl (hDriver, TC_IOCTL_LEGACY_GET_DRIVER_VERSION, NULL, 0, &driverVersion, sizeof (driverVersion), &dwResult, NULL);

		if (bResult)
			InstalledVersion = driverVersion;

		bUpgrade = (bResult && driverVersion < VERSION_NUM);
		bDowngrade = (bResult && driverVersion > VERSION_NUM);

		/* TrueCrypt to CipherShed migration flag. */
		bCipherShedMigration = (bUpgrade && driverVersion < 0x730);

		/* Determine if the driver was loaded in portable mode. */
		PortableMode = DeviceIoControl (hDriver, TC_IOCTL_GET_PORTABLE_MODE_STATUS, NULL, 0, NULL, 0, &dwResult, NULL);

		if (bCloseDriverHandle)
		{
			CloseHandle (hDriver);
			hDriver = INVALID_HANDLE_VALUE;
		}
	}

	*driverVersionPtr = driverVersion;
}


static BOOL IsFileInUse (const string &filePath)
{
	HANDLE useTestHandle = CreateFile (filePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (useTestHandle != INVALID_HANDLE_VALUE)
		CloseHandle (useTestHandle);
	else if (GetLastError() == ERROR_SHARING_VIOLATION)
		return TRUE;

	return FALSE;
}


BOOL DoFilesInstall (HWND hwndDlg, char *szDestDir)
{
	/* WARNING: Note that, despite its name, this function is used during UNinstallation as well. */

	char szTmp[TC_MAX_PATH];
	BOOL bOK = TRUE;
	int i, x, fileNo;
	char curFileName [TC_MAX_PATH] = {0};

	if (!bUninstall && !bDevm)
	{
		// Self-extract all files to memory

		GetModuleFileName (NULL, szTmp, sizeof (szTmp));

		if (!SelfExtractInMemory (szTmp))
			return FALSE;
	}

	x = strlen (szDestDir);
	if (x < 2)
		return FALSE;

	if (szDestDir[x - 1] != '\\')
		strcat_s (szDestDir, sizeof(szDestDir), "\\");

	for (i = 0; i < sizeof (szFiles) / sizeof (szFiles[0]); i++)
	{
		BOOL bResult;
		char szDir[TC_MAX_PATH];

		if (strstr (szFiles[i], "CipherShed-Setup") != 0)
		{
			if (bUninstall)
				continue;	// Prevent 'access denied' error

			if (bRepairMode)
				continue;	// Destination = target
		}

		if (*szFiles[i] == 'A')
			strcpy (szDir, szDestDir);
		else if (*szFiles[i] == 'D')
		{
			GetSystemDirectory (szDir, sizeof (szDir));

			x = strlen (szDir);
			if (szDir[x - 1] != '\\')
				strcat_s (szDir, sizeof(szDir), "\\");

			strcat_s (szDir, sizeof(szDir), "Drivers\\");
		}
		else if (*szFiles[i] == 'W')
			GetWindowsDirectory (szDir, sizeof (szDir));

		if (*szFiles[i] == 'I')
			continue;

		snprintf (szTmp, sizeof(szTmp), "%s%s", szDir, szFiles[i] + 1);

		if (bUninstall == FALSE)
			CopyMessage (hwndDlg, szTmp);
		else
			RemoveMessage (hwndDlg, szTmp);

		if (bUninstall == FALSE)
		{
			SetCurrentDirectory (SetupFilesDir);

			if (strstr (szFiles[i], "CipherShed-Setup") != 0)
			{
				// Copy ourselves (the distribution package) to the destination location as 'CipherShed-Setup.exe'

				char mp[MAX_PATH];

				GetModuleFileName (NULL, mp, sizeof (mp));
				bResult = TCCopyFile (mp, szTmp);
			}
			else
			{
				BOOL driver64 = FALSE;

				strncpy (curFileName, szFiles[i] + 1, strlen (szFiles[i]) - 1);
				curFileName [strlen (szFiles[i]) - 1] = 0;

				if (Is64BitOs ()
					&& strcmp (szFiles[i], "Dtruecrypt.sys") == 0)
				{
					driver64 = TRUE;
					strncpy (curFileName, FILENAME_64BIT_DRIVER, sizeof (FILENAME_64BIT_DRIVER));
				}

				if (!bDevm)
				{
					bResult = FALSE;

					// Find the correct decompressed file in memory
					for (fileNo = 0; fileNo < NBR_COMPRESSED_FILES; fileNo++)
					{
						// Write the file (stored in memory) directly to the destination location 
						// (there will be no temporary files).
						if (memcmp (
							curFileName, 
							Decompressed_Files[fileNo].fileName, 
							min (strlen (curFileName), (size_t) Decompressed_Files[fileNo].fileNameLength)) == 0)
						{
							// Dump filter driver cannot be installed to SysWOW64 directory
							if (driver64 && !EnableWow64FsRedirection (FALSE))
							{
								handleWin32Error (hwndDlg);
								bResult = FALSE;
								goto err;
							}

							bResult = SaveBufferToFile (
								(char *) Decompressed_Files[fileNo].fileContent,
								szTmp,
								Decompressed_Files[fileNo].fileLength, 
								FALSE);

							if (driver64)
							{
								if (!EnableWow64FsRedirection (TRUE))
								{
									handleWin32Error (hwndDlg);
									bResult = FALSE;
									goto err;
								}

								if (!bResult)
									goto err;

								if (bUpgrade && InstalledVersion < 0x700)
								{
									bResult = WriteLocalMachineRegistryString ("SYSTEM\\CurrentControlSet\\Services\\truecrypt", "ImagePath", "System32\\drivers\\truecrypt.sys", TRUE);
									if (!bResult)
									{
										handleWin32Error (hwndDlg);
										goto err;
									}

									DeleteFile (szTmp);
								}
							}

							break;
						}
					}
				}
				else
				{
					if (driver64)
						EnableWow64FsRedirection (FALSE);

					bResult = TCCopyFile (curFileName, szTmp);

					if (driver64)
						EnableWow64FsRedirection (TRUE);
				}

				if (bResult && strcmp (szFiles[i], "ACipherShed.exe") == 0)
				{
					string servicePath = GetServiceConfigPath (TC_APP_NAME ".exe");
					if (FileExists (servicePath.c_str()))
					{
						CopyMessage (hwndDlg, (char *) servicePath.c_str());
						bResult = CopyFile (szTmp, servicePath.c_str(), FALSE);
					}
				}
			}
		}
		else
		{
			bResult = StatDeleteFile (szTmp);
		}

err:
		if (bResult == FALSE)
		{
			LPVOID lpMsgBuf;
			DWORD dwError = GetLastError ();
			wchar_t szTmp2[700];

			FormatMessage (
					      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					      NULL,
					      dwError,
				 MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),	/* Default language */
					      (char *) &lpMsgBuf,
					      0,
					      NULL
				);


			if (bUninstall == FALSE)
				wsprintfW (szTmp2, GetString ("INSTALL_OF_FAILED"), szTmp, lpMsgBuf);
			else
				wsprintfW (szTmp2, GetString ("UNINSTALL_OF_FAILED"), szTmp, lpMsgBuf);

			LocalFree (lpMsgBuf);

			if (!Silent && MessageBoxW (hwndDlg, szTmp2, lpszTitle, MB_YESNO | MB_ICONHAND) != IDYES)
				return FALSE;
		}
	}

	// Language pack
	if (bUninstall == FALSE)
	{
		WIN32_FIND_DATA f;
		HANDLE h;
		
		SetCurrentDirectory (SetupFilesDir);
		h = FindFirstFile ("Language.*.xml", &f);

		if (h != INVALID_HANDLE_VALUE)
		{
			char d[MAX_PATH*2];
			snprintf (d, sizeof(d), "%s%s", szDestDir, f.cFileName);
			CopyMessage (hwndDlg, d);
			TCCopyFile (f.cFileName, d);
			FindClose (h);
		}

		SetCurrentDirectory (SetupFilesDir);
		SetCurrentDirectory ("Setup files");
		h = FindFirstFile ("CipherShed User Guide.*.pdf", &f);
		if (h != INVALID_HANDLE_VALUE)
		{
			char d[MAX_PATH*2];
			snprintf (d, sizeof(d), "%s%s", szDestDir, f.cFileName);
			CopyMessage (hwndDlg, d);
			TCCopyFile (f.cFileName, d);
			FindClose (h);
		}
		SetCurrentDirectory (SetupFilesDir);
	}

	return bOK;
}

/**
 * Uninstall TrueCrypt program files.
 * @param	[in] HWND hwndDlg	Dialog window handle.
 * @return	BOOL	False if the directory could not be deleted.
 */
BOOL DoTrueCryptFilesUninstall (HWND hwndDlg)
{
	char path[MAX_PATH] = { 0 };
	BOOL bOK = TRUE;

	/* TrueCrypt.exe */
	if (_snprintf (path, sizeof (path) - 1, "%s%s", UninstallationPath, TC_APP_NAME_LEGACY ".exe") >= 0)
	{
		RemoveMessage (hwndDlg, path);
		StatDeleteFile (path);
	}

	/* TrueCrypt Format.exe */
	if (_snprintf (path, sizeof (path) - 1, "%s%s", UninstallationPath, TC_APP_NAME_LEGACY " Format.exe") >= 0)
	{
		RemoveMessage (hwndDlg, path);
		StatDeleteFile (path);
	}

	/* TrueCrypt Setup.exe */
	if (_snprintf (path, sizeof (path) - 1, "%s%s", UninstallationPath, TC_APP_NAME_LEGACY " Setup.exe") >= 0)
	{
		RemoveMessage (hwndDlg, path);
		StatDeleteFile (path);
	}

	/* TrueCrypt User Guide.pdf */
	if (_snprintf (path, sizeof (path) - 1, "%s%s", UninstallationPath, TC_APP_NAME_LEGACY " User Guide.pdf") >= 0)
	{
		RemoveMessage (hwndDlg, path);
		StatDeleteFile (path);
	}

	if (strcmp(InstallationPath, UninstallationPath) != 0)
	{
		/* truecrypt.sys */
		if (_snprintf (path, sizeof (path) - 1, "%s%s", UninstallationPath, "truecrypt.sys") >= 0)
		{
			RemoveMessage (hwndDlg, path);
			StatDeleteFile (path);
		}

		/* truecrypt-x64.sys */
		if (_snprintf (path, sizeof (path) - 1, "%s%s", UninstallationPath, "truecrypt-x64.sys") >= 0)
		{
			RemoveMessage (hwndDlg, path);
			StatDeleteFile (path);
		}

		/* License.txt */
		if (_snprintf (path, sizeof (path) - 1, "%s%s", UninstallationPath, "License.txt") >= 0)
		{
			RemoveMessage (hwndDlg, path);
			StatDeleteFile (path);
		}

		strcpy(path, UninstallationPath);

		/* Remove trailing backslash. */
		size_t len = strlen (path);
		if (path[len - 1] == '\\')
			path[len - 1] = 0;

		/* Remove directory */
		RemoveMessage (hwndDlg, path);
		if (!StatRemoveDirectory (path))
		{
			handleWin32Error (hwndDlg);
			bOK = FALSE;
		}
	}

	return bOK;
}

BOOL DoRegInstall (HWND hwndDlg, char *szDestDir, BOOL bInstallType)
{
	char szDir[TC_MAX_PATH], *key;
	char szTmp[TC_MAX_PATH*4];
	HKEY hkey = 0;
	BOOL bSlash, bOK = FALSE;
	DWORD dw;
	int x;

	strcpy (szDir, szDestDir);
	x = strlen (szDestDir);
	if (szDestDir[x - 1] == '\\')
		bSlash = TRUE;
	else
		bSlash = FALSE;

	if (bSlash == FALSE)
		strcat_s (szDir, sizeof(szDir), "\\");

	/* CipherShed registry migration. */
	if (bCipherShedMigration)
	{
		/* Gui autorun entry. */
		char regk[64];
		char exe[MAX_PATH * 2] = { 0 }; // terminating null character

		GetStartupRegKeyName (regk);
		ReadRegistryString (regk, "TrueCrypt", "", szTmp, sizeof (szTmp));

		if (strstr (szTmp, "\\TrueCrypt.exe") &&
			_snprintf (exe, sizeof (exe) - 32, "\"%sCipherShed.exe\" /q preferences /a logon", szDir) >= 0)
		{
			if (strstr (szTmp, " /a devices"))
				strcat_s (exe, sizeof(exe), " /a devices");
			if (strstr (szTmp, " /a favorites"))
				strcat_s (exe, sizeof(exe), " /a favorites");

			WriteRegistryString (regk, "CipherShed", exe);
		}

		DeleteRegistryValue (regk, "TrueCrypt");
	}

	else if (SystemEncryptionUpdate)
	{
		if (RegCreateKeyEx (HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TrueCrypt",
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dw) == ERROR_SUCCESS)
		{
			strcpy (szTmp, VERSION_STRING);
			RegSetValueEx (hkey, "DisplayVersion", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1);

			strcpy (szTmp, TC_HOMEPAGE);
			RegSetValueEx (hkey, "URLInfoAbout", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1);

			RegCloseKey (hkey);
		}

		return TRUE;
	}

	if (bInstallType)
	{

		key = "Software\\Classes\\TrueCryptVolume";
		RegMessage (hwndDlg, key);
		if (RegCreateKeyEx (HKEY_LOCAL_MACHINE,
				    key,
				    0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dw) != ERROR_SUCCESS)
			goto error;

		strcpy (szTmp, "CipherShed Volume");
		if (RegSetValueEx (hkey, "", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
			goto error;

		snprintf (szTmp, sizeof(szTmp), "%ws", TC_APPLICATION_ID);
		if (RegSetValueEx (hkey, "AppUserModelID", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
			goto error;

		RegCloseKey (hkey);
		hkey = 0;

		key = "Software\\Classes\\TrueCryptVolume\\DefaultIcon";
		RegMessage (hwndDlg, key);
		if (RegCreateKeyEx (HKEY_LOCAL_MACHINE,
				    key,
				    0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dw) != ERROR_SUCCESS)
			goto error;

		snprintf (szTmp, sizeof(szTmp), "%sCipherShed.exe,1", szDir);
		if (RegSetValueEx (hkey, "", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
			goto error;

		RegCloseKey (hkey);
		hkey = 0;

		key = "Software\\Classes\\TrueCryptVolume\\Shell\\open\\command";
		RegMessage (hwndDlg, key);
		if (RegCreateKeyEx (HKEY_LOCAL_MACHINE,
				    key,
				    0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dw) != ERROR_SUCCESS)
			goto error;

		snprintf (szTmp, sizeof(szTmp), "\"%sCipherShed.exe\" /v \"%%1\"", szDir );
		if (RegSetValueEx (hkey, "", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
			goto error;

		RegCloseKey (hkey);
		hkey = 0;

		key = "Software\\Classes\\.tc";
		BOOL typeClassChanged = TRUE;
		char typeClass[256];
		DWORD typeClassSize = sizeof (typeClass);

		if (ReadLocalMachineRegistryString (key, "", typeClass, &typeClassSize) && typeClassSize > 0 && strcmp (typeClass, "TrueCryptVolume") == 0)
			typeClassChanged = FALSE;

		RegMessage (hwndDlg, key);
		if (RegCreateKeyEx (HKEY_LOCAL_MACHINE,
				    key,
				    0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dw) != ERROR_SUCCESS)
			goto error;

		strcpy (szTmp, "TrueCryptVolume");
		if (RegSetValueEx (hkey, "", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
			goto error;
		
		RegCloseKey (hkey);
		hkey = 0;

		if (typeClassChanged)
			SHChangeNotify (SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}

	key = "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TrueCrypt";
	RegMessage (hwndDlg, key);
	if (RegCreateKeyEx (HKEY_LOCAL_MACHINE,
		key,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dw) != ERROR_SUCCESS)
		goto error;

	/* IMPORTANT: IF YOU CHANGE THIS IN ANY WAY, REVISE AND UPDATE SetInstallationPath() ACCORDINGLY! */ 
	snprintf (szTmp, sizeof(szTmp), "\"%sCipherShed-Setup.exe\" /u", szDir);
	if (RegSetValueEx (hkey, "UninstallString", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
		goto error;

	snprintf (szTmp, sizeof(szTmp), "\"%sCipherShed-Setup.exe\" /c", szDir);
	if (RegSetValueEx (hkey, "ModifyPath", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
		goto error;

	snprintf (szTmp, sizeof(szTmp), "\"%sCipherShed-Setup.exe\"", szDir);
	if (RegSetValueEx (hkey, "DisplayIcon", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
		goto error;

	strcpy (szTmp, VERSION_STRING);
	if (RegSetValueEx (hkey, "DisplayVersion", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
		goto error;
		
	strcpy (szTmp, "CipherShed");
	if (RegSetValueEx (hkey, "DisplayName", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
		goto error;

	strcpy (szTmp, "CipherShed Project");
	if (RegSetValueEx (hkey, "Publisher", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
		goto error;

	strcpy (szTmp, TC_HOMEPAGE);
	if (RegSetValueEx (hkey, "URLInfoAbout", 0, REG_SZ, (BYTE *) szTmp, strlen (szTmp) + 1) != ERROR_SUCCESS)
		goto error;

	RegCloseKey (hkey);
	hkey = 0;

	/* CipherShed. */
	key = "Software\\CipherShed";
	RegMessage (hwndDlg, key);
	if (RegCreateKeyEx (HKEY_LOCAL_MACHINE,
		key,
		0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dw) != ERROR_SUCCESS)
		goto error;

	if (RegSetValueEx (hkey, "InstallationPath", 0, REG_SZ, (BYTE *) szDir, strlen (szDir) + 1) != ERROR_SUCCESS)
		goto error;

	if (bCipherShedMigration && RegSetValueEx (hkey, "MigrationPath", 0, REG_SZ, (BYTE *) UninstallationPath, strlen (UninstallationPath) + 1) != ERROR_SUCCESS)
		goto error;

	if (bCipherShedMigration && RegSetValueEx (hkey, "MigrationVersion", 0, REG_DWORD, (BYTE *) &InstalledVersion, sizeof (InstalledVersion)) != ERROR_SUCCESS)
		goto error;

	bOK = TRUE;

error:
	if (hkey != 0)
		RegCloseKey (hkey);

	if (bOK == FALSE)
	{
		handleWin32Error (hwndDlg);
		Error ("REG_INSTALL_FAILED");
	}
	
	// Register COM servers for UAC
	if (IsOSAtLeast (WIN_VISTA))
	{
		if (!RegisterComServers (szDir))
		{
			Error ("COM_REG_FAILED");
			return FALSE;
		}
	}

	return bOK;
}

BOOL DoApplicationDataUninstall (HWND hwndDlg)
{
	char path[MAX_PATH];
	char path2[MAX_PATH];
	BOOL bOK = TRUE;

	StatusMessage (hwndDlg, "REMOVING_APPDATA");

	SHGetFolderPath (NULL, CSIDL_APPDATA, NULL, 0, path);
	strcat_s (path, sizeof(path), "\\TrueCrypt\\");

	// Delete favorite volumes file
	snprintf (path2, sizeof(path2), "%s%s", path, TC_APPD_FILENAME_FAVORITE_VOLUMES);
	RemoveMessage (hwndDlg, path2);
	StatDeleteFile (path2);

	// Delete keyfile defaults
	snprintf (path2, sizeof(path2), "%s%s", path, TC_APPD_FILENAME_DEFAULT_KEYFILES);
	RemoveMessage (hwndDlg, path2);
	StatDeleteFile (path2);

	// Delete history file
	snprintf (path2, sizeof(path2), "%s%s", path, TC_APPD_FILENAME_HISTORY);
	RemoveMessage (hwndDlg, path2);
	StatDeleteFile (path2);
	
	// Delete configuration file
	snprintf (path2, sizeof(path2), "%s%s", path, TC_APPD_FILENAME_CONFIGURATION);
	RemoveMessage (hwndDlg, path2);
	StatDeleteFile (path2);

	// Delete system encryption configuration file
	snprintf (path2, sizeof(path2), "%s%s", path, TC_APPD_FILENAME_SYSTEM_ENCRYPTION);
	RemoveMessage (hwndDlg, path2);
	StatDeleteFile (path2);

	SHGetFolderPath (NULL, CSIDL_APPDATA, NULL, 0, path);
	strcat_s (path, sizeof(path), "\\TrueCrypt");
	RemoveMessage (hwndDlg, path);
	if (!StatRemoveDirectory (path))
	{
		handleWin32Error (hwndDlg);
		bOK = FALSE;
	}

	return bOK;
}

BOOL DoRegUninstall (HWND hwndDlg, BOOL bRemoveDeprecated)
{
	BOOL bOK = FALSE;
	char regk [64];

	// Unregister COM servers
	if (!bRemoveDeprecated && IsOSAtLeast (WIN_VISTA))
	{
		if (!UnregisterComServers (UninstallationPath))
			StatusMessage (hwndDlg, "COM_DEREG_FAILED");
	}

	if (!bRemoveDeprecated)
		StatusMessage (hwndDlg, "REMOVING_REG");

	RegDeleteKey (HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TrueCrypt");
	RegDeleteKey (HKEY_LOCAL_MACHINE, "Software\\Classes\\TrueCryptVolume\\Shell\\open\\command");
	RegDeleteKey (HKEY_LOCAL_MACHINE, "Software\\Classes\\TrueCryptVolume\\Shell\\open");
	RegDeleteKey (HKEY_LOCAL_MACHINE, "Software\\Classes\\TrueCryptVolume\\Shell");
	RegDeleteKey (HKEY_LOCAL_MACHINE, "Software\\Classes\\TrueCryptVolume\\DefaultIcon");
	RegDeleteKey (HKEY_LOCAL_MACHINE, "Software\\Classes\\TrueCryptVolume");
	RegDeleteKey (HKEY_CURRENT_USER, "Software\\TrueCrypt");

	if (!bRemoveDeprecated)
	{
		/* CipherShed. */
		RegDeleteKey (HKEY_LOCAL_MACHINE, "Software\\CipherShed");

		GetStartupRegKeyName (regk);
		DeleteRegistryValue (regk, "CipherShed");

		RegDeleteKey (HKEY_LOCAL_MACHINE, "Software\\Classes\\.tc");
		SHChangeNotify (SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}

	bOK = TRUE;

	if (bOK == FALSE && GetLastError ()!= ERROR_NO_TOKEN && GetLastError ()!= ERROR_FILE_NOT_FOUND
	    && GetLastError ()!= ERROR_PATH_NOT_FOUND)
	{
		handleWin32Error (hwndDlg);
	}
	else
		bOK = TRUE;

	return bOK;
}

/**
 * Uninstall the kernel driver Windows service.
 * Note: The driver will be unloaded, this is fatal in case of full system encryption.
 * @param	[in] HWND hwndDlg		Dialog window handle.
 * @param	[in] char *lpszService	Service name.
 * @return	BOOL					True if successful.
 */
BOOL DoServiceUninstall (HWND hwndDlg, char *lpszService)
{
	SC_HANDLE hManager, hService = NULL;
	BOOL bOK = FALSE, bRet;
	SERVICE_STATUS status;
	BOOL firstTry = TRUE;
	int x;

	memset (&status, 0, sizeof (status));	/* Keep VC6 quiet */

retry:

	/* Service control manager. */
	hManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager == NULL)
		goto error;

	hService = OpenService (hManager, lpszService, SERVICE_ALL_ACCESS);
	if (hService == NULL)
		goto error;

	if (strcmp ("truecrypt", lpszService) == 0)
	{
		try
		{
			BootEncryption bootEnc (hwndDlg);

			if (bootEnc.GetDriverServiceStartType() == SERVICE_BOOT_START)
			{
				/* Uninstall filter drivers. */
				try { bootEnc.RegisterFilterDriver (false, BootEncryption::DriveFilter); } catch (...) { }
				try { bootEnc.RegisterFilterDriver (false, BootEncryption::VolumeFilter); } catch (...) { }
				try { bootEnc.RegisterFilterDriver (false, BootEncryption::DumpFilter); } catch (...) { }
			}
		}
		catch (...) { }

		StatusMessage (hwndDlg, "STOPPING_DRIVER");
	}
	else
		StatusMessageParam (hwndDlg, "STOPPING", lpszService);

#define WAIT_PERIOD 3

	/* Status pending. */
	for (x = 0; x < WAIT_PERIOD; x++)
	{
		bRet = QueryServiceStatus (hService, &status);
		if (bRet != TRUE)
			goto error;

		if (status.dwCurrentState != SERVICE_START_PENDING &&
		    status.dwCurrentState != SERVICE_STOP_PENDING &&
		    status.dwCurrentState != SERVICE_CONTINUE_PENDING)
			break;

		Sleep (1000);
	}

	/* Check if the service is running (driver is loaded). */
	if (status.dwCurrentState != SERVICE_STOPPED)
	{
		/* Stop the service (unload the driver). */
		bRet = ControlService (hService, SERVICE_CONTROL_STOP, &status);
		if (bRet == FALSE)
			goto try_delete;

		/* Status pending. */
		for (x = 0; x < WAIT_PERIOD; x++)
		{
			bRet = QueryServiceStatus (hService, &status);
			if (bRet != TRUE)
				goto error;

			if (status.dwCurrentState != SERVICE_START_PENDING &&
			    status.dwCurrentState != SERVICE_STOP_PENDING &&
			  status.dwCurrentState != SERVICE_CONTINUE_PENDING)
				break;

			Sleep (1000);
		}

		if (status.dwCurrentState != SERVICE_STOPPED && status.dwCurrentState != SERVICE_STOP_PENDING)
			goto error;
	}

try_delete:

	if (strcmp ("truecrypt", lpszService) == 0)
		StatusMessage (hwndDlg, "REMOVING_DRIVER");
	else
		StatusMessageParam (hwndDlg, "REMOVING", lpszService);

	if (hService != NULL)
		CloseServiceHandle (hService);

	if (hManager != NULL)
		CloseServiceHandle (hManager);

	hManager = OpenSCManager (NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hManager == NULL)
		goto error;

	hService = OpenService (hManager, lpszService, SERVICE_ALL_ACCESS);
	if (hService == NULL)
		goto error;

	/* Delete service registry keys. */
	bRet = DeleteService (hService);
	if (bRet == FALSE)
	{
		if (firstTry && GetLastError () == ERROR_SERVICE_MARKED_FOR_DELETE)
		{
			// Second try for an eventual no-install driver instance
			CloseServiceHandle (hService);
			CloseServiceHandle (hManager);

			Sleep(1000);
			firstTry = FALSE;
			goto retry;
		}

		goto error;
	}

	bOK = TRUE;

error:

	if (bOK == FALSE && GetLastError ()!= ERROR_SERVICE_DOES_NOT_EXIST)
	{
		handleWin32Error (hwndDlg);
		MessageBoxW (hwndDlg, GetString ("DRIVER_UINSTALL_FAILED"), lpszTitle, MB_ICONHAND);
	}
	else
		bOK = TRUE;

	if (hService != NULL)
		CloseServiceHandle (hService);

	if (hManager != NULL)
		CloseServiceHandle (hManager);

	return bOK;
}

/**
 * Unload the kernel driver from CipherShed.
 * @param	[in] HWND hwndDlg	Dialog window handle.
 * @return	BOOL				True if successful.
 */
BOOL DoDriverUnload (HWND hwndDlg)
{
	BOOL bOK = TRUE;
	int status;

	/* Open a handle to the driver device. */
	status = DriverAttach ();
	if (status != 0)
	{
		if (status == ERR_OS_ERROR && GetLastError () != ERROR_FILE_NOT_FOUND)
		{
			handleWin32Error (hwndDlg);
			AbortProcess ("NODRIVER");
		}

		if (status != ERR_OS_ERROR)
		{
			handleError (NULL, status);
			AbortProcess ("NODRIVER");
		}
	}

	if (hDriver != INVALID_HANDLE_VALUE)
	{
		MOUNT_LIST_STRUCT driver;
		LONG driverVersion = VERSION_NUM;
		int refCount;
		DWORD dwResult;
		BOOL bResult;

		// Try to determine if it's upgrade (and not reinstall, downgrade, or first-time install).
		DetermineUpgradeDowngradeStatus (FALSE, &driverVersion);

		// Test for encrypted boot drive
		try
		{
			BootEncryption bootEnc (hwndDlg);

			/* The driver starts at boot time in case of full system encryption. */
			if (bootEnc.GetDriverServiceStartType() == SERVICE_BOOT_START)
			{
				try
				{
					// Check hidden OS update consistency
					if (IsHiddenOSRunning() &&
						bootEnc.GetInstalledBootLoaderVersion() != VERSION_NUM &&
						AskWarnNoYes ("UPDATE_TC_IN_DECOY_OS_FIRST") == IDNO)
					{
						AbortProcessSilent ();
					}
				}
				catch (...) { }

				/* Uninstall filter drivers and set driver start to "system start". */
				if (bUninstallInProgress && driverVersion >= 0x500 && !bootEnc.GetStatus().DriveMounted)
				{
					try { bootEnc.RegisterFilterDriver (false, BootEncryption::DriveFilter); } catch (...) { }
					try { bootEnc.RegisterFilterDriver (false, BootEncryption::VolumeFilter); } catch (...) { }
					try { bootEnc.RegisterFilterDriver (false, BootEncryption::DumpFilter); } catch (...) { }
					bootEnc.SetDriverServiceStartType (SERVICE_SYSTEM_START);
				}
				/* Abort if the boot drive is encrypted. */
				else if (bUninstallInProgress || bDowngrade)
				{
					Error (bDowngrade ? "SETUP_FAILED_BOOT_DRIVE_ENCRYPTED_DOWNGRADE" : "SETUP_FAILED_BOOT_DRIVE_ENCRYPTED");
					return FALSE;
				}
				else
				{
					if (CurrentOSMajor == 6 && CurrentOSMinor == 0 && CurrentOSServicePack < 1)
						AbortProcess ("SYS_ENCRYPTION_UPGRADE_UNSUPPORTED_ON_VISTA_SP0");

					SystemEncryptionUpdate = TRUE;
					PortableMode = FALSE;
				}
			}
		}
		catch (...)	{ }

		/* Do not unload the driver in case of upgrade or system encryption update. */
		if (!bUninstall
			&& (bUpgrade || SystemEncryptionUpdate)
			&& (!bDevm || SystemEncryptionUpdate))
		{
			UnloadDriver = FALSE;
		}

		if (PortableMode && !SystemEncryptionUpdate)
			UnloadDriver = TRUE;

		/* Check mounted volumes. */
		if (UnloadDriver)
		{
			int volumesMounted = 0;

			bResult = DeviceIoControl (hDriver, TC_IOCTL_IS_ANY_VOLUME_MOUNTED, NULL, 0, &volumesMounted, sizeof (volumesMounted), &dwResult, NULL);

			if (!bResult)
			{
				bResult = DeviceIoControl (hDriver, TC_IOCTL_LEGACY_GET_MOUNTED_VOLUMES, NULL, 0, &driver, sizeof (driver), &dwResult, NULL);
				if (bResult)
					volumesMounted = driver.ulMountedDrives;
			}

			if (bResult)
			{
				if (volumesMounted != 0)
				{
					/* Abort. */
					bOK = FALSE;
					MessageBoxW (hwndDlg, GetString ("DISMOUNT_ALL_FIRST"), lpszTitle, MB_ICONHAND);
				}
			}
			else
			{
				/* Abort. */
				bOK = FALSE;
				handleWin32Error (hwndDlg);
			}
		}

		// Try to close all open TC windows
		if (bOK)
		{
			BOOL TCWindowClosed = FALSE;

			EnumWindows (CloseTCWindowsEnum, (LPARAM) &TCWindowClosed);

			if (TCWindowClosed) 
				Sleep (2000);
		}

		// Test for any applications attached to driver
		if (!bUpgrade)
		{
			bResult = DeviceIoControl (hDriver, TC_IOCTL_GET_DEVICE_REFCOUNT, &refCount, sizeof (refCount), &refCount,
				sizeof (refCount), &dwResult, NULL);

			if (bOK && bResult && refCount > 1)
			{
				/* Abort. */
				MessageBoxW (hwndDlg, GetString ("CLOSE_TC_FIRST"), lpszTitle, MB_ICONSTOP);
				bOK = FALSE;
			}
		}

		/* Close the driver device handle. */
		if (!bOK || UnloadDriver)
		{
			CloseHandle (hDriver);
			hDriver = INVALID_HANDLE_VALUE;
		}
	}
	else
	{
		// Note that the driver may have already been unloaded during this session (e.g. retry after an error, etc.) so it is not 
		// guaranteed that the user is installing CipherShed for the first time now (we also cannot know if the user has already
		// installed and used CipherShed on another system before).
		bPossiblyFirstTimeInstall = TRUE;
	}

	return bOK;
}

/**
 * Upgrade the installed bootloader.
 * @param	[in] HWND hwndDlg	Dialog window handle.
 * @return	BOOL				Returns true if the upgrade was successful.
 */
BOOL UpgradeBootLoader (HWND hwndDlg)
{
	/*
	 * The bootloader is only installed in case of full system encryption,
	 * otherwise no upgrade is needed and we return here.
	 */
	if (!SystemEncryptionUpdate)
		return TRUE;

	try
	{
		BootEncryption bootEnc (hwndDlg);

		if (bootEnc.GetInstalledBootLoaderVersion() < VERSION_NUM)
		{
			StatusMessage (hwndDlg, "INSTALLER_UPDATING_BOOT_LOADER");

			/* Upgrade the installed bootloader to new version. */
			bootEnc.InstallBootLoader (true);

			/* Give the user an advice to create a new rescue disk with updated bootloader (<= TrueCrypt 6.0a). */
			if (bootEnc.GetInstalledBootLoaderVersion() <= TC_RESCUE_DISK_UPGRADE_NOTICE_MAX_VERSION)
				Info (IsHiddenOSRunning() ? "BOOT_LOADER_UPGRADE_OK_HIDDEN_OS" : "BOOT_LOADER_UPGRADE_OK");
		}

		return TRUE;
	}
	catch (Exception &e)
	{
		e.Show (hwndDlg);
	}
	catch (...) { }

	Error ("BOOT_LOADER_UPGRADE_FAILED");
	return FALSE;
}

/**
 * Remove CipherShed desktop and startmenu shortcuts.
 */
BOOL DoShortcutsUninstall (HWND hwndDlg, char *szDestDir)
{
	char szLinkDir[TC_MAX_PATH];
	char szTmp2[TC_MAX_PATH];
	BOOL bSlash, bOK = FALSE;
	HRESULT hOle;
	int x;
	BOOL allUsers = FALSE;

	hOle = OleInitialize (NULL);

	// User start menu
	SHGetSpecialFolderPath (hwndDlg, szLinkDir, CSIDL_PROGRAMS, 0);
	x = strlen (szLinkDir);
	if (szLinkDir[x - 1] == '\\')
		bSlash = TRUE;
	else
		bSlash = FALSE;

	if (bSlash == FALSE)
		strcat_s (szLinkDir, sizeof(szLinkDir), "\\");

	strcat_s (szLinkDir, sizeof(szLinkDir), "CipherShed");

	// Global start menu
	{
		struct _stat st;
		char path[TC_MAX_PATH];

		SHGetSpecialFolderPath (hwndDlg, path, CSIDL_COMMON_PROGRAMS, 0);
		strcat_s (path, sizeof(path), "\\CipherShed");

		if (_stat (path, &st) == 0)
		{
			strcpy (szLinkDir, path);
			allUsers = TRUE;
		}
	}

	// Start menu entries
	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\CipherShed.lnk");
	RemoveMessage (hwndDlg, szTmp2);
	if (StatDeleteFile (szTmp2) == FALSE)
		goto error;

	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\CipherShed Website.url");
	RemoveMessage (hwndDlg, szTmp2);
	if (StatDeleteFile (szTmp2) == FALSE)
		goto error;

	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\Uninstall CipherShed.lnk");
	RemoveMessage (hwndDlg, szTmp2);
	if (StatDeleteFile (szTmp2) == FALSE)
		goto error;
	
	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\CipherShed User's Guide.lnk");
	DeleteFile (szTmp2);

	// Start menu group
	RemoveMessage ((HWND) hwndDlg, szLinkDir);
	if (StatRemoveDirectory (szLinkDir))
		SHChangeNotify (SHCNE_RMDIR, SHCNF_PATH, szLinkDir, NULL);
	else
		handleWin32Error ((HWND) hwndDlg);

	// Desktop icon
	if (allUsers)
		SHGetSpecialFolderPath (hwndDlg, szLinkDir, CSIDL_COMMON_DESKTOPDIRECTORY, 0);
	else
		SHGetSpecialFolderPath (hwndDlg, szLinkDir, CSIDL_DESKTOPDIRECTORY, 0);

	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\CipherShed.lnk");

	RemoveMessage (hwndDlg, szTmp2);
	if (StatDeleteFile (szTmp2) == FALSE)
		goto error;

	SHChangeNotify (SHCNE_DELETE, SHCNF_PATH, szTmp2, NULL);

	bOK = TRUE;

error:
	OleUninitialize ();

	return bOK;
}

/**
 * Remove TrueCrypt desktop and startmenu shortcuts.
 */
BOOL DoTrueCryptShortcutsUninstall (HWND hwndDlg, char *szDestDir)
{
	char szLinkDir[TC_MAX_PATH];
	char szTmp2[TC_MAX_PATH];
	BOOL bSlash, bOK = FALSE;
	HRESULT hOle;
	int x;
	BOOL allUsers = FALSE;

	hOle = OleInitialize (NULL);

	// User start menu
	SHGetSpecialFolderPath (hwndDlg, szLinkDir, CSIDL_PROGRAMS, 0);
	x = strlen (szLinkDir);
	if (szLinkDir[x - 1] == '\\')
		bSlash = TRUE;
	else
		bSlash = FALSE;

	if (bSlash == FALSE)
		strcat_s (szLinkDir, sizeof(szLinkDir), "\\");

	strcat_s (szLinkDir, sizeof(szLinkDir), "TrueCrypt");

	// Global start menu
	{
		struct _stat st;
		char path[TC_MAX_PATH];

		SHGetSpecialFolderPath (hwndDlg, path, CSIDL_COMMON_PROGRAMS, 0);
		strcat_s (path, sizeof(path), "\\TrueCrypt");

		if (_stat (path, &st) == 0)
		{
			strcpy (szLinkDir, path);
			allUsers = TRUE;
		}
	}

	// Start menu entries
	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\TrueCrypt.lnk");
	RemoveMessage (hwndDlg, szTmp2);
	if (StatDeleteFile (szTmp2) == FALSE)
		goto error;

	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\TrueCrypt Website.url");
	RemoveMessage (hwndDlg, szTmp2);
	if (StatDeleteFile (szTmp2) == FALSE)
		goto error;

	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\Uninstall TrueCrypt.lnk");
	RemoveMessage (hwndDlg, szTmp2);
	if (StatDeleteFile (szTmp2) == FALSE)
		goto error;
	
	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\TrueCrypt User's Guide.lnk");
	DeleteFile (szTmp2);

	// Start menu group
	RemoveMessage ((HWND) hwndDlg, szLinkDir);
	if (StatRemoveDirectory (szLinkDir))
		SHChangeNotify (SHCNE_RMDIR, SHCNF_PATH, szLinkDir, NULL);
	else
		handleWin32Error ((HWND) hwndDlg);

	// Desktop icon
	if (allUsers)
		SHGetSpecialFolderPath (hwndDlg, szLinkDir, CSIDL_COMMON_DESKTOPDIRECTORY, 0);
	else
		SHGetSpecialFolderPath (hwndDlg, szLinkDir, CSIDL_DESKTOPDIRECTORY, 0);

	snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\TrueCrypt.lnk");

	RemoveMessage (hwndDlg, szTmp2);
	if (StatDeleteFile (szTmp2) == FALSE)
		goto error;

	SHChangeNotify (SHCNE_DELETE, SHCNF_PATH, szTmp2, NULL);

	bOK = TRUE;

error:
	OleUninitialize ();

	return bOK;
}

BOOL DoShortcutsInstall (HWND hwndDlg, char *szDestDir, BOOL bProgGroup, BOOL bDesktopIcon)
{
	char szLinkDir[TC_MAX_PATH], szDir[TC_MAX_PATH];
	char szTmp[TC_MAX_PATH], szTmp2[TC_MAX_PATH], szTmp3[TC_MAX_PATH];
	BOOL bSlash, bOK = FALSE;
	HRESULT hOle;
	int x;

	if (bProgGroup == FALSE && bDesktopIcon == FALSE)
		return TRUE;

	hOle = OleInitialize (NULL);

	GetProgramPath (hwndDlg, szLinkDir);

	x = strlen (szLinkDir);
	if (szLinkDir[x - 1] == '\\')
		bSlash = TRUE;
	else
		bSlash = FALSE;

	if (bSlash == FALSE)
		strcat_s (szLinkDir, sizeof(szLinkDir), "\\");

	strcat_s (szLinkDir, sizeof(szLinkDir), "CipherShed");

	strcpy (szDir, szDestDir);
	x = strlen (szDestDir);
	if (szDestDir[x - 1] == '\\')
		bSlash = TRUE;
	else
		bSlash = FALSE;

	if (bSlash == FALSE)
		strcat_s (szDir, sizeof(szDir), "\\");

	if (bProgGroup)
	{
		FILE *f;

		if (mkfulldir (szLinkDir, TRUE) != 0)
		{
			if (mkfulldir (szLinkDir, FALSE) != 0)
			{
				wchar_t szTmp[TC_MAX_PATH];

				handleWin32Error (hwndDlg);
				wsprintfW (szTmp, GetString ("CANT_CREATE_FOLDER"), szLinkDir);
				MessageBoxW (hwndDlg, szTmp, lpszTitle, MB_ICONHAND);
				goto error;
			}
		}

		snprintf (szTmp, sizeof(szTmp), "%s%s", szDir, "CipherShed.exe");
		snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\CipherShed.lnk");

		IconMessage (hwndDlg, szTmp2);
		if (CreateLink (szTmp, "", szTmp2) != S_OK)
			goto error;

		snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\CipherShed Website.url");
		IconMessage (hwndDlg, szTmp2);
		f = fopen (szTmp2, "w");
		if (f)
		{
			fprintf (f, "[InternetShortcut]\nURL=%s\n", TC_HOMEPAGE);

			CheckFileStreamWriteErrors (f, szTmp2);
			fclose (f);
		}
		else
			goto error;

		snprintf (szTmp, sizeof(szTmp), "%s%s", szDir, "CipherShed-Setup.exe");
		snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\Uninstall CipherShed.lnk");
		strcpy (szTmp3, "/u");

		IconMessage (hwndDlg, szTmp2);
		if (CreateLink (szTmp, szTmp3, szTmp2) != S_OK)
			goto error;

		snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\CipherShed User's Guide.lnk");
		DeleteFile (szTmp2);
	}

	if (bDesktopIcon)
	{
		strcpy (szDir, szDestDir);
		x = strlen (szDestDir);
		if (szDestDir[x - 1] == '\\')
			bSlash = TRUE;
		else
			bSlash = FALSE;

		if (bSlash == FALSE)
			strcat_s (szDir, sizeof(szDir), "\\");

		if (bForAllUsers)
			SHGetSpecialFolderPath (hwndDlg, szLinkDir, CSIDL_COMMON_DESKTOPDIRECTORY, 0);
		else
			SHGetSpecialFolderPath (hwndDlg, szLinkDir, CSIDL_DESKTOPDIRECTORY, 0);

		snprintf (szTmp, sizeof(szTmp), "%s%s", szDir, "CipherShed.exe");
		snprintf (szTmp2, sizeof(szTmp2), "%s%s", szLinkDir, "\\CipherShed.lnk");

		IconMessage (hwndDlg, szTmp2);

		if (CreateLink (szTmp, "", szTmp2) != S_OK)
			goto error;
	}

	bOK = TRUE;

error:
	OleUninitialize ();

	return bOK;
}


void OutcomePrompt (HWND hwndDlg, BOOL bOK)
{
	if (bOK)
	{
		EnableWindow (GetDlgItem ((HWND) hwndDlg, IDCANCEL), FALSE);

		bDone = TRUE;

		if (bUninstall == FALSE)
		{
			if (bDevm)
				PostMessage (MainDlg, WM_CLOSE, 0, 0);
			else if (bPossiblyFirstTimeInstall || bRepairMode || (!bUpgrade && !bDowngrade))
				Info ("INSTALL_OK");
			else
				Info ("SETUP_UPDATE_OK");
		}
		else
		{
			wchar_t str[4096];

			swprintf (str, GetString ("UNINSTALL_OK"), UninstallationPath);
			MessageBoxW (hwndDlg, str, lpszTitle, MB_ICONASTERISK);
		}
	}
	else
	{
		if (bUninstall == FALSE)
			Error ("INSTALL_FAILED");
		else
			Error ("UNINSTALL_FAILED");
	}
}

/**
 * Set windows system restore point.
 * @param	[in] HWND hwndDlg	Dialog window handle.
 * @param	[in] BOOL finalize	If true the restore point is finalized.
 * @return	void
 */
static void SetSystemRestorePoint (HWND hwndDlg, BOOL finalize)
{
	static RESTOREPOINTINFO RestPtInfo;
	static STATEMGRSTATUS SMgrStatus;
	static BOOL failed = FALSE;
	static BOOL (__stdcall *_SRSetRestorePoint)(PRESTOREPOINTINFO, PSTATEMGRSTATUS);

	/* Abort if the system restore dll wasn't loaded successfully. */
	if (!SystemRestoreDll)
		return;

	/* Retrieve the address of SRSetRestorePointA. */
	_SRSetRestorePoint = (BOOL (__stdcall *)(PRESTOREPOINTINFO, PSTATEMGRSTATUS))GetProcAddress (SystemRestoreDll,"SRSetRestorePointA");
	if (_SRSetRestorePoint == 0)
	{
		FreeLibrary (SystemRestoreDll);
		SystemRestoreDll = 0;
		return;
	}

	/* Begin system change. */
	if (!finalize)
	{
		StatusMessage (hwndDlg, "CREATING_SYS_RESTORE");

		RestPtInfo.dwEventType = BEGIN_SYSTEM_CHANGE;
		RestPtInfo.dwRestorePtType = bUninstall ? APPLICATION_UNINSTALL : APPLICATION_INSTALL | DEVICE_DRIVER_INSTALL;
		RestPtInfo.llSequenceNumber = 0;
		strcpy (RestPtInfo.szDescription, bUninstall ? "CipherShed uninstallation" : "CipherShed installation");

		if(!_SRSetRestorePoint (&RestPtInfo, &SMgrStatus)) 
		{
			StatusMessage (hwndDlg, "FAILED_SYS_RESTORE");
			failed = TRUE;
		}
	}

	/* End system change. */
	else if (!failed)
	{
		RestPtInfo.dwEventType = END_SYSTEM_CHANGE;
		RestPtInfo.llSequenceNumber = SMgrStatus.llSequenceNumber;

		if(!_SRSetRestorePoint(&RestPtInfo, &SMgrStatus)) 
		{
			StatusMessage (hwndDlg, "FAILED_SYS_RESTORE");
		}
	}
}

/**
 * Uninstall worker.
 * @param	[in] void *arg	Dialog window handle.
 * @return	void
 */
void DoUninstall (void *arg)
{
	HWND hwndDlg = (HWND) arg;
	BOOL bOK = TRUE;
	BOOL bTempSkipSysRestore = FALSE;

	if (!Rollback)
		EnableWindow (GetDlgItem ((HWND) hwndDlg, IDC_UNINSTALL), FALSE);

	WaitCursor ();

	if (!Rollback)
	{
		ClearLogWindow (hwndDlg);
	}

	/* Unload the kernel driver. */
	if (DoDriverUnload (hwndDlg) == FALSE)
	{
		bOK = FALSE;
		bTempSkipSysRestore = TRUE;		// Volumes are possibly mounted; defer System Restore point creation for this uninstall attempt.
	}
	else
	{
		/* Begin system change. */
		if (!Rollback && bSystemRestore && !bTempSkipSysRestore)
			SetSystemRestorePoint (hwndDlg, FALSE);

		/* Uninstall the kernel driver. */
		if (DoServiceUninstall (hwndDlg, "truecrypt") == FALSE)
		{
			bOK = FALSE;
		}
		/* Uninstall registry keys. */
		else if (DoRegUninstall ((HWND) hwndDlg, FALSE) == FALSE)
		{
			bOK = FALSE;
		}
		/* Remove files. */
		else if (DoFilesInstall ((HWND) hwndDlg, UninstallationPath) == FALSE)
		{
			bOK = FALSE;
		}
		/* Remove desktop and startmenu shortcuts. */
		else if (DoShortcutsUninstall (hwndDlg, UninstallationPath) == FALSE)
		{
			bOK = FALSE;
		}
		/* Remove configs. */
		else if (!DoApplicationDataUninstall (hwndDlg))
		{
			bOK = FALSE;
		}
		else
		{
			char temp[MAX_PATH];
			FILE *f;

			/* Remove driver with deprecated name. */
			DoServiceUninstall (hwndDlg, "TrueCryptService");

			GetTempPath (sizeof (temp), temp);
			_snprintf (UninstallBatch, sizeof (UninstallBatch), "%s\\CipherShed-Uninstall.bat", temp);

			UninstallBatch [sizeof(UninstallBatch)-1] = 0;

			// Create uninstall batch
			f = fopen (UninstallBatch, "w");
			if (!f)
				bOK = FALSE;
			else
			{
				fprintf (f, ":loop\n"
					"del \"%s%s\"\n"
					"if exist \"%s%s\" goto loop\n"
					"rmdir \"%s\"\n"
					"del \"%s\"",
					UninstallationPath, "CipherShed-Setup.exe",
					UninstallationPath, "CipherShed-Setup.exe",
					UninstallationPath,
					UninstallBatch
					);

				CheckFileStreamWriteErrors (f, UninstallBatch);
				fclose (f);
			}
		}
	}

	NormalCursor ();

	if (Rollback)
		return;

	/* End system change. */
	if (bSystemRestore && !bTempSkipSysRestore)
		SetSystemRestorePoint (hwndDlg, TRUE);

	if (bOK)
		PostMessage (hwndDlg, TC_APPMSG_UNINSTALL_SUCCESS, 0, 0);
	else
		bUninstallInProgress = FALSE;

	EnableWindow (GetDlgItem ((HWND) hwndDlg, IDC_UNINSTALL), TRUE);
	OutcomePrompt (hwndDlg, bOK);
}

/**
 * Install / Upgrade worker.
 * @param	[in] void *arg	Dialog window handle.
 * @return	void
 */
void DoInstall (void *arg)
{
	HWND hwndDlg = (HWND) arg;
	BOOL bOK = TRUE;
	char path[MAX_PATH];

	/* BootEncryption instance. */
	BootEncryption bootEnc (hwndDlg);

	// Refresh the main GUI (wizard thread)
	InvalidateRect (MainDlg, NULL, TRUE);

	ClearLogWindow (hwndDlg);

	/* Create the installation folder if it doesn't already exist. */
	if (mkfulldir (InstallationPath, TRUE) != 0)
	{
		if (mkfulldir (InstallationPath, FALSE) != 0)
		{
			wchar_t szTmp[TC_MAX_PATH];

			handleWin32Error (hwndDlg);
			wsprintfW (szTmp, GetString ("CANT_CREATE_FOLDER"), InstallationPath);
			MessageBoxW (hwndDlg, szTmp, lpszTitle, MB_ICONHAND);
			Error ("INSTALL_FAILED");
			PostMessage (MainDlg, TC_APPMSG_INSTALL_FAILURE, 0, 0);
			return;
		}
	}

	UpdateProgressBarProc(2);

	/* Unload the kernel driver, in case of full system encryption the driver is NOT unloaded. */
	if (DoDriverUnload (hwndDlg) == FALSE)
	{
		NormalCursor ();
		PostMessage (MainDlg, TC_APPMSG_INSTALL_FAILURE, 0, 0);
		return;
	}

	/* Check if the previous installed version is running. */
	if (bUpgrade
		&&	(
			/* CipherShed. */
			IsFileInUse (string (InstallationPath) + TC_APP_NAME ".exe")
			|| IsFileInUse (string (InstallationPath) + TC_APP_NAME " Format.exe")
			|| IsFileInUse (string (InstallationPath) + TC_APP_NAME " Setup.exe")
			/* TrueCrypt. */
			|| IsFileInUse (string (UninstallationPath) + TC_APP_NAME_LEGACY ".exe")
			|| IsFileInUse (string (UninstallationPath) + TC_APP_NAME_LEGACY " Format.exe")
			|| IsFileInUse (string (UninstallationPath) + TC_APP_NAME_LEGACY " Setup.exe")
			)
		)
	{
		NormalCursor ();
		Error ("CLOSE_TC_FIRST");
		PostMessage (MainDlg, TC_APPMSG_INSTALL_FAILURE, 0, 0);
		return;
	}

	UpdateProgressBarProc(12);

	/* Begin system restore point. */
	if (bSystemRestore)
		SetSystemRestorePoint (hwndDlg, FALSE);

	UpdateProgressBarProc(48);

	if (bDisableSwapFiles
		&& IsPagingFileActive (FALSE))
	{
		if (!DisablePagingFile())
		{
			handleWin32Error (hwndDlg);
			Error ("FAILED_TO_DISABLE_PAGING_FILES");
		}
		else
			bRestartRequired = TRUE;
	}

	UpdateProgressBarProc(50);

	/* Remove driver with deprecated name. */
	DoServiceUninstall (hwndDlg, "TrueCryptService");

	UpdateProgressBarProc(55);

	/* Remove registry keys before re-install. */
	if (!SystemEncryptionUpdate)
		DoRegUninstall ((HWND) hwndDlg, TRUE);

	/* Install new dump filter driver. */
	if (SystemEncryptionUpdate && InstalledVersion < 0x700)
	{
		try
		{
			bootEnc.RegisterFilterDriver (false, BootEncryption::DumpFilter);
		}
		catch (...) { }

		try
		{
			bootEnc.RegisterFilterDriver (true, BootEncryption::DumpFilter);
		}
		catch (Exception &e)
		{
			try
			{
				bootEnc.RegisterFilterDriver (false, BootEncryption::DumpFilter);
			}
			catch (...) { }

			e.Show (hwndDlg);

			bOK = FALSE;
			goto outcome;
		}

		if (ReadDriverConfigurationFlags() & TC_DRIVER_CONFIG_CACHE_BOOT_PASSWORD_FOR_SYS_FAVORITES)
		{
			WriteLocalMachineRegistryString ("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Minimal\\" TC_SYSTEM_FAVORITES_SERVICE_NAME, NULL, "Service", FALSE);
			WriteLocalMachineRegistryString ("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Network\\" TC_SYSTEM_FAVORITES_SERVICE_NAME, NULL, "Service", FALSE);
		}
	}

	UpdateProgressBarProc(61);

	/* Migrate config file. */
	if (bUpgrade && InstalledVersion < 0x700)
	{
		bool bMountFavoritesOnLogon = ConfigReadInt ("MountFavoritesOnLogon", FALSE) != 0;
		bool bOpenExplorerWindowAfterMount = ConfigReadInt ("OpenExplorerWindowAfterMount", FALSE) != 0;

		if (bMountFavoritesOnLogon || bOpenExplorerWindowAfterMount)
		{
			char *favoritesFilename = GetConfigPath (TC_APPD_FILENAME_FAVORITE_VOLUMES);
			DWORD size;
			char *favoritesXml = LoadFile (favoritesFilename, &size);

			if (favoritesXml && size != 0)
			{
				string favorites;
				favorites.insert (0, favoritesXml, size);

				size_t p = favorites.find ("<volume ");
				while (p != string::npos)
				{
					if (bMountFavoritesOnLogon)
						favorites.insert (p + 8, "mountOnLogOn=\"1\" ");

					if (bOpenExplorerWindowAfterMount)
						favorites.insert (p + 8, "openExplorerWindow=\"1\" ");

					p = favorites.find ("<volume ", p + 1);
				}

				SaveBufferToFile (favorites.c_str(), favoritesFilename, favorites.size(), FALSE);

				if (bMountFavoritesOnLogon)
				{
					char regk[64];
					char regVal[MAX_PATH * 2];

					GetStartupRegKeyName (regk);

					ReadRegistryString (regk, "TrueCrypt", "", regVal, sizeof (regVal));

					if (strstr (regVal, "favorites"))
					{
						strcat_s (regVal, sizeof (regVal), " /a logon");
						WriteRegistryString (regk, "TrueCrypt", regVal);
					}
				}
			}

			if (favoritesXml)
				free (favoritesXml);
		}
	}

	/* Remove deprecated setup file (TrueCrypt 1.0 - 4.2a) */
	GetWindowsDirectory (path, sizeof (path));
	strcat_s (path, sizeof (path), "\\TrueCrypt Setup.exe");
	DeleteFile (path);

	/* Uninstall the old kernel driver, if it was unloaded before (NOT in case of full system encryption). */
	if (UpdateProgressBarProc(63) && UnloadDriver && DoServiceUninstall (hwndDlg, "truecrypt") == FALSE)
	{
		bOK = FALSE;
	}
	/* Install new files. */
	else if (UpdateProgressBarProc(72) && DoFilesInstall ((HWND) hwndDlg, InstallationPath) == FALSE)
	{
		bOK = FALSE;
	}
	/* Install registry keys. */
	else if (UpdateProgressBarProc(80) && DoRegInstall ((HWND) hwndDlg, InstallationPath, bRegisterFileExt) == FALSE)
	{
		bOK = FALSE;
	}
	/* Install the new kernel driver, if it was unloaded before (NOT in case of full system encryption). */
	else if (UpdateProgressBarProc(85) && UnloadDriver && DoDriverInstall (hwndDlg) == FALSE)
	{
		bOK = FALSE;
	}
	/* Upgrade the bootloader in case of full system encryption. */
	else if (UpdateProgressBarProc(90) && SystemEncryptionUpdate && UpgradeBootLoader (hwndDlg) == FALSE)
	{
		bOK = FALSE;
	}
	/* Install desktop / startmenu shortcuts. */
	else if (UpdateProgressBarProc(93) && DoShortcutsInstall (hwndDlg, InstallationPath, bAddToStartMenu, bDesktopIcon) == FALSE)
	{
		bOK = FALSE;
	}

	/* Remove TrueCrypt program files and shortcuts. */
	else if (bCipherShedMigration)
	{
		DoTrueCryptShortcutsUninstall (hwndDlg, UninstallationPath);
		DoTrueCryptFilesUninstall (hwndDlg);
	}

	/* Driver couldn't be unloaded, we need a system restart (full system encryption). */
	if (!UnloadDriver)
		bRestartRequired = TRUE;

	try
	{
		bootEnc.RenameDeprecatedSystemLoaderBackup();
	}
	catch (...)	{ }

	/* Move system favorite volumes config. */
	if (SystemEncryptionUpdate && InstalledVersion == 0x630)
	{
		string sysFavorites = GetServiceConfigPath (TC_APPD_FILENAME_SYSTEM_FAVORITE_VOLUMES);
		string legacySysFavorites = GetProgramConfigPath ("System Favorite Volumes.xml");

		if (FileExists (legacySysFavorites.c_str()) && !FileExists (sysFavorites.c_str()))
			MoveFile (legacySysFavorites.c_str(), sysFavorites.c_str());
	}

	if (bOK)
		UpdateProgressBarProc(97);

	/* End system restore point. */
	if (bSystemRestore)
		SetSystemRestorePoint (hwndDlg, TRUE);

	if (bOK)
	{
		UpdateProgressBarProc(100);
		UninstallBatch[0] = 0;
		StatusMessage (hwndDlg, "INSTALL_COMPLETED");
	}
	else
	{
		UpdateProgressBarProc(0);

		if (!SystemEncryptionUpdate)
		{
			bUninstall = TRUE;
			Rollback = TRUE;
			Silent = TRUE;

			DoUninstall (hwndDlg);

			bUninstall = FALSE;
			Rollback = FALSE;
			Silent = FALSE;

			StatusMessage (hwndDlg, "ROLLBACK");
		}
		else
		{
			Warning ("SYS_ENC_UPGRADE_FAILED");
		}
	}

outcome:
	OutcomePrompt (hwndDlg, bOK);

	if (bOK && !bUninstall && !bDowngrade && !bRepairMode && !bDevm)
	{
		if (!IsHiddenOSRunning())	// A hidden OS user should not see the post-install notes twice (on decoy OS and then on hidden OS).
		{
			if (bRestartRequired || SystemEncryptionUpdate)
			{
				// Restart required

				if (bUpgrade)
				{
					SavePostInstallTasksSettings (TC_POST_INSTALL_CFG_RELEASE_NOTES);
				}
				else if (bPossiblyFirstTimeInstall)
				{
					SavePostInstallTasksSettings (TC_POST_INSTALL_CFG_TUTORIAL);
				}
			}
			else
			{
				// No restart will be required

				if (bUpgrade)
				{
					bPromptReleaseNotes = TRUE;
				}
				else if (bPossiblyFirstTimeInstall)
				{
					bPromptTutorial = TRUE;
				}
			}
		}
	}

	PostMessage (MainDlg, bOK ? TC_APPMSG_INSTALL_SUCCESS : TC_APPMSG_INSTALL_FAILURE, 0, 0);
}

/**
 * Determines the current installation path and sets the "InstallationPath" global,
 * if no installation path was found it defaults to "Program Files".
 * @param	[in] HWND hwndDlg	Dialog window handle.
 * @return	void
 */
void SetInstallationPath (HWND hwndDlg)
{
	HKEY hkey;
	BOOL bInstallPathDetermined = FALSE;
	char path[MAX_PATH+20];
	ITEMIDLIST *itemList;

	memset (InstallationPath, 0, sizeof (InstallationPath));

	// Determine if CipherShed is already installed and try to determine its "Program Files" location
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TrueCrypt", 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		/* Default 'UninstallString' registry strings written by past versions of CipherShed:
		------------------------------------------------------------------------------------
		1.0		C:\WINDOWS\TrueCrypt Setup.exe /u			[optional]
		1.0a	C:\WINDOWS\TrueCrypt Setup.exe /u			[optional]
		2.0		C:\WINDOWS\TrueCrypt Setup.exe /u			[optional]
		2.1		C:\WINDOWS\TrueCrypt Setup.exe /u			[optional]
		2.1a	C:\WINDOWS\TrueCrypt Setup.exe /u			[optional]
		3.0		C:\WINDOWS\TrueCrypt Setup.exe /u			[optional]
		3.0a	C:\WINDOWS\TrueCrypt Setup.exe /u			[optional]
		3.1		The UninstallString was NEVER written (fortunately, 3.1a replaced 3.1 after 2 weeks)
		3.1a	C:\WINDOWS\TrueCrypt Setup.exe /u
		4.0		C:\WINDOWS\TrueCrypt Setup.exe /u C:\Program Files\TrueCrypt
		4.1		C:\WINDOWS\TrueCrypt Setup.exe /u C:\Program Files\TrueCrypt
		4.2		C:\WINDOWS\TrueCrypt Setup.exe /u C:\Program Files\TrueCrypt
		4.2a	C:\WINDOWS\TrueCrypt Setup.exe /u C:\Program Files\TrueCrypt
		4.3		"C:\Program Files\TrueCrypt\TrueCrypt Setup.exe" /u C:\Program Files\TrueCrypt\
		4.3a	"C:\Program Files\TrueCrypt\TrueCrypt Setup.exe" /u C:\Program Files\TrueCrypt\
		5.0+	"C:\Program Files\TrueCrypt\TrueCrypt Setup.exe" /u

		Note: In versions 1.0-3.0a the user was able to choose whether to install the uninstaller.
			  The default was to install it. If it wasn't installed, there was no UninstallString.
		*/

		char rv[MAX_PATH*4];
		DWORD size = sizeof (rv);
		if (RegQueryValueEx (hkey, "UninstallString", 0, 0, (LPBYTE) &rv, &size) == ERROR_SUCCESS && strrchr (rv, '/'))
		{
			size_t len = 0;

			// Cut and paste the location (path) where CipherShed is installed to InstallationPath
			if (rv[0] == '"')
			{
				// 4.3 or later

				len = strrchr (rv, '/') - rv - 2;
				strncpy (InstallationPath, rv + 1, len);
				InstallationPath [len] = 0;
				bInstallPathDetermined = TRUE;

				if (InstallationPath [strlen (InstallationPath) - 1] != '\\')
				{
					len = strrchr (InstallationPath, '\\') - InstallationPath;
					InstallationPath [len] = 0;
				}
			}
			else
			{
				// 1.0-4.2a (except 3.1)

				len = strrchr (rv, '/') - rv;
				if (rv[len+2] == ' ')
				{
					// 4.0-4.2a

					strncpy (InstallationPath, rv + len + 3, strlen (rv) - len - 3);
					InstallationPath [strlen (rv) - len - 3] = 0;
					bInstallPathDetermined = TRUE;
				}
				else
				{
					// 1.0-3.1a (except 3.1)

					// We know that CipherShed is installed but don't know where. It's not safe to continue installing
					// over the old version.

					Error ("UNINSTALL_OLD_VERSION_FIRST");

					len = strrchr (rv, '/') - rv - 1;
					strncpy (InstallationPath, rv, len);	// Path and filename of the uninstaller
					InstallationPath [len] = 0;
					bInstallPathDetermined = FALSE;

					ShellExecute (NULL, "open", InstallationPath, "/u", NULL, SW_SHOWNORMAL);
					RegCloseKey (hkey);
					exit (1);
				}
			}

		}
		RegCloseKey (hkey);
	}

	if (bInstallPathDetermined)
	{
		char mp[MAX_PATH];

		// Determine whether we were launched from the folder where CipherShed is installed
		GetModuleFileName (NULL, mp, sizeof (mp));
		if (strncmp (InstallationPath, mp, min (strlen(InstallationPath), strlen(mp))) == 0)
		{
			// We were launched from the folder where CipherShed is installed

			if (!IsNonInstallMode() && !bDevm)
				bChangeMode = TRUE;
		}
	}
	else
	{
		/* CipherShed is not installed or it wasn't possible to determine where it is installed. */

		// Default "Program Files" path. 
		SHGetSpecialFolderLocation (hwndDlg, CSIDL_PROGRAM_FILES, &itemList);
		SHGetPathFromIDList (itemList, path);

		if (Is64BitOs())
		{
			// Use a unified default installation path (registry redirection of %ProgramFiles% does not work if the installation path is user-selectable)
			string s = path;
			size_t p = s.find (" (x86)");
			if (p != string::npos)
			{
				s = s.substr (0, p);
				if (_access (s.c_str(), 0) != -1)
					strcpy_s (path, sizeof (path), s.c_str());
			}
		}

		strncat (path, "\\CipherShed\\", min (strlen("\\CipherShed\\"), sizeof(path)-strlen(path)-1));
		strncpy (InstallationPath, path, sizeof(InstallationPath)-1);
	}

	// Make sure the path ends with a backslash
	if (InstallationPath [strlen (InstallationPath) - 1] != '\\')
	{
		strcat (InstallationPath, "\\");
	}

	/*
	 * CipherShed changes the InstallationPath in case of TrueCrypt upgrade,
	 * we need to store the original path for TrueCrypt uninstall.
	 */
	strcpy(UninstallationPath, InstallationPath);
}


/**
 * Handler for uninstall only (install is handled by the wizard).
 * @param	[in] HWND hwndDlg	Dialog window handle.
 * @param	[in] UINT msg		The message.
 * @param	[in] WPARAM wParam	Additional message-specific information.
 * @param	[in] LPARAM lParam	Additional message-specific information.
 * @return	BOOL				TRUE if it processed the message, and FALSE if it did not.
 * @see		http://msdn.microsoft.com/en-us/library/windows/desktop/ms645469%28v=vs.85%29.aspx
 */
BOOL CALLBACK UninstallDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WORD lw = LOWORD (wParam);

	switch (msg)
	{
	case WM_INITDIALOG:

		MainDlg = hwndDlg;

		if (!CreateAppSetupMutex ())
			AbortProcess ("TC_INSTALLER_IS_RUNNING");

		InitDialog (hwndDlg);
		LocalizeDialog (hwndDlg, NULL);

		SetWindowTextW (hwndDlg, lpszTitle);

		// System Restore
		SetCheckBox (hwndDlg, IDC_SYSTEM_RESTORE, bSystemRestore);
		if (SystemRestoreDll == 0)
		{
			/* No System Restore dll available, unset the checkbox and disable it. */
			SetCheckBox (hwndDlg, IDC_SYSTEM_RESTORE, FALSE);
			EnableWindow (GetDlgItem (hwndDlg, IDC_SYSTEM_RESTORE), FALSE);
		}

		SetFocus (GetDlgItem (hwndDlg, IDC_UNINSTALL));

		return 1;

	case WM_SYSCOMMAND:
		if (lw == IDC_ABOUT)
		{
			DialogBoxW (hInst, MAKEINTRESOURCEW (IDD_ABOUT_DLG), hwndDlg, (DLGPROC) AboutDlgProc);
			return 1;
		}
		return 0;

	case WM_COMMAND:
		if (lw == IDC_UNINSTALL)
		{
			if (bDone)
			{
				bUninstallInProgress = FALSE;
				PostMessage (hwndDlg, WM_CLOSE, 0, 0);
				return 1;
			}

			bUninstallInProgress = TRUE;

			WaitCursor ();

			/* Do uninstall. */
			if (bUninstall)
				_beginthread (DoUninstall, 0, (void *) hwndDlg);

			return 1;
		}

		if (lw == IDC_SYSTEM_RESTORE)
		{
			bSystemRestore = IsButtonChecked (GetDlgItem (hwndDlg, IDC_SYSTEM_RESTORE));
			return 1;
		}

		if (lw == IDCANCEL)
		{
			PostMessage (hwndDlg, WM_CLOSE, 0, 0);
			return 1;
		}

		return 0;

	case TC_APPMSG_UNINSTALL_SUCCESS:
		SetWindowTextW (GetDlgItem ((HWND) hwndDlg, IDC_UNINSTALL), GetString ("FINALIZE"));
		NormalCursor ();
		return 1;

	case WM_CLOSE:
		if (bUninstallInProgress)
		{
			NormalCursor();
			if (AskNoYes("CONFIRM_EXIT_UNIVERSAL") == IDNO)
			{
				return 1;
			}
			WaitCursor ();
		}
		EndDialog (hwndDlg, IDCANCEL);
		return 1;
	}

	return 0;
}

/**
 * Setup main.
 */
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpszCommandLine, int nCmdShow)
{
	atexit (localcleanup);

	SelfExtractStartupInit();

	lpszTitle = L"CipherShed Setup";

	InitCommonControls ();

	/* Call InitApp to initialize the common code */
	InitApp (hInstance, NULL);

	/* The setup requires admin privileges, but give the user an option to continue. */
	if (IsAdmin () != TRUE &&
		MessageBoxW (NULL, GetString ("SETUP_ADMIN"), lpszTitle, MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		exit (1);
	}

	/* Setup directory */
	{
		char *s;
		GetModuleFileName (NULL, SetupFilesDir, sizeof (SetupFilesDir));
		s = strrchr (SetupFilesDir, '\\');
		if (s)
			s[1] = 0;
	}

	/* Parse command line arguments */
	if (lpszCommandLine[0] == '/')
	{
		if (lpszCommandLine[1] == 'u')
		{
			// Uninstall:	/u
			bUninstall = TRUE;
		}
		else if (lpszCommandLine[1] == 'c')
		{
			// Change:	/c
			bChangeMode = TRUE;
		}
		else if (lpszCommandLine[1] == 'p')
		{
			// Create self-extracting package:	/p
			bMakePackage = TRUE;

			if ('q' == lpszCommandLine[2] )
			{
				quiet=TRUE;
			}

		}
		else if (lpszCommandLine[1] == 'd')
		{
			// Dev mode:	/d
			bDevm = TRUE;
		}
	}

	if (bMakePackage)
	{
		/* Create self-extracting package */
		MakeSelfExtractingPackage (NULL, SetupFilesDir, quiet);
	}
	else
	{
		SetInstallationPath (NULL);

		if (!bUninstall)
		{
			if (IsSelfExtractingPackage())
			{
				if (!VerifyPackageIntegrity())
				{
					// Package corrupted 
					exit (1);
				}
				bDevm = FALSE;
			}
			else if (!bDevm)
			{
				MessageBox (NULL, "Error: This installer file does not contain any compressed files.\n\nTo create a self-extracting installation package (with embedded compressed files), run:\n\"CipherShed-Setup.exe\" /p", "CipherShed", MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
				exit (1);
			}

			if (bChangeMode)
			{
				/* CipherShed is already installed on this system and we were launched from the Program Files folder */
				char *tmpStr[] = {0, "SELECT_AN_ACTION", "REPAIR_REINSTALL", "UNINSTALL", "EXIT", 0};

				// Ask the user to select either Repair or Unistallation
				switch (AskMultiChoice ((void **) tmpStr, FALSE))
				{
				case 1:
					bRepairMode = TRUE;
					break;
				case 2:
					bUninstall = TRUE;
					break;
				default:
					exit (1);
				}
			}
		}

		/* Try to load the System Restore dll. */
		SystemRestoreDll = LoadLibrary ("srclient.dll");

		if (!bUninstall)
		{
			/* Create the main dialog for install */
			DialogBoxParamW (hInstance, MAKEINTRESOURCEW (IDD_INSTL_DLG), NULL, (DLGPROC) MainDialogProc, 
				(LPARAM)lpszCommandLine);
		}
		else
		{
			/* Create the main dialog for uninstall */
			DialogBoxW (hInstance, MAKEINTRESOURCEW (IDD_UNINSTALL), NULL, (DLGPROC) UninstallDlgProc);

			/* Check if the UninstallBatch path is not empty. */
			if (UninstallBatch[0])
			{
				STARTUPINFO si;
				PROCESS_INFORMATION pi;

				ZeroMemory (&si, sizeof (si));
				si.cb = sizeof (si);
				si.dwFlags = STARTF_USESHOWWINDOW;
				si.wShowWindow = SW_HIDE;

				/* Execute the uninstall batch script. */
				if (!CreateProcess (UninstallBatch, NULL, NULL, NULL, FALSE, IDLE_PRIORITY_CLASS, NULL, NULL, &si, &pi))
				{
					DeleteFile (UninstallBatch);
				}
				else
				{
					CloseHandle (pi.hProcess);
					CloseHandle (pi.hThread);
				}
			}
		}
	}

	return 0;
}
