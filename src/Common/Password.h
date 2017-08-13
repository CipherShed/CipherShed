/*
 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of Encryption for the Masses 2.02a, which is
 Copyright (c) 1998-2000 Paul Le Roux and which is governed by the 'License
 Agreement for Encryption for the Masses'. Modifications and additions to
 the original source code (contained in this file) and all other portions
 of this file are Copyright (c) 2003-2008 TrueCrypt Developers Association
 and are governed by the TrueCrypt License 3.0 the full text of which is
 contained in the file License.txt included in TrueCrypt binary and source
 code distribution packages. */

#ifndef PASSWORD_H
#define PASSWORD_H

#include "constants.h"
#include "types/password.h"

#ifdef CS_UNITTESTING
#include "../unit-tests/faux/windows/EnableWindow.h"
#include "../unit-tests/faux/windows/GetWindowText.h"
#include "../unit-tests/faux/windows/GetWindowTextLength.h"
#include "../unit-tests/faux/windows/ACCESSTYPES.h"
#include "../unit-tests/faux/windows/IOCTL_DISK.h"
#include "../unit-tests/faux/windows/ERROR.h"
#include "../unit-tests/faux/windows/CreateFile.h"
#include "../unit-tests/faux/windows/DeviceIoControl.h"
#include "../unit-tests/faux/windows/GetFileSize.h"
#include "../unit-tests/faux/windows/SetFilePointer.h"
#include "../unit-tests/faux/windows/FlushFileBuffers.h"
#include "../unit-tests/faux/windows/GetFileTime.h"
#include "../unit-tests/faux/windows/GetLastError.h"
#include "../unit-tests/faux/windows/SetLastError.h"
#include "../unit-tests/faux/windows/SetFileTime.h"
#include "../unit-tests/faux/windows/CloseHandle.h"
#include "../unit-tests/faux/windows/MessageBox.h"
#undef __nullterminated
#include <string.h>
#define MB_YESNO                    0x00000004L
#define MB_ICONWARNING              MB_ICONEXCLAMATION
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_DEFBUTTON2               0x00000100L
#define IDYES               6
#ifdef __cplusplus
extern "C" 
{
#endif
int strlenw(WCHAR* s);
#ifdef __cplusplus
}
#endif
#endif

// User text input limits
#define MIN_PASSWORD			1		// Minimum possible password length

#define PASSWORD_LEN_WARNING	20		// Display a warning when a password is shorter than this

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && !defined(TC_WINDOWS_DRIVER) || defined(CS_UNITTESTING)

void VerifyPasswordAndUpdate2(HWND hwndDlg, HWND hButton, HWND hPassword, HWND hVerify, unsigned char *szPassword, int sizeOfPassword, char *szVerify, int sizeOfVerify, BOOL keyFilesEnabled);
BOOL CheckPasswordLength (HWND hwndDlg, HWND hwndItem);		
BOOL CheckPasswordCharEncoding (HWND hPassword, Password *ptrPw);			
int ChangePwd (char *lpszVolume, Password *oldPassword, Password *newPassword, int pkcs5, HWND hwndDlg);
BOOL CheckPasswordLengthAlertTitle (HWND hwndDlg, wchar_t* title, HWND hwndItem);

#endif	// defined(_WIN32) && !defined(TC_WINDOWS_DRIVER)

#ifdef __cplusplus
}
#endif

#endif	// PASSWORD_H
