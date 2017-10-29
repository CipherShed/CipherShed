typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef unsigned __int8 byte;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;

typedef byte BiosResult;

typedef unsigned __int32	TC_LARGEST_COMPILER_UINT;

typedef union 
{
	struct 
	{
		unsigned __int32 LowPart;
		unsigned __int32 HighPart;
	};
#ifndef TC_NO_COMPILER_INT64
	uint64 Value;
#endif

} UINT64_STRUCT;


typedef UINT64_STRUCT uint64;

typedef char bool;

struct BiosLbaPacket
{
	byte Size;
	byte Reserved;
	uint16 SectorCount;
	uint32 Buffer;
	uint64 Sector;
};

#define TC_MAX_BIOS_DISK_IO_RETRIES 5

enum
{
	BiosResultSuccess = 0x00,
	BiosResultInvalidFunction = 0x01
};

enum
{
	BiosResultEccCorrected = 0x11
};



static BiosResult ReadWriteSectors (bool write, BiosLbaPacket &dapPacket, byte drive, const uint64 &sector, uint16 sectorCount, bool silent)
{
/*
	CheckStack();

	if (!IsLbaSupported (drive))
	{
		DriveGeometry geometry;

		BiosResult result = GetDriveGeometry (drive, geometry, silent);
		if (result != BiosResultSuccess)
			return result;

		ChsAddress chs;
		LbaToChs (geometry, sector, chs);
		return ReadWriteSectors (write, (uint16) (dapPacket.Buffer >> 16), (uint16) dapPacket.Buffer, drive, chs, sectorCount, silent);
	}
*/
	dapPacket.Size = sizeof (dapPacket);
	dapPacket.Reserved = 0;
	dapPacket.SectorCount = sectorCount;
	dapPacket.Sector = sector;

	byte function = write ? 0x43 : 0x42;
	
	BiosResult result;
	byte tryCount = TC_MAX_BIOS_DISK_IO_RETRIES;

	do
	{
		result = BiosResultSuccess;

#ifndef __GNUC__
		__asm
		{
			mov	bx, 0x55aa
			mov	dl, drive
			mov si, [dapPacket]
			mov	ah, function
			xor al, al
			int	0x13
			jnc ok				// If CF=0, ignore AH to prevent issues caused by potential bugs in BIOSes
			mov	result, ah
		ok:
		}
#else
	asm volatile
	(R"ASM(
			mov	$0x55aa, %%bx
			mov	%[drive], %%dl
			mov %[dapPacket], %%si
			mov	%[function], %%ah
			xor %%al, %%al
			int	$0x13
			jnc ReadWriteSectors_ok				// If CF=0, ignore AH to prevent issues caused by potential bugs in BIOSes
			mov	%%ah, %[result]
ReadWriteSectors_ok:
)ASM"
		: [result] "=g" (result)
		: [drive] "g" (drive), 
		  [dapPacket] "g" (dapPacket), 
		  [function] "g" (function)
		: /* no clobbers */
	);
#endif

		if (result == BiosResultEccCorrected)
			result = BiosResultSuccess;

	// Some BIOSes report I/O errors prematurely in some cases
	} while (result != BiosResultSuccess && --tryCount != 0);
/*
	if (!silent && result != BiosResultSuccess)
		PrintDiskError (result, write, drive, &sector);
*/
	return result;
}

