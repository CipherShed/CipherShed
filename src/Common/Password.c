/*
 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of Encryption for the Masses 2.02a, which is
 Copyright (c) 1998-2000 Paul Le Roux and which is governed by the 'License
 Agreement for Encryption for the Masses'. Modifications and additions to
 the original source code (contained in this file) and all other portions
 of this file are Copyright (c) 2003-2010 TrueCrypt Developers Association
 and are governed by the TrueCrypt License 3.0 the full text of which is
 contained in the file License.txt included in TrueCrypt binary and source
 code distribution packages. */



#include "Crypto.h"

#include "Password.h"
#include "Dlgcode.h"
#include "Language.h"
#include "Pkcs5.h"
#include "Endian.h"
#include "Random.h"

#if defined(_MSC_VER) || defined(__CYGWIN__)
#include <io.h>
#endif
#include "strcpys.h"

#include "util/unicode/ConvertUTF.h"
#include "util/unicode/strcmpw.h"

#ifdef _MSC_VER
__inline
#else
inline 
#endif
int strlenw(WCHAR* s)
{
	int len=0;
	if (!s) return 0;
	while (*s++) ++len;
	return len;
}

BOOL CheckPasswordCharEncoding (HWND hPassword, Password *ptrPw)
{
	int i, len;

	//It would not make sense to have both parameters specified.
	if (hPassword!=NULL && ptrPw!=NULL)
	{
		return FALSE;
	}

	if (hPassword==NULL && ptrPw==NULL)
	{
		return FALSE;
	}

	if (hPassword == NULL)
	{
		unsigned char *pw;
		len = ptrPw->Length;

		if (len>=sizeof(ptrPw->Text))
		{
			return FALSE;
		}

		pw = (unsigned char *) ptrPw->Text;

		for (i = 0; i < len; i++)
		{
			//for i in `seq 32 126`; do printf "\x$(printf %x $i) "; done
			if (pw[i] >= 0x7f || pw[i] < 0x20)	// A non-ASCII or non-printable character?
			{
				return FALSE;
			}
		}
	}
	else
	{
		wchar_t s[MAX_PASSWORD + 1];
		// Below GetWindowTextW is called, ensure the same suffix functionis called 
		// GetWindowTextLength may be defined as GetWindowTextLengthA which would at 
		// best return double the length
		len = GetWindowTextLengthW (hPassword);

		// this is disingenuous here, because the GetWindowTextLength function docs say to use strlen 
		// on the return from GetWindowText because GetWindowTextLength >= strlen
		//
		// This check should happen after GetWindowText
		// len=strlen(s)
		if (len > MAX_PASSWORD)
		{
			return FALSE; 
		}

		GetWindowTextW (hPassword, s, sizeof (s) / sizeof (wchar_t));

		for (i = 0; i < len; i++)
		{
			if (s[i] >= 0x7f || s[i] < 0x20)	// A non-ASCII or non-printable character?
				break;
		}

		burn (s, sizeof(s));

		if (i < len)
		{
			return FALSE; 
		}
	}

	return TRUE;
}


void VerifyPasswordAndUpdate2 (HWND hwndDlg, HWND hButton, HWND hPassword,
			 HWND hVerify, unsigned char *szPassword,
			 int sizeOfPassword,
			 char *szVerify,
			 int sizeOfVerify,
			 BOOL keyFilesEnabled)
{
	WCHAR szTmp1[MAX_PASSWORD + 1];
	WCHAR szTmp2[MAX_PASSWORD + 1];
	int k = GetWindowTextLengthW(hPassword);
	BOOL bEnable = FALSE;
	int len;
	int r;

	if (hwndDlg);		/* Remove warning */

	GetWindowTextW(hPassword, szTmp1, sizeof (szTmp1)/sizeof(*szTmp1));
	GetWindowTextW(hVerify, szTmp2, sizeof (szTmp2)/sizeof(*szTmp2));

	if (strcmpw (szTmp1, szTmp2) != 0)
		bEnable = FALSE;
	else
	{
		if (k >= MIN_PASSWORD || keyFilesEnabled)
			bEnable = TRUE;
		else
			bEnable = FALSE;
	}

	if (szPassword != NULL)
	{
		len=strlenw(szTmp1);
		r=ConvertUTF16toUTF8s(szTmp1,len+1,szPassword,sizeOfPassword,strictConversion);
		if (r!=conversionOK) bEnable=FALSE;
	}

	if (szVerify != NULL)
	{
		len=strlenw(szTmp2);
		r=ConvertUTF16toUTF8s(szTmp2,len+1,(UTF8*)szVerify,sizeOfVerify,strictConversion);
		if (r!=conversionOK) bEnable=FALSE;
	}

	//clean up memory which contains passwords or metadata (e.g. length)
	burn (szTmp1, sizeof (szTmp1));
	burn (szTmp2, sizeof (szTmp2));
	k=0;
	len=0;

	EnableWindow (hButton, bEnable);
}


BOOL CheckPasswordLength (HWND hwndDlg, HWND hwndItem)
{
	return CheckPasswordLengthAlertTitle(hwndDlg, lpszTitle, hwndItem);
}

BOOL CheckPasswordLengthAlertTitle (HWND hwndDlg, wchar_t* title, HWND hwndItem)
{
	if (hwndDlg==NULL)
	{
		return FALSE;
	}
	
	if (hwndItem==NULL)
	{
		return FALSE;
	}

	// Again, this is not what this function is for, use strlen on GetWindowText, it 
	// can return a size twice the actual size of the string.
	if (GetWindowTextLengthW (hwndItem) < PASSWORD_LEN_WARNING)
	{
//DEBUG builds do not have this dialog box prompt, and the function always returns true.
#if !defined(_DEBUG) || defined(CS_UNITTESTING)
		// The MessageBoxW function is taking the title bar text from a global var "lpszTitle" in Dlgcode.c
		if (MessageBoxW (hwndDlg, GetString ("PASSWORD_LENGTH_WARNING"), title, MB_YESNO|MB_ICONWARNING|MB_DEFBUTTON2) != IDYES)
		{
			return FALSE;
		}
#endif
	}
	return TRUE;
}

int ChangePwd (char *lpszVolume, Password *oldPassword, Password *newPassword, int pkcs5, HWND hwndDlg)
{
	int nDosLinkCreated = 1, nStatus = ERR_OS_ERROR;
	char szDiskFile[TC_MAX_PATH], szCFDevice[TC_MAX_PATH];
	char szDosDevice[TC_MAX_PATH];
	char buffer[TC_VOLUME_HEADER_EFFECTIVE_SIZE];
	PCRYPTO_INFO cryptoInfo = NULL, ci = NULL;
	void *dev = INVALID_HANDLE_VALUE;
	DWORD dwError;
	DWORD bytesRead;
	BOOL bDevice;
	unsigned __int64 hostSize = 0;
	int volumeType;
	int wipePass;
	FILETIME ftCreationTime;
	FILETIME ftLastWriteTime;
	FILETIME ftLastAccessTime;
	BOOL bTimeStampValid = FALSE;
	LARGE_INTEGER headerOffset;
	BOOL backupHeader;
	DISK_GEOMETRY driveInfo;

	if (oldPassword->Length == 0 || newPassword->Length == 0) return -1;

	WaitCursor ();

	CreateFullVolumePath (szDiskFile, lpszVolume, &bDevice);

	if (bDevice == FALSE)
	{
		strcpy_s (szCFDevice, sizeof(szCFDevice), szDiskFile);
	}
	else
	{
		nDosLinkCreated = FakeDosNameForDevice (szDiskFile, szDosDevice, szCFDevice, FALSE);
		
		if (nDosLinkCreated != 0)
			goto error;
	}

	dev = CreateFile (szCFDevice, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (dev == INVALID_HANDLE_VALUE) 
		goto error;

	if (bDevice)
	{
		/* This is necessary to determine the hidden volume header offset */

		if (dev == INVALID_HANDLE_VALUE)
		{
			goto error;
		}
		else
		{
			PARTITION_INFORMATION diskInfo;
			DWORD dwResult;
			BOOL bResult;

			bResult = DeviceIoControl (dev, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0,
				&driveInfo, sizeof (driveInfo), &dwResult, NULL);

			if (!bResult)
				goto error;

			bResult = GetPartitionInfo (lpszVolume, &diskInfo);

			if (bResult)
			{
				hostSize = diskInfo.PartitionLength.QuadPart;
			}
			else
			{
				hostSize = driveInfo.Cylinders.QuadPart * driveInfo.BytesPerSector *
					driveInfo.SectorsPerTrack * driveInfo.TracksPerCylinder;
			}

			if (hostSize == 0)
			{
				nStatus = ERR_VOL_SIZE_WRONG;
				goto error;
			}
		}
	}
	else
	{
		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx (dev, &fileSize))
		{
			nStatus = ERR_OS_ERROR;
			goto error;
		}

		hostSize = fileSize.QuadPart;
	}

	if (Randinit ())
		goto error;

	if (!bDevice && bPreserveTimestamp)
	{
		if (GetFileTime ((HANDLE) dev, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime) == 0)
			bTimeStampValid = FALSE;
		else
			bTimeStampValid = TRUE;
	}

	for (volumeType = TC_VOLUME_TYPE_NORMAL; volumeType < TC_VOLUME_TYPE_COUNT; volumeType++)
	{
		// Seek the volume header
		switch (volumeType)
		{
		case TC_VOLUME_TYPE_NORMAL:
			headerOffset.QuadPart = TC_VOLUME_HEADER_OFFSET;
			break;

		case TC_VOLUME_TYPE_HIDDEN:
			if (TC_HIDDEN_VOLUME_HEADER_OFFSET + TC_VOLUME_HEADER_SIZE > hostSize)
				continue;

			headerOffset.QuadPart = TC_HIDDEN_VOLUME_HEADER_OFFSET;
			break;

		case TC_VOLUME_TYPE_HIDDEN_LEGACY:
			if (bDevice && driveInfo.BytesPerSector != TC_SECTOR_SIZE_LEGACY)
				continue;

			headerOffset.QuadPart = hostSize - TC_HIDDEN_VOLUME_HEADER_OFFSET_LEGACY;
			break;
		}

		if (!SetFilePointerEx ((HANDLE) dev, headerOffset, NULL, FILE_BEGIN))
		{
			nStatus = ERR_OS_ERROR;
			goto error;
		}

		/* Read in volume header */
		if (!ReadEffectiveVolumeHeader (bDevice, dev, (byte*)buffer, &bytesRead))
		{
			nStatus = ERR_OS_ERROR;
			goto error;
		}

		if (bytesRead != sizeof (buffer))
		{
			// Windows may report EOF when reading sectors from the last cluster of a device formatted as NTFS 
			memset (buffer, 0, sizeof (buffer));
		}

		/* Try to decrypt the header */

		nStatus = ReadVolumeHeader (FALSE, buffer, oldPassword, &cryptoInfo, NULL);
		if (nStatus == ERR_CIPHER_INIT_WEAK_KEY)
			nStatus = 0;	// We can ignore this error here

		if (nStatus == ERR_PASSWORD_WRONG)
		{
			continue;		// Try next volume type
		}
		else if (nStatus != 0)
		{
			cryptoInfo = NULL;
			goto error;
		}
		else 
			break;
	}

	if (nStatus != 0)
	{
		cryptoInfo = NULL;
		goto error;
	}

	if (cryptoInfo->HeaderFlags & TC_HEADER_FLAG_ENCRYPTED_SYSTEM)
	{
		nStatus = ERR_SYS_HIDVOL_HEAD_REENC_MODE_WRONG;
		goto error;
	}

	// Change the PKCS-5 PRF if requested by user
	if (pkcs5 != 0)
		cryptoInfo->pkcs5 = pkcs5;

	RandSetHashFunction (cryptoInfo->pkcs5);

	NormalCursor();
	UserEnrichRandomPool (hwndDlg);
	EnableElevatedCursorChange (hwndDlg);
	WaitCursor();

	/* Re-encrypt the volume header */ 
	backupHeader = FALSE;

	while (TRUE)
	{
		/* The header will be re-encrypted PRAND_DISK_WIPE_PASSES times to prevent adversaries from using 
		techniques such as magnetic force microscopy or magnetic force scanning tunnelling microscopy
		to recover the overwritten header. According to Peter Gutmann, data should be overwritten 22
		times (ideally, 35 times) using non-random patterns and pseudorandom data. However, as users might
		impatiently interupt the process (etc.) we will not use the Gutmann's patterns but will write the
		valid re-encrypted header, i.e. pseudorandom data, and there will be many more passes than Guttman
		recommends. During each pass we will write a valid working header. Each pass will use the same master
		key, and also the same header key, secondary key (XTS), etc., derived from the new password. The only
		item that will be different for each pass will be the salt. This is sufficient to cause each "version"
		of the header to differ substantially and in a random manner from the versions written during the
		other passes. */

		for (wipePass = 0; wipePass < PRAND_DISK_WIPE_PASSES; wipePass++)
		{
			// Prepare new volume header
			nStatus = CreateVolumeHeaderInMemory (FALSE,
				buffer,
				cryptoInfo->ea,
				cryptoInfo->mode,
				newPassword,
				cryptoInfo->pkcs5,
				(char*)cryptoInfo->master_keydata, //unsigned __int8
				&ci,
				cryptoInfo->VolumeSize.Value,
				(volumeType == TC_VOLUME_TYPE_HIDDEN || volumeType == TC_VOLUME_TYPE_HIDDEN_LEGACY) ? cryptoInfo->hiddenVolumeSize : 0,
				cryptoInfo->EncryptedAreaStart.Value,
				cryptoInfo->EncryptedAreaLength.Value,
				cryptoInfo->RequiredProgramVersion,
				cryptoInfo->HeaderFlags,
				cryptoInfo->SectorSize,
				wipePass < PRAND_DISK_WIPE_PASSES - 1);

			if (ci != NULL)
				crypto_close (ci);

			if (nStatus != 0)
				goto error;

			if (!SetFilePointerEx ((HANDLE) dev, headerOffset, NULL, FILE_BEGIN))
			{
				nStatus = ERR_OS_ERROR;
				goto error;
			}

			if (!WriteEffectiveVolumeHeader (bDevice, dev, (byte*)buffer))
			{
				nStatus = ERR_OS_ERROR;
				goto error;
			}

			if (bDevice
				&& !cryptoInfo->LegacyVolume
				&& !cryptoInfo->hiddenVolume
				&& cryptoInfo->HeaderVersion == 4
				&& (cryptoInfo->HeaderFlags & TC_HEADER_FLAG_NONSYS_INPLACE_ENC) != 0
				&& (cryptoInfo->HeaderFlags & ~TC_HEADER_FLAG_NONSYS_INPLACE_ENC) == 0)
			{
				nStatus = WriteRandomDataToReservedHeaderAreas (dev, cryptoInfo, cryptoInfo->VolumeSize.Value, !backupHeader, backupHeader);
				if (nStatus != ERR_SUCCESS)
					goto error;
			}

			FlushFileBuffers (dev);
		}

		if (backupHeader || cryptoInfo->LegacyVolume)
			break;
			
		backupHeader = TRUE;
		headerOffset.QuadPart += hostSize - TC_VOLUME_HEADER_GROUP_SIZE;
	}

	/* Password successfully changed */
	nStatus = 0;

error:
	dwError = GetLastError ();

	burn (buffer, sizeof (buffer));

	if (cryptoInfo != NULL)
		crypto_close (cryptoInfo);

	if (bTimeStampValid)
		SetFileTime (dev, &ftCreationTime, &ftLastAccessTime, &ftLastWriteTime);

	if (dev != INVALID_HANDLE_VALUE)
		CloseHandle ((HANDLE) dev);

	if (nDosLinkCreated == 0)
		RemoveFakeDosName (szDiskFile, szDosDevice);

	RandStop (FALSE);
	NormalCursor ();

	SetLastError (dwError);

	if (nStatus == ERR_OS_ERROR && dwError == ERROR_ACCESS_DENIED
		&& bDevice
		&& !UacElevated
		&& IsUacSupported ())
		return nStatus;

	if (nStatus != 0)
		handleError (hwndDlg, nStatus);

	return nStatus;
}

