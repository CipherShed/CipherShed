#include "volutil.h"

#include "../snprintf.h"
#include "../util/csstringutil.h"
#include "../Apidrvr.h"
#include "../fsutil/fsutil.h"
#include "../Dlgcode.h"

#ifdef CS_UNITTESTING
#include "../../unit-tests/faux/windows/ARRAYSIZE.h"
#include "../../unit-tests/faux/windows/QueryDosDevice.h"
#include "../../unit-tests/faux/windows/swprintf_s.h"
#include "../../unit-tests/faux/windows/strcpy_s.h"
#include "../../unit-tests/faux/windows/_wcsicmp.h"
#include "../../unit-tests/faux/windows/OutputDebugString.h"
#endif

using namespace std;

void CreateFullVolumePath (char *lpszDiskFile, const char *lpszFileName, BOOL * bDevice)
{
	UpperCaseCopy (lpszDiskFile, lpszFileName);

	*bDevice = FALSE;
	// seriously, just use strlen and optimize or #define and sizeof: http://stackoverflow.com/questions/12541739/determining-the-length-of-a-string-literal
	if (memcmp (lpszDiskFile, "\\DEVICE", sizeof (char) * 7) == 0)
	{
		*bDevice = TRUE;
	}

	strcpy (lpszDiskFile, lpszFileName);

#if _DEBUG
	OutputDebugStringA("CreateFullVolumePath: ");
	OutputDebugStringA(lpszDiskFile);
	OutputDebugStringA("\n");
#endif

}

BOOL IsVolumeDeviceHosted (const char *lpszDiskFile)
{
	// and what about \\DeViCe\\ or \\device\\ ?
	return strstr (lpszDiskFile, "\\Device\\") == lpszDiskFile
		|| strstr (lpszDiskFile, "\\DEVICE\\") == lpszDiskFile;
}

std::string HarddiskVolumePathToPartitionPath (const std::string &harddiskVolumePath)
{
	wstring volPath = SingleStringToWide (harddiskVolumePath);

	for (int driveNumber = 0; driveNumber < MAX_HOST_DRIVE_NUMBER; driveNumber++)
	{
		for (int partNumber = 0; partNumber < MAX_HOST_PARTITION_NUMBER; partNumber++)
		{
			wchar_t partitionPath[TC_MAX_PATH];
			swprintf_s (partitionPath, ARRAYSIZE (partitionPath), L"\\Device\\Harddisk%d\\Partition%d", driveNumber, partNumber);

			wchar_t resolvedPath[TC_MAX_PATH];
			if (ResolveSymbolicLink (partitionPath, (PWSTR)resolvedPath))
			{
				if (volPath == resolvedPath)
					return WideToSingleString (partitionPath);
			}
			else if (partNumber == 0)
				break;
		}
	}

	return string();
}

std::string VolumeGuidPathToDevicePath (std::string volumeGuidPath)
{
	if (volumeGuidPath.find ("\\\\?\\") == 0)
		volumeGuidPath = volumeGuidPath.substr (4);

	if (volumeGuidPath.find ("Volume{") != 0 || volumeGuidPath.rfind ("}\\") != volumeGuidPath.size() - 2)
		return string();

	char volDevPath[TC_MAX_PATH];
	if (QueryDosDeviceA(volumeGuidPath.substr (0, volumeGuidPath.size() - 1).c_str(), volDevPath, TC_MAX_PATH) == 0)
		return string();

	string partitionPath = HarddiskVolumePathToPartitionPath (volDevPath);

	return partitionPath.empty() ? volDevPath : partitionPath;
}

BOOL IsMountedVolume (const char *volname)
{
	MOUNT_LIST_STRUCT mlist;
	DWORD dwResult;
	int i;
	char volume[TC_MAX_PATH*2+16];

	strcpy (volume, volname);

	if (strstr (volname, "\\Device\\") != volname)
		snprintf(volume, ARRAY_LENGTH(volume), "\\??\\%s", volname);

	string resolvedPath = VolumeGuidPathToDevicePath (volname);
	if (!resolvedPath.empty())
		strcpy_s (volume, sizeof (volume), resolvedPath.c_str());

	ToUNICODE (volume);

	memset (&mlist, 0, sizeof (mlist));
	DeviceIoControl (hDriver, TC_IOCTL_GET_MOUNTED_VOLUMES, &mlist,
		sizeof (mlist), &mlist, sizeof (mlist), &dwResult,
		NULL);

	for (i=0 ; i<26; i++)
		if (0 == _wcsicmp ((wchar_t *) mlist.wszVolume[i], (const wchar_t*)volume))
			return TRUE;

	return FALSE;
}

int GetMountedVolumeDriveNo (char *volname)
{
	MOUNT_LIST_STRUCT mlist;
	DWORD dwResult;
	int i;
	char volume[TC_MAX_PATH*2+16];

	if (volname == NULL)
		return -1;

	strcpy (volume, volname);

	if (strstr (volname, "\\Device\\") != volname)
		snprintf(volume, ARRAY_LENGTH(volume), "\\??\\%s", volname);

	string resolvedPath = VolumeGuidPathToDevicePath (volname);
	if (!resolvedPath.empty())
		strcpy_s (volume, sizeof (volume), resolvedPath.c_str());

	ToUNICODE (volume);

	memset (&mlist, 0, sizeof (mlist));
	DeviceIoControl (hDriver, TC_IOCTL_GET_MOUNTED_VOLUMES, &mlist,
		sizeof (mlist), &mlist, sizeof (mlist), &dwResult,
		NULL);

	for (i=0 ; i<26; i++)
		if (0 == _wcsicmp ((wchar_t *) mlist.wszVolume[i], (const wchar_t*)volume))
			return i;

	return -1;
}