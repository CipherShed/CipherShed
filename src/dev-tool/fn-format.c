/*
 Portions of this file are
 Licensed to the The CipherShed Project (CP) under one
 or more contributor license agreements.  See the NOTICE file
 distributed with this work for additional information
 regarding copyright ownership.  The CP licenses this file
 to you under the Apache License, Version 2.0 (the
 "License"); you may not use this file except in compliance
 with the License.  You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing,
 software distributed under the License is distributed on an
 "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 KIND, either express or implied.  See the License for the
 specific language governing permissions and limitations
 under the License.

 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of Encryption for the Masses 2.02a, which is
 Copyright (c) 1998-2000 Paul Le Roux and which is governed by the 'License
 Agreement for Encryption for the Masses'. Modifications and additions to
 the original source code (contained in this file) and all other portions
 of this file are Copyright (c) 2003-2010 TrueCrypt Developers Association
 and are governed by the TrueCrypt License 3.0 the full text of which is
 contained in the file License.txt included in TrueCrypt binary and source
 code distribution packages. */

#include "../Common/Tcdefs.h"

#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <errno.h>
#include <io.h>
#include <sys/stat.h>
#include <shlobj.h>

#include "../Common/Crypto.h"
#include "../Common/Apidrvr.h"
#include "../Common/Dlgcode.h"
#include "../Common/Language.h"
#include "../Common/Combo.h"
#include "../Common/Registry.h"
#include "../Boot/Windows/BootDefs.h"
#include "../Common/Common.h"
#include "../Common/BootEncryption.h"
#include "../Common/Dictionary.h"
#include "../Common/Endian.h"
#include "../Common/resource.h"
#include "../Platform/Finally.h"
#include "../Platform/ForEach.h"
#include "../Common/Random.h"
#include "../Common/Fat.h"
#include "../Format/InPlace.h"
#include "../Format/Resource.h"
#include "../Format/TcFormat.h"
#include "../Common/Format.h"
#include "../Format/FormatCom.h"
#include "../Common/Password.h"
#include "../Common/Progress.h"
#include "../Common/Tests.h"
#include "../Common/Cmdline.h"
#include "../Common/Volumes.h"
#include "../Common/Wipe.h"
#include "../Common/Xml.h"
#include "../Common/EncryptionThreadPool.h"

#include "../Common/util/unicode/ConvertUTF.h"

using namespace CipherShed;
//using namespace System; // (fn)
// COPIED CODE \/ from: src/Format/Tcformat.c

template <typename T,unsigned S>
inline unsigned arraysize(const T (&v)[S]) { return S; }

enum wizard_pages
{
	/* IMPORTANT: IF YOU ADD/REMOVE/MOVE ANY PAGES THAT ARE RELATED TO SYSTEM ENCRYPTION,
	REVISE THE 'DECOY_OS_INSTRUCTIONS_PORTION_??' STRINGS! */

	INTRO_PAGE,
			SYSENC_TYPE_PAGE,
					SYSENC_HIDDEN_OS_REQ_CHECK_PAGE,
			SYSENC_SPAN_PAGE,
			SYSENC_PRE_DRIVE_ANALYSIS_PAGE,
			SYSENC_DRIVE_ANALYSIS_PAGE,
			SYSENC_MULTI_BOOT_MODE_PAGE,
			SYSENC_MULTI_BOOT_SYS_EQ_BOOT_PAGE,
			SYSENC_MULTI_BOOT_NBR_SYS_DRIVES_PAGE,
			SYSENC_MULTI_BOOT_ADJACENT_SYS_PAGE,
			SYSENC_MULTI_BOOT_NONWIN_BOOT_LOADER_PAGE,
			SYSENC_MULTI_BOOT_OUTCOME_PAGE,
	VOLUME_TYPE_PAGE,
				HIDDEN_VOL_WIZARD_MODE_PAGE,
	VOLUME_LOCATION_PAGE,
		DEVICE_TRANSFORM_MODE_PAGE,
				HIDDEN_VOL_HOST_PRE_CIPHER_PAGE,
				HIDDEN_VOL_PRE_CIPHER_PAGE,
	CIPHER_PAGE,
	SIZE_PAGE,
				HIDDEN_VOL_HOST_PASSWORD_PAGE,
	PASSWORD_PAGE,
		FILESYS_PAGE,
			SYSENC_COLLECTING_RANDOM_DATA_PAGE,
			SYSENC_KEYS_GEN_PAGE,
			SYSENC_RESCUE_DISK_CREATION_PAGE,
			SYSENC_RESCUE_DISK_BURN_PAGE,
			SYSENC_RESCUE_DISK_VERIFIED_PAGE,
			SYSENC_WIPE_MODE_PAGE,
			SYSENC_PRETEST_INFO_PAGE,
			SYSENC_PRETEST_RESULT_PAGE,
			SYSENC_ENCRYPTION_PAGE,
		NONSYS_INPLACE_ENC_RESUME_PASSWORD_PAGE,
		NONSYS_INPLACE_ENC_RESUME_PARTITION_SEL_PAGE,
		NONSYS_INPLACE_ENC_RAND_DATA_PAGE,
		NONSYS_INPLACE_ENC_WIPE_MODE_PAGE,
		NONSYS_INPLACE_ENC_ENCRYPTION_PAGE,
		NONSYS_INPLACE_ENC_ENCRYPTION_FINISHED_PAGE,
	FORMAT_PAGE,
	FORMAT_FINISHED_PAGE,
					SYSENC_HIDDEN_OS_INITIAL_INFO_PAGE,
					SYSENC_HIDDEN_OS_WIPE_INFO_PAGE,
						DEVICE_WIPE_MODE_PAGE,
						DEVICE_WIPE_PAGE
};

#define TIMER_INTERVAL_RANDVIEW							30
#define TIMER_INTERVAL_SYSENC_PROGRESS					30
#define TIMER_INTERVAL_NONSYS_INPLACE_ENC_PROGRESS		30
#define TIMER_INTERVAL_SYSENC_DRIVE_ANALYSIS_PROGRESS	100
#define TIMER_INTERVAL_WIPE_PROGRESS					30
#define TIMER_INTERVAL_KEYB_LAYOUT_GUARD				10

enum sys_encryption_cmd_line_switches
{
	SYSENC_COMMAND_NONE = 0,
	SYSENC_COMMAND_RESUME,
	SYSENC_COMMAND_STARTUP_SEQ_RESUME,
	SYSENC_COMMAND_ENCRYPT,
	SYSENC_COMMAND_DECRYPT,
	SYSENC_COMMAND_CREATE_HIDDEN_OS,
	SYSENC_COMMAND_CREATE_HIDDEN_OS_ELEV
};

typedef struct 
{
	int NumberOfSysDrives;			// Number of drives that contain an operating system. -1: unknown, 1: one, 2: two or more
	int MultipleSystemsOnDrive;		// Multiple systems are installed on the drive where the currently running system resides.  -1: unknown, 0: no, 1: yes
	int BootLoaderLocation;			// Boot loader (boot manager) installed in: 1: MBR/1st cylinder, 0: partition/bootsector: -1: unknown
	int BootLoaderBrand;			// -1: unknown, 0: Microsoft Windows, 1: any non-Windows boot manager/loader
	int SystemOnBootDrive;			// If the currently running operating system is installed on the boot drive. -1: unknown, 0: no, 1: yes
} SYSENC_MULTIBOOT_CFG;

#define SYSENC_PAUSE_RETRY_INTERVAL		100
#define SYSENC_PAUSE_RETRIES			200

// Expected duration of system drive analysis, in ms 
#define SYSENC_DRIVE_ANALYSIS_ETA		(4*60000)

BootEncryption			*BootEncObj = NULL;
BootEncryptionStatus	BootEncStatus;

HWND hCurPage = NULL;		/* Handle to current wizard page */
int nCurPageNo = -1;		/* The current wizard page */
int nLastPageNo = -1;
volatile int WizardMode = DEFAULT_VOL_CREATION_WIZARD_MODE; /* IMPORTANT: Never change this value directly -- always use ChangeWizardMode() instead. */
volatile BOOL bHiddenOS = FALSE;		/* If TRUE, we are performing or (or supposed to perform) actions relating to an operating system installed in a hidden volume (i.e., encrypting a decoy OS partition or creating the outer/hidden volume for the hidden OS). To determine or set the phase of the process, call ChangeHiddenOSCreationPhase() and DetermineHiddenOSCreationPhase()) */
BOOL bDirectSysEncMode = FALSE;
BOOL bDirectSysEncModeCommand = SYSENC_COMMAND_NONE;
BOOL DirectDeviceEncMode = FALSE;
BOOL DirectNonSysInplaceEncResumeMode = FALSE;
BOOL DirectPromptNonSysInplaceEncResumeMode = FALSE;
volatile BOOL bInPlaceEncNonSys = FALSE;		/* If TRUE, existing data on a non-system partition/volume are to be encrypted (for system encryption, this flag is ignored) */
volatile BOOL bInPlaceEncNonSysResumed = FALSE;	/* If TRUE, the wizard is supposed to resume (or has resumed) process of non-system in-place encryption. */
volatile BOOL bFirstNonSysInPlaceEncResumeDone = FALSE;
__int64 NonSysInplaceEncBytesDone = 0;
__int64 NonSysInplaceEncTotalSize = 0;
BOOL bDeviceTransformModeChoiceMade = FALSE;		/* TRUE if the user has at least once manually selected the 'in-place' or 'format' option (on the 'device transform mode' page). */
int nNeedToStoreFilesOver4GB = 0;		/* Whether the user wants to be able to store files larger than 4GB on the volume: -1 = Undecided or error, 0 = No, 1 = Yes */
int nVolumeEA = 1;			/* Default encryption algorithm */
BOOL bSystemEncryptionInProgress = FALSE;		/* TRUE when encrypting/decrypting the system partition/drive (FALSE when paused). */
BOOL bWholeSysDrive = FALSE;	/* Whether to encrypt the entire system drive or just the system partition. */
static BOOL bSystemEncryptionStatusChanged = FALSE;   /* TRUE if this instance changed the value of SystemEncryptionStatus (it's set to FALSE each time the system encryption settings are saved to the config file). This value is to be treated as protected -- only the wizard can change this value (others may only read it). */
volatile BOOL bSysEncDriveAnalysisInProgress = FALSE;
volatile BOOL bSysEncDriveAnalysisTimeOutOccurred = FALSE;
int SysEncDetectHiddenSectors = -1;		/* Whether the user wants us to detect and encrypt the Host Protect Area (if any): -1 = Undecided or error, 0 = No, 1 = Yes */
int SysEncDriveAnalysisStart;
BOOL bDontVerifyRescueDisk = FALSE;
BOOL bFirstSysEncResumeDone = FALSE;
int nMultiBoot = 0;			/* The number of operating systems installed on the computer, according to the user. 0: undetermined, 1: one, 2: two or more */
volatile BOOL bHiddenVol = FALSE;	/* If true, we are (or will be) creating a hidden volume. */
volatile BOOL bHiddenVolHost = FALSE;	/* If true, we are (or will be) creating the host volume (called "outer") for a hidden volume. */
volatile BOOL bHiddenVolDirect = FALSE;	/* If true, the wizard omits creating a host volume in the course of the process of hidden volume creation. */
volatile BOOL bHiddenVolFinished = FALSE;
int hiddenVolHostDriveNo = -1;	/* Drive letter for the volume intended to host a hidden volume. */
BOOL bRemovableHostDevice = FALSE;	/* TRUE when creating a device/partition-hosted volume on a removable device. State undefined when creating file-hosted volumes. */
int realClusterSize;		/* Parameter used when determining the maximum possible size of a hidden volume. */
int hash_algo = DEFAULT_HASH_ALGORITHM;	/* Which PRF to use in header key derivation (PKCS #5) and in the RNG. */
unsigned __int64 nUIVolumeSize = 0;		/* The volume size. Important: This value is not in bytes. It has to be multiplied by nMultiplier. Do not use this value when actually creating the volume (it may chop off sector size, if it is not a multiple of 1024 bytes). */
unsigned __int64 nVolumeSize = 0;		/* The volume size, in bytes. */
unsigned __int64 nHiddenVolHostSize = 0;	/* Size of the hidden volume host, in bytes */
__int64 nMaximumHiddenVolSize = 0;		/* Maximum possible size of the hidden volume, in bytes */
__int64 nbrFreeClusters = 0;
int nMultiplier = BYTES_PER_MB;		/* Size selection multiplier. */
char szFileName[TC_MAX_PATH+1];	/* The file selected by the user */
char szDiskFile[TC_MAX_PATH+1];	/* Fully qualified name derived from szFileName */
char szRescueDiskISO[TC_MAX_PATH+1];	/* The filename and path to the Rescue Disk ISO file to be burned (for boot encryption) */
BOOL bDeviceWipeInProgress = FALSE;
volatile BOOL bTryToCorrectReadErrors = FALSE;
volatile BOOL DiscardUnreadableEncryptedSectors = FALSE;

volatile BOOL bVolTransformThreadCancel = FALSE;	/* TRUE if the user cancels/pauses volume encryption/format */
volatile BOOL bVolTransformThreadRunning = FALSE;	/* Is the volume encryption/format thread running */
volatile BOOL bVolTransformThreadToRun = FALSE;		/* TRUE if the Format/Encrypt button has been clicked and we are proceeding towards launching the thread. */

volatile BOOL bConfirmQuit = FALSE;		/* If TRUE, the user is asked to confirm exit when he clicks the X icon, Exit, etc. */
volatile BOOL bConfirmQuitSysEncPretest = FALSE;

BOOL bDevice = FALSE;		/* Is this a partition volume ? */

BOOL showKeys = TRUE;
volatile HWND hMasterKey = NULL;		/* Text box showing hex dump of the master key */
volatile HWND hHeaderKey = NULL;		/* Text box showing hex dump of the header key */
volatile HWND hRandPool = NULL;		/* Text box showing hex dump of the random pool */
volatile HWND hRandPoolSys = NULL;	/* Text box showing hex dump of the random pool for system encryption */
volatile HWND hPasswordInputField = NULL;	/* Password input field */
volatile HWND hVerifyPasswordInputField = NULL;		/* Verify-password input field */

HBITMAP hbmWizardBitmapRescaled = NULL;

char OrigKeyboardLayout [8+1] = "00000409";
BOOL bKeyboardLayoutChanged = FALSE;		/* TRUE if the keyboard layout was changed to the standard US keyboard layout (from any other layout). */ 
BOOL bKeybLayoutAltKeyWarningShown = FALSE;	/* TRUE if the user has been informed that it is not possible to type characters by pressing keys while the right Alt key is held down. */ 

#ifndef _DEBUG
	BOOL bWarnDeviceFormatAdvanced = TRUE;
#else
	BOOL bWarnDeviceFormatAdvanced = FALSE;
#endif

BOOL bWarnOuterVolSuitableFileSys = TRUE;

Password volumePassword;			/* User password */
char szTmpPas[MAX_PASSWORD + 1];	/* Tmp password buffer */
WCHAR szVerify[MAX_PASSWORD + 1];	
WCHAR szRawPassword[MAX_PASSWORD + 1];	/* Password before keyfile was applied to it */

BOOL bHistoryCmdLine = FALSE; /* History control is always disabled */
BOOL ComServerMode = FALSE;

int nPbar = 0;			/* Control ID of progress bar:- for format code */

char HeaderKeyGUIView [KEY_GUI_VIEW_SIZE];
char MasterKeyGUIView [KEY_GUI_VIEW_SIZE];

#define RANDPOOL_DISPLAY_COLUMNS	15
#define RANDPOOL_DISPLAY_ROWS		8
#define RANDPOOL_DISPLAY_BYTE_PORTION	(RANDPOOL_DISPLAY_COLUMNS * RANDPOOL_DISPLAY_ROWS)
#define RANDPOOL_DISPLAY_SIZE	(RANDPOOL_DISPLAY_BYTE_PORTION * 3 + RANDPOOL_DISPLAY_ROWS + 2)
unsigned char randPool [RANDPOOL_DISPLAY_BYTE_PORTION];
unsigned char lastRandPool [RANDPOOL_DISPLAY_BYTE_PORTION];
unsigned char outRandPoolDispBuffer [RANDPOOL_DISPLAY_SIZE];
BOOL bDisplayPoolContents = TRUE;

volatile BOOL bSparseFileSwitch = FALSE;
volatile BOOL quickFormat = FALSE;	/* WARNING: Meaning of this variable depends on bSparseFileSwitch. If bSparseFileSwitch is TRUE, this variable represents the sparse file flag. */
volatile int fileSystem = FILESYS_NONE;	
volatile int clusterSize = 0;

SYSENC_MULTIBOOT_CFG	SysEncMultiBootCfg;
wchar_t SysEncMultiBootCfgOutcome [4096] = {'N','/','A',0};
volatile int NonSysInplaceEncStatus = NONSYS_INPLACE_ENC_STATUS_NONE;

vector <HostDevice> DeferredNonSysInPlaceEncDevices;

// COPIED CODE /\ from: src/Format/Tcformat.c

////////////////////////////////////////
static unsigned int DetermineHiddenOSCreationPhase(void);
////////////////////////////////////////
static void LoadPage(HWND hwndDlg, int nPageNo) {};
BOOL WipeHiddenOSCreationConfig(void) { return FALSE; };
int ScanVolClusterBitmap(HWND hwndDlg, int *driveNo, __int64 nbrClusters, __int64 *nbrFreeClusters) { return 0; };


static int GetFormatSectorSize()
{
	if (!bDevice)
		return TC_SECTOR_SIZE_FILE_HOSTED_VOLUME;

	DISK_GEOMETRY geometry;

	if (!GetDriveGeometry(szDiskFile, &geometry))
	{
		handleWin32Error(MainDlg);
		AbortProcessSilent();
	}

	return geometry.BytesPerSector;
}

// COPIED CODE \/ from: src/Format/Tcformat.c

static BOOL CALLBACK BroadcastSysEncCfgUpdateCallb(HWND hwnd, LPARAM lParam)
{
	/* Modifying 'TRUE' can introduce incompatibility with previous versions. */
	if (GetWindowLongPtr(hwnd, GWLP_USERDATA) == (LONG_PTR) 'TRUE')
	{
		char name[1024] = { 0 };
		GetWindowTextA(hwnd, name, sizeof(name) - 1);
		if (hwnd != MainDlg && strstr(name, "CipherShed"))
		{
			PostMessage(hwnd, TC_APPMSG_SYSENC_CONFIG_UPDATE, 0, 0);
		}
	}
	return TRUE;
}

static BOOL BroadcastSysEncCfgUpdate(void)
{
	BOOL bSuccess = FALSE;
	EnumWindows(BroadcastSysEncCfgUpdateCallb, (LPARAM)&bSuccess);
	return bSuccess;
}

// IMPORTANT: This function may be called only by Format (other modules can only _read_ the system encryption config).
// Returns TRUE if successful (otherwise FALSE)
static BOOL SaveSysEncSettings(HWND hwndDlg)
{
	FILE *f;

	if (!bSystemEncryptionStatusChanged)
		return TRUE;

	if (hwndDlg == NULL && MainDlg != NULL)
		hwndDlg = MainDlg;

	if (!CreateSysEncMutex())
		return FALSE;		// Only one instance that has the mutex can modify the system encryption settings

	if (SystemEncryptionStatus == SYSENC_STATUS_NONE)
	{
		if (remove(GetConfigPath(TC_APPD_FILENAME_SYSTEM_ENCRYPTION)) != 0)
		{
			Error("CANNOT_SAVE_SYS_ENCRYPTION_SETTINGS");
			return FALSE;
		}

		bSystemEncryptionStatusChanged = FALSE;
		BroadcastSysEncCfgUpdate();
		return TRUE;
	}

	f = fopen(GetConfigPath(TC_APPD_FILENAME_SYSTEM_ENCRYPTION), "w");
	if (f == NULL)
	{
		Error("CANNOT_SAVE_SYS_ENCRYPTION_SETTINGS");
		handleWin32Error(hwndDlg);
		return FALSE;
	}

	if (XmlWriteHeader(f) < 0

		|| fputs("\n\t<sysencryption>", f) < 0

		|| fprintf(f, "\n\t\t<config key=\"SystemEncryptionStatus\">%d</config>", SystemEncryptionStatus) < 0

		|| fprintf(f, "\n\t\t<config key=\"WipeMode\">%d</config>", (int)nWipeMode) < 0

		|| fputs("\n\t</sysencryption>", f) < 0

		|| XmlWriteFooter(f) < 0)
	{
		handleWin32Error(hwndDlg);
		fclose(f);
		Error("CANNOT_SAVE_SYS_ENCRYPTION_SETTINGS");
		return FALSE;
	}

	TCFlushFile(f);

	fclose(f);

	bSystemEncryptionStatusChanged = FALSE;
	BroadcastSysEncCfgUpdate();

	return TRUE;
}

// IMPORTANT: This function may be called only by Format (other modules can only _read_ the status).
// Returns TRUE if successful (otherwise FALSE)
static BOOL ChangeHiddenOSCreationPhase(int newPhase)
{
	if (!CreateSysEncMutex())
	{
		Error("SYSTEM_ENCRYPTION_IN_PROGRESS_ELSEWHERE");
		return FALSE;
	}

	try
	{
		BootEncObj->SetHiddenOSCreationPhase(newPhase);
	}
	catch (Exception &e)
	{
		e.Show(MainDlg);
		return FALSE;
	}

	//// The contents of the following items might be inappropriate after a change of the phase
	//szFileName[0] = 0;
	//szDiskFile[0] = 0;
	//nUIVolumeSize = 0;
	//nVolumeSize = 0;

	return TRUE;
}

// IMPORTANT: This function may be called only by Format (other modules can only _read_ the system encryption status).
// Returns TRUE if successful (otherwise FALSE)
static BOOL ChangeSystemEncryptionStatus(int newStatus)
{
	if (!CreateSysEncMutex())
	{
		Error("SYSTEM_ENCRYPTION_IN_PROGRESS_ELSEWHERE");
		return FALSE;		// Only one instance that has the mutex can modify the system encryption settings
	}

	SystemEncryptionStatus = newStatus;
	bSystemEncryptionStatusChanged = TRUE;

	if (newStatus == SYSENC_STATUS_ENCRYPTING)
	{
		// If the user has created a hidden OS and now is creating a decoy OS, we must wipe the hidden OS
		// config area in the MBR.
		WipeHiddenOSCreationConfig();
	}

	if (newStatus == SYSENC_STATUS_NONE && !IsHiddenOSRunning())
	{
		if (DetermineHiddenOSCreationPhase() != TC_HIDDEN_OS_CREATION_PHASE_NONE
			&& !ChangeHiddenOSCreationPhase(TC_HIDDEN_OS_CREATION_PHASE_NONE))
			return FALSE;

		WipeHiddenOSCreationConfig();
	}

	if (!SaveSysEncSettings(MainDlg))
	{
		return FALSE;
	}

	return TRUE;
}

// COPIED CODE /\ from: src/Format/Tcformat.c

// Use this function e.g. if the config file with the system encryption settings was lost or not written
// correctly, and we don't know whether to encrypt or decrypt (but we know that encryption or decryption
// is required). Returns FALSE if failed or cancelled.
static BOOL ResolveUnknownSysEncDirection(void)
{
	if (CreateSysEncMutex())
	{
		if (SystemEncryptionStatus != SYSENC_STATUS_ENCRYPTING
			&& SystemEncryptionStatus != SYSENC_STATUS_DECRYPTING)
		{
			try
			{
				BootEncStatus = BootEncObj->GetStatus();
			}
			catch (Exception &e)
			{
				e.Show(MainDlg);
				Error("ERR_GETTING_SYSTEM_ENCRYPTION_STATUS");
				return FALSE;
			}

			if (BootEncStatus.SetupInProgress)
			{
				return ChangeSystemEncryptionStatus(
					(BootEncStatus.SetupMode != SetupDecryption) ? SYSENC_STATUS_ENCRYPTING : SYSENC_STATUS_DECRYPTING);
			}
			else
			{
				// Ask the user to select encryption, decryption, or cancel

				char *tmpStr[] = { 0,
					!BootEncStatus.DriveEncrypted ? "CHOOSE_ENCRYPT_OR_DECRYPT_FINALIZE_DECRYPT_NOTE" : "CHOOSE_ENCRYPT_OR_DECRYPT",
					"ENCRYPT",
					"DECRYPT",
					"IDCANCEL",
					0 };

				switch (AskMultiChoice((void **)tmpStr, FALSE))
				{
				case 1:
					return ChangeSystemEncryptionStatus(SYSENC_STATUS_ENCRYPTING);
				case 2:
					return ChangeSystemEncryptionStatus(SYSENC_STATUS_DECRYPTING);
				default:
					return FALSE;
				}
			}
		}
		else
			return TRUE;
	}
	else
	{
		Error("SYSTEM_ENCRYPTION_IN_PROGRESS_ELSEWHERE");
		return FALSE;
	}
}

// WARNING: This function may take a long time to finish
static unsigned int DetermineHiddenOSCreationPhase(void)
{
	unsigned int phase = TC_HIDDEN_OS_CREATION_PHASE_NONE;

	try
	{
		phase = BootEncObj->GetHiddenOSCreationPhase();
	}
	catch (Exception &e)
	{
		e.Show(MainDlg);
		AbortProcess("ERR_GETTING_SYSTEM_ENCRYPTION_STATUS");
	}

	return phase;
}



static BOOL ElevateWholeWizardProcess(string arguments)
{
	char modPath[MAX_PATH];

	if (IsAdmin())
		return TRUE;

	if (!IsUacSupported())
		return IsAdmin();

	GetModuleFileName(NULL, modPath, sizeof(modPath));

	if ((int)ShellExecute(MainDlg, "runas", modPath, (string("/q UAC ") + arguments).c_str(), NULL, SW_SHOWNORMAL) > 32)
	{
		exit(0);
	}
	else
	{
		Error("UAC_INIT_ERROR");
		return FALSE;
	}
}

static void WipePasswordsAndKeyfiles(void)
{
	WCHAR tmp[MAX_PASSWORD + 1];

	// Attempt to wipe passwords stored in the input field buffers
	memset(tmp, 0x20, sizeof(tmp)); //0x20 = ASCII space or 0x2020 = Unicode DAGGER
	tmp[MAX_PASSWORD] = 0;
	SetWindowTextW(hPasswordInputField, tmp);
	SetWindowTextW(hVerifyPasswordInputField, tmp);

	burn(&szTmpPas[0], sizeof(szTmpPas));
	burn(&szVerify[0], sizeof(szVerify));
	burn(&volumePassword, sizeof(volumePassword));
	burn(&szRawPassword[0], sizeof(szRawPassword));

	SetWindowTextW(hPasswordInputField, L"");
	SetWindowTextW(hVerifyPasswordInputField, L"");

	KeyFileRemoveAll(&FirstKeyFile);
	KeyFileRemoveAll(&defaultKeyFilesParam.FirstKeyFile);
}

// Returns TRUE if the system partition/drive is completely encrypted
static BOOL SysDriveOrPartitionFullyEncrypted(BOOL bSilent)
{
	/* If you update this function, revise SysDriveOrPartitionFullyEncrypted() in Mount.c as well. */

	static BootEncryptionStatus locBootEncStatus;

	try
	{
		locBootEncStatus = BootEncObj->GetStatus();
	}
	catch (Exception &e)
	{
		if (!bSilent)
			e.Show(MainDlg);
	}

	return (!locBootEncStatus.SetupInProgress
		&& locBootEncStatus.ConfiguredEncryptedAreaEnd != 0
		&& locBootEncStatus.ConfiguredEncryptedAreaEnd != -1
		&& locBootEncStatus.ConfiguredEncryptedAreaStart == locBootEncStatus.EncryptedAreaStart
		&& locBootEncStatus.ConfiguredEncryptedAreaEnd == locBootEncStatus.EncryptedAreaEnd);
}

// COPIED CODE \/ from: src/Format/Tcformat.c

// Returns TRUE if system encryption or decryption had been or is in progress and has not been completed
static BOOL SysEncryptionOrDecryptionRequired(void)
{
	/* If you update this function, revise SysEncryptionOrDecryptionRequired() in Mount.c as well. */

	static BootEncryptionStatus locBootEncStatus;

	try
	{
		locBootEncStatus = BootEncObj->GetStatus();
	}
	catch (Exception &e)
	{
		e.Show(MainDlg);
	}

	return (SystemEncryptionStatus == SYSENC_STATUS_ENCRYPTING
		|| SystemEncryptionStatus == SYSENC_STATUS_DECRYPTING
		||
		(
		locBootEncStatus.DriveMounted
		&&
		(
		locBootEncStatus.ConfiguredEncryptedAreaStart != locBootEncStatus.EncryptedAreaStart
		|| locBootEncStatus.ConfiguredEncryptedAreaEnd != locBootEncStatus.EncryptedAreaEnd
		)
		)
		);
}

// COPIED CODE /\ from: src/Format/Tcformat.c

// If the return code of this function is ignored and newWizardMode == WIZARD_MODE_SYS_DEVICE, then this function
// may be called only after CreateSysEncMutex() returns TRUE. It returns TRUE if successful (otherwise FALSE).
static BOOL ChangeWizardMode(int newWizardMode)
{
	if (WizardMode != newWizardMode)
	{
		if (WizardMode == WIZARD_MODE_SYS_DEVICE || newWizardMode == WIZARD_MODE_SYS_DEVICE)
		{
			if (newWizardMode == WIZARD_MODE_SYS_DEVICE)
			{
				if (!CreateSysEncMutex())
				{
					Error("SYSTEM_ENCRYPTION_IN_PROGRESS_ELSEWHERE");
					return FALSE;
				}
			}

			// If the previous mode was different, the password may have been typed using a different
			// keyboard layout (which might confuse the user and cause other problems if system encryption
			// was or will be involved).
			WipePasswordsAndKeyfiles();
		}

		if (newWizardMode != WIZARD_MODE_NONSYS_DEVICE)
			bInPlaceEncNonSys = FALSE;

		if (newWizardMode == WIZARD_MODE_NONSYS_DEVICE && !IsAdmin() && IsUacSupported())
		{
			if (!ElevateWholeWizardProcess("/e"))
				return FALSE;
		}

		// The contents of the following items may be inappropriate after a change of mode
		szFileName[0] = 0;
		szDiskFile[0] = 0;
		nUIVolumeSize = 0;
		nVolumeSize = 0;

		WizardMode = newWizardMode;
	}

	bDevice = (WizardMode != WIZARD_MODE_FILE_CONTAINER);

	if (newWizardMode != WIZARD_MODE_SYS_DEVICE
		&& !bHiddenOS)
	{
		CloseSysEncMutex();
	}

	return TRUE;
}

// COPIED CODE \/ from: src/Format/Tcformat.c

// This functions is to be used when the wizard mode needs to be changed to WIZARD_MODE_SYS_DEVICE.
// If the function fails to switch the mode, it returns FALSE (otherwise TRUE).
BOOL SwitchWizardToSysEncMode(void)
{
	WaitCursor();

	try
	{
		BootEncStatus = BootEncObj->GetStatus();
		bWholeSysDrive = BootEncObj->SystemPartitionCoversWholeDrive();
	}
	catch (Exception &e)
	{
		e.Show(MainDlg);
		Error("ERR_GETTING_SYSTEM_ENCRYPTION_STATUS");
		NormalCursor();
		return FALSE;
	}

	// From now on, we should be the only instance of the TC wizard allowed to deal with system encryption
	if (!CreateSysEncMutex())
	{
		Warning("SYSTEM_ENCRYPTION_IN_PROGRESS_ELSEWHERE");
		NormalCursor();
		return FALSE;
	}

	// User-mode app may have crashed and its mutex may have gotten lost, so we need to check the driver status too
	if (BootEncStatus.SetupInProgress)
	{
		if (AskWarnYesNo("SYSTEM_ENCRYPTION_RESUME_PROMPT") == IDYES)
		{
			if (SystemEncryptionStatus != SYSENC_STATUS_ENCRYPTING
				&& SystemEncryptionStatus != SYSENC_STATUS_DECRYPTING)
			{
				// The config file with status was lost or not written correctly
				if (!ResolveUnknownSysEncDirection())
				{
					CloseSysEncMutex();
					NormalCursor();
					return FALSE;
				}
			}

			bDirectSysEncMode = TRUE;
			ChangeWizardMode(WIZARD_MODE_SYS_DEVICE);
			LoadPage(MainDlg, SYSENC_ENCRYPTION_PAGE);
			NormalCursor();
			return TRUE;
		}
		else
		{
			CloseSysEncMutex();
			Error("SYS_ENCRYPTION_OR_DECRYPTION_IN_PROGRESS");
			NormalCursor();
			return FALSE;
		}
	}

	if (BootEncStatus.DriveMounted
		|| BootEncStatus.DriveEncrypted
		|| SysEncryptionOrDecryptionRequired())
	{

		if (!SysDriveOrPartitionFullyEncrypted(FALSE)
			&& AskWarnYesNo("SYSTEM_ENCRYPTION_RESUME_PROMPT") == IDYES)
		{
			if (SystemEncryptionStatus == SYSENC_STATUS_NONE)
			{
				// If the config file with status was lost or not written correctly, we
				// don't know whether to encrypt or decrypt (but we know that encryption or
				// decryption is required). Ask the user to select encryption, decryption, 
				// or cancel
				if (!ResolveUnknownSysEncDirection())
				{
					CloseSysEncMutex();
					NormalCursor();
					return FALSE;
				}
			}

			bDirectSysEncMode = TRUE;
			ChangeWizardMode(WIZARD_MODE_SYS_DEVICE);
			LoadPage(MainDlg, SYSENC_ENCRYPTION_PAGE);
			NormalCursor();
			return TRUE;
		}
		else
		{
			CloseSysEncMutex();
			Error("SETUP_FAILED_BOOT_DRIVE_ENCRYPTED");
			NormalCursor();
			return FALSE;
		}
	}
	else
	{
		// Check compliance with requirements for boot encryption

		if (!IsAdmin())
		{
			if (!IsUacSupported())
			{
				Warning("ADMIN_PRIVILEGES_WARN_DEVICES");
			}
		}

		try
		{
			BootEncObj->CheckRequirements();
		}
		catch (Exception &e)
		{
			CloseSysEncMutex();
			e.Show(MainDlg);
			NormalCursor();
			return FALSE;
		}

		if (!ChangeWizardMode(WIZARD_MODE_SYS_DEVICE))
		{
			NormalCursor();
			return FALSE;
		}

		if (bSysDriveSelected || bSysPartitionSelected)
		{
			// The user selected the non-sys-device wizard mode but then selected a system device

			bWholeSysDrive = (bSysDriveSelected && !bSysPartitionSelected);

			bSysDriveSelected = FALSE;
			bSysPartitionSelected = FALSE;

			try
			{
				if (!bHiddenVol)
				{
					if (bWholeSysDrive && !BootEncObj->SystemPartitionCoversWholeDrive())
					{
						if (BootEncObj->SystemDriveContainsNonStandardPartitions())
						{
							if (AskWarnYesNoString((wstring(GetString("SYSDRIVE_NON_STANDARD_PARTITIONS")) + L"\n\n" + GetString("ASK_ENCRYPT_PARTITION_INSTEAD_OF_DRIVE")).c_str()) == IDYES)
								bWholeSysDrive = FALSE;
						}

						if (!IsOSAtLeast(WIN_VISTA) && bWholeSysDrive)
						{
							if (BootEncObj->SystemDriveContainsExtendedPartition())
							{
								bWholeSysDrive = FALSE;

								Error("WDE_UNSUPPORTED_FOR_EXTENDED_PARTITIONS");

								if (AskYesNo("ASK_ENCRYPT_PARTITION_INSTEAD_OF_DRIVE") == IDNO)
								{
									ChangeWizardMode(WIZARD_MODE_NONSYS_DEVICE);
									return FALSE;
								}
							}
							else
								Warning("WDE_EXTENDED_PARTITIONS_WARNING");
						}
					}
					else if (BootEncObj->SystemPartitionCoversWholeDrive()
						&& !bWholeSysDrive)
						bWholeSysDrive = (AskYesNo("WHOLE_SYC_DEVICE_RECOM") == IDYES);
				}

			}
			catch (Exception &e)
			{
				e.Show(MainDlg);
				return FALSE;
			}

			if (!bHiddenVol)
			{
				// Skip SYSENC_SPAN_PAGE and SYSENC_TYPE_PAGE as the user already made the choice
				LoadPage(MainDlg, bWholeSysDrive ? SYSENC_PRE_DRIVE_ANALYSIS_PAGE : SYSENC_MULTI_BOOT_MODE_PAGE);
			}
			else
			{
				// The user selected the non-sys-device wizard mode but then selected a system device.
				// In addition, he selected the hidden volume mode.

				if (bWholeSysDrive)
					Warning("HIDDEN_OS_PRECLUDES_SINGLE_KEY_WDE");

				bWholeSysDrive = FALSE;

				LoadPage(MainDlg, SYSENC_TYPE_PAGE);
			}
		}
		else
			LoadPage(MainDlg, SYSENC_TYPE_PAGE);

		NormalCursor();
		return TRUE;
	}
}

void SwitchWizardToFileContainerMode(void)
{
	ChangeWizardMode(WIZARD_MODE_FILE_CONTAINER);

	LoadPage(MainDlg, VOLUME_LOCATION_PAGE);

	NormalCursor();
}

void DisplayPortionsOfKeys(HWND headerKeyHandle, HWND masterKeyHandle, char *headerKeyStr, char *masterKeyStr, BOOL hideKeys)
{
	const wchar_t *hiddenKey = L"********************************                                              ";

	SetWindowTextW(headerKeyHandle, hideKeys ? hiddenKey : (SingleStringToWide(headerKeyStr) + GetString("TRIPLE_DOT_GLYPH_ELLIPSIS")).c_str());
	SetWindowTextW(masterKeyHandle, hideKeys ? hiddenKey : (SingleStringToWide(masterKeyStr) + GetString("TRIPLE_DOT_GLYPH_ELLIPSIS")).c_str());
}

// Tests whether the file system of the given volume is suitable to host a hidden volume,
// retrieves the cluster size, and scans the volume cluster bitmap. In addition, checks
// the CipherShed volume format version and the type of volume.
int AnalyzeHiddenVolumeHost(HWND hwndDlg, int *driveNo, __int64 hiddenVolHostSize, int *realClusterSize, __int64 *pnbrFreeClusters)
{
	HANDLE hDevice;
	DWORD bytesReturned;
	DWORD dwSectorsPerCluster, dwBytesPerSector, dwNumberOfFreeClusters, dwTotalNumberOfClusters;
	DWORD dwResult;
	int result;
	char szFileSystemNameBuffer[256];
	char tmpPath[7] = { '\\', '\\', '.', '\\', (char)*driveNo + 'A', ':', 0 };
	char szRootPathName[4] = { (char)*driveNo + 'A', ':', '\\', 0 };
	BYTE readBuffer[TC_MAX_VOLUME_SECTOR_SIZE * 2];
	LARGE_INTEGER offset, offsetNew;
	VOLUME_PROPERTIES_STRUCT volProp;

	memset(&volProp, 0, sizeof(volProp));
	volProp.driveNo = *driveNo;
	if (!DeviceIoControl(hDriver, TC_IOCTL_GET_VOLUME_PROPERTIES, &volProp, sizeof(volProp), &volProp, sizeof(volProp), &dwResult, NULL) || dwResult == 0)
	{
		handleWin32Error(hwndDlg);
		Error("CANT_ACCESS_OUTER_VOL");
		goto efsf_error;
	}

	if (volProp.volFormatVersion < TC_VOLUME_FORMAT_VERSION)
	{
		// We do not support creating hidden volumes within volumes created by TrueCrypt 5.1a or earlier.
		Error("ERR_VOL_FORMAT_BAD");
		return 0;
	}

	if (volProp.hiddenVolume)
	{
		// The user entered a password for a hidden volume
		Error("ERR_HIDDEN_NOT_NORMAL_VOLUME");
		return 0;
	}

	if (volProp.volumeHeaderFlags & TC_HEADER_FLAG_NONSYS_INPLACE_ENC
		|| volProp.volumeHeaderFlags & TC_HEADER_FLAG_ENCRYPTED_SYSTEM)
	{
		Warning("ERR_HIDDEN_VOL_HOST_ENCRYPTED_INPLACE");
		return 0;
	}

	hDevice = CreateFile(tmpPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hDevice == INVALID_HANDLE_VALUE)
	{
		MessageBoxW(hwndDlg, GetString("CANT_ACCESS_OUTER_VOL"), lpszTitle, ICON_HAND);
		goto efsf_error;
	}

	offset.QuadPart = 0;

	if (SetFilePointerEx(hDevice, offset, &offsetNew, FILE_BEGIN) == 0)
	{
		handleWin32Error(hwndDlg);
		goto efs_error;
	}

	result = ReadFile(hDevice, &readBuffer, TC_MAX_VOLUME_SECTOR_SIZE, &bytesReturned, NULL);

	if (result == 0)
	{
		handleWin32Error(hwndDlg);
		MessageBoxW(hwndDlg, GetString("CANT_ACCESS_OUTER_VOL"), lpszTitle, ICON_HAND);
		goto efs_error;
	}

	CloseHandle(hDevice);
	hDevice = INVALID_HANDLE_VALUE;

	// Determine file system type

	GetVolumeInformation(szRootPathName, NULL, 0, NULL, NULL, NULL, szFileSystemNameBuffer, sizeof(szFileSystemNameBuffer));

	// The Windows API sometimes fails to indentify the file system correctly so we're using "raw" analysis too.
	if (!strncmp(szFileSystemNameBuffer, "FAT", 3)
		|| (readBuffer[0x36] == 'F' && readBuffer[0x37] == 'A' && readBuffer[0x38] == 'T')
		|| (readBuffer[0x52] == 'F' && readBuffer[0x53] == 'A' && readBuffer[0x54] == 'T'))
	{
		// FAT12/FAT16/FAT32

		// Retrieve the cluster size
		*realClusterSize = ((int)readBuffer[0xb] + ((int)readBuffer[0xc] << 8)) * (int)readBuffer[0xd];

		// Get the map of the clusters that are free and in use on the outer volume.
		// The map will be scanned to determine the size of the uninterrupted block of free
		// space (provided there is any) whose end is aligned with the end of the volume.
		// The value will then be used to determine the maximum possible size of the hidden volume.

		return ScanVolClusterBitmap(hwndDlg,
			driveNo,
			hiddenVolHostSize / *realClusterSize,
			pnbrFreeClusters);
	}
	else if (!strncmp(szFileSystemNameBuffer, "NTFS", 4))
	{
		// NTFS

		if (nCurrentOS == WIN_2000)
		{
			Error("HIDDEN_VOL_HOST_UNSUPPORTED_FILESYS_WIN2000");
			return 0;
		}

		if (bHiddenVolDirect && GetVolumeDataAreaSize(FALSE, hiddenVolHostSize) <= TC_MAX_FAT_SECTOR_COUNT * GetFormatSectorSize())
			Info("HIDDEN_VOL_HOST_NTFS");

		if (!GetDiskFreeSpace(szRootPathName,
			&dwSectorsPerCluster,
			&dwBytesPerSector,
			&dwNumberOfFreeClusters,
			&dwTotalNumberOfClusters))
		{
			handleWin32Error(hwndDlg);
			Error("CANT_GET_OUTER_VOL_INFO");
			return -1;
		};

		*realClusterSize = dwBytesPerSector * dwSectorsPerCluster;

		// Get the map of the clusters that are free and in use on the outer volume.
		// The map will be scanned to determine the size of the uninterrupted block of free
		// space (provided there is any) whose end is aligned with the end of the volume.
		// The value will then be used to determine the maximum possible size of the hidden volume.

		return ScanVolClusterBitmap(hwndDlg,
			driveNo,
			hiddenVolHostSize / *realClusterSize,
			pnbrFreeClusters);
	}
	else
	{
		// Unsupported file system

		Error((nCurrentOS == WIN_2000) ? "HIDDEN_VOL_HOST_UNSUPPORTED_FILESYS_WIN2000" : "HIDDEN_VOL_HOST_UNSUPPORTED_FILESYS");
		return 0;
	}

efs_error:
	CloseHandle(hDevice);

efsf_error:
	CloseVolumeExplorerWindows(hwndDlg, *driveNo);

	return -1;
}

// COPIED CODE /\ from: src/Format/Tcformat.c






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void dumpStatus() {
	static BootEncryptionStatus status;
	wchar_t s[200];

	if (BootEncObj == NULL) {
		MessageBoxW(NULL, L"BootEncObj is NULL", L"DevTool", MB_ICONINFORMATION);
		return;
	}

	status = BootEncObj->GetStatus();

	swprintf(s, sizeof(s), L"DeviceFilterActive: %i\nDriveMounted: %i\nDriveEncrypted: %i\nSetupMode: %i\nEncAreaStart: 0x%Lx\nEncAreaEnd: 0x%Lx",
		status.DeviceFilterActive, status.DriveMounted, status.DriveEncrypted, status.SetupMode, 
		status.EncryptedAreaStart, status.EncryptedAreaEnd);
	MessageBoxW(NULL, s, L"EncryptionStatus", MB_ICONINFORMATION);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, char *lpszCommandLine, int nCmdShow)
{
	int status;

	try
	{
		BootEncObj = new BootEncryption (NULL);
	}
	catch (Exception &e)
	{
		e.Show (NULL);
	}

	if (BootEncObj == NULL) {
		MessageBoxW(NULL, L"INIT_SYS_ENC", L"DevTool", MB_ICONINFORMATION);
		exit(1);
	}

	InitCommonControls ();
	InitOSVersionInfo();
	CoInitialize(NULL);
	if (!EncryptionThreadPoolStart(ReadEncryptionThreadPoolFreeCpuCountLimit()))
	{
		MessageBoxW(NULL, L"EncryptionThreadPoolStart", L"DevTool", MB_ICONINFORMATION);
		exit(1);
	}

	//InitApp(hInstance, lpszCommandLine);

	status = DriverAttach ();
	if (status != 0)
	{
		MessageBoxW(NULL, L"NODRIVER", L"DevTool", MB_ICONINFORMATION);
		exit(1);
	}

	if (!AutoTestAlgorithms()) {
		MessageBoxW(NULL, L"ERR_SELF_TESTS_FAILED", L"DevTool", MB_ICONINFORMATION);
		exit(1);
	}

	///////////////////////////////////////////////////////////
	dumpStatus();

	// UNINSTALL
	if (strstr(lpszCommandLine, "uninstall") == lpszCommandLine)
	{
		MessageBoxW(NULL, L"uninstalling driver...", L"DevTool", MB_ICONINFORMATION);
		try {
			BootEncObj->RegisterFilterDriver(false, BootEncryption::DriveFilter);
			BootEncObj->RegisterFilterDriver(false, BootEncryption::VolumeFilter);
			BootEncObj->RegisterFilterDriver(false, BootEncryption::DumpFilter);
			BootEncObj->SetDriverServiceStartType(SERVICE_SYSTEM_START);
		}
		catch (...) {}

		MessageBoxW(NULL, L"Ok.", L"DevTool", MB_ICONINFORMATION);
		dumpStatus();

		return 0;
	}

	// INSTALL
	if (strstr(lpszCommandLine, "install") == lpszCommandLine)
	{
		MessageBoxW(NULL, L"installing driver...", L"DevTool", MB_ICONINFORMATION);

		BootEncObj->RegisterBootDriver(false /* hidden */);

		MessageBoxW(NULL, L"Ok.", L"DevTool", MB_ICONINFORMATION);
		dumpStatus();

		return 0;
	}

	MessageBoxW(NULL, L"usage: dev-tool [install|uninstall]", L"DevTool", MB_ICONINFORMATION);

	return 0;
}

