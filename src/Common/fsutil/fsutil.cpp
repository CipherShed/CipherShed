#include "fsutil.h"

#include <string.h>
#include "../Apidrvr.h"
#include "../Dlgcode.h"

BOOL ResolveSymbolicLink (const wchar_t *symLinkName, PWSTR targetName)
{
	BOOL bResult;
	DWORD dwResult;
	RESOLVE_SYMLINK_STRUCT resolve;

	memset (&resolve, 0, sizeof(resolve));
	wcscpy ((PWSTR) &resolve.symLinkName, symLinkName);

	bResult = DeviceIoControl (hDriver, TC_IOCTL_GET_RESOLVED_SYMLINK, &resolve,
		sizeof (resolve), &resolve, sizeof (resolve), &dwResult,
		NULL);

	wcscpy (targetName, (PWSTR) &resolve.targetName);

	return bResult;
}

// WARNING: This function does NOT provide 100% reliable results -- do NOT use it for critical/dangerous operations!
// Return values: 0 - filesystem does not appear empty, 1 - filesystem appears empty, -1 - an error occurred
int FileSystemAppearsEmpty (const char *devicePath)
{
	float percentFreeSpace = 0.0;
	__int64 occupiedBytes = 0;

	if (GetStatsFreeSpaceOnPartition (devicePath, &percentFreeSpace, &occupiedBytes, TRUE) != -1)
	{
		if (occupiedBytes > BYTES_PER_GB && percentFreeSpace < 99.99	// "percentFreeSpace < 99.99" is needed because an NTFS filesystem larger than several terabytes can have more than 1GB of data in use, even if there are no files stored on it.
			|| percentFreeSpace < 88)		// A 24-MB NTFS filesystem has 11.5% of space in use even if there are no files stored on it.
		{
			return 0;
		}
		else
			return 1;
	}
	else
		return -1;
}