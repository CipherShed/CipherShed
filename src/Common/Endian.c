/*
 Legal Notice: Some portions of the source code contained in this file were
 derived from the source code of Encryption for the Masses 2.02a, which is
 Copyright (c) 1998-2000 Paul Le Roux and which is governed by the 'License
 Agreement for Encryption for the Masses'. Modifications and additions to
 the original source code (contained in this file) and all other portions
 of this file are Copyright (c) 2003-2009 TrueCrypt Developers Association
 and are governed by the TrueCrypt License 3.0 the full text of which is
 contained in the file License.txt included in TrueCrypt binary and source
 code distribution packages. */

#include "Tcdefs.h"
#include "Common/Endian.h"


unsigned __int16 MirrorBytes16 (unsigned __int16 x)
{
	return (x << 8) | (x >> 8);
}


unsigned __int32 MirrorBytes32 (unsigned __int32 x)
{
	unsigned __int32 n = (unsigned __int8) x;
	n <<= 8; n |= (unsigned __int8) (x >> 8);
	n <<= 8; n |= (unsigned __int8) (x >> 16);
	return (n << 8) | (unsigned __int8) (x >> 24);
}

#ifndef TC_NO_COMPILER_INT64
uint64 MirrorBytes64 (uint64 x)
{
	uint64 n = (unsigned __int8) x;
	n <<= 8; n |= (unsigned __int8) (x >> 8);
	n <<= 8; n |= (unsigned __int8) (x >> 16);
	n <<= 8; n |= (unsigned __int8) (x >> 24);
	n <<= 8; n |= (unsigned __int8) (x >> 32);
	n <<= 8; n |= (unsigned __int8) (x >> 40);
	n <<= 8; n |= (unsigned __int8) (x >> 48);
	return (n << 8) | (unsigned __int8) (x >> 56);
}
#endif

void
LongReverse (unsigned __int32 *buffer, unsigned byteCount)
{
	unsigned __int32 value;

	byteCount /= sizeof (unsigned __int32);
	while (byteCount--)
	{
		value = *buffer;
		value = ((value & 0xFF00FF00L) >> 8) | \
		    ((value & 0x00FF00FFL) << 8);
		*buffer++ = (value << 16) | (value >> 16);
	}
}
