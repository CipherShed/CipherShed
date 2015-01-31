#include "diskutil.h"

#include "../snprintf.h"


BOOL GetDeviceInfo (const char *deviceName, DISK_PARTITION_INFO_STRUCT *info)
{
	DWORD dwResult;

	memset (info, 0, sizeof(*info));
	_snwprintf ((PWSTR) &info->deviceName, ARRAY_LENGTH(info->deviceName), L"%hs", deviceName);

	return DeviceIoControl (hDriver, TC_IOCTL_GET_DRIVE_PARTITION_INFO, info, sizeof (*info), info, sizeof (*info), &dwResult, NULL);
}


BOOL GetDriveGeometry (const char *deviceName, PDISK_GEOMETRY diskGeometry)
{
	BOOL bResult;
	DWORD dwResult;
	DISK_GEOMETRY_STRUCT dg;

	memset (&dg, 0, sizeof(dg));
	_snwprintf ((PWSTR) &dg.deviceName, ARRAY_LENGTH(dg.deviceName), L"%hs", deviceName);

	bResult = DeviceIoControl (hDriver, TC_IOCTL_GET_DRIVE_GEOMETRY, &dg,
		sizeof (dg), &dg, sizeof (dg), &dwResult, NULL);

	memcpy (diskGeometry, &dg.diskGeometry, sizeof (DISK_GEOMETRY));
	return bResult;
}


// Returns drive letter number assigned to device (-1 if none)
int GetDiskDeviceDriveLetter (PWSTR deviceName)
{
	int i;
	WCHAR link[MAX_PATH];
	WCHAR target[MAX_PATH];
	WCHAR device[MAX_PATH];

	if (!ResolveSymbolicLink (deviceName, device))
		wcscpy (device, deviceName);

	for (i = 0; i < 26; i++)
	{
		WCHAR drive[] = { (WCHAR) i + 'A', ':', 0 };

		wcscpy (link, L"\\DosDevices\\");
		wcscat (link, drive);

		ResolveSymbolicLink (link, target);

		if (wcscmp (device, target) == 0)
			return i;
	}

	return -1;
}
