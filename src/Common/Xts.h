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

 Copyright (c) 2008-2010 TrueCrypt Developers Association. All rights reserved.

 Governed by the TrueCrypt License 3.0 the full text of which is contained in
 the file License.txt included in TrueCrypt binary and source code distribution
 packages.
*/

#ifndef XTS_H
#define XTS_H

// Header files (optional)

#include "Tcdefs.h"
#include "Endian.h" // we are in Common ... #include "Common/Endian.h"
#ifndef EFI
#include "Crypto.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EFI
// taken from Crypto.h:
// Encryption data unit size, which may differ from the sector size and must always be 512
#define ENCRYPTION_DATA_UNIT_SIZE	512
// Ciphertext/plaintext block size for XTS mode (in bytes)
#define BYTES_PER_XTS_BLOCK			16
// Number of ciphertext/plaintext blocks per XTS data unit
#define BLOCKS_PER_XTS_DATA_UNIT	(ENCRYPTION_DATA_UNIT_SIZE / BYTES_PER_XTS_BLOCK)
#endif

// Macros

#ifndef LITTLE_ENDIAN
#	define LITTLE_ENDIAN	1
#endif

#ifndef BIG_ENDIAN
#	define BIG_ENDIAN		2
#endif

#ifndef BYTE_ORDER
#	define BYTE_ORDER		LITTLE_ENDIAN
#endif

#ifndef LE64
#	if BYTE_ORDER == LITTLE_ENDIAN
#		define LE64(x)	(x)
#	endif
#endif

// Custom data types

#ifndef TC_LARGEST_COMPILER_UINT
#	ifdef TC_NO_COMPILER_INT64
		typedef unsigned __int32	TC_LARGEST_COMPILER_UINT;
#	else
#ifdef EFI
		typedef uint64	TC_LARGEST_COMPILER_UINT;
#else
		typedef unsigned __int64	TC_LARGEST_COMPILER_UINT;
#endif
#	endif
#endif

#ifndef TCDEFS_H
typedef union 
{
	struct 
	{
		unsigned __int32 LowPart;
		unsigned __int32 HighPart;
	};
#	ifndef TC_NO_COMPILER_INT64
	unsigned __int64 Value;
#	endif

} UINT64_STRUCT;
#endif

// Public function prototypes

void EncryptBufferXTS (unsigned __int8 *buffer, TC_LARGEST_COMPILER_UINT length, const UINT64_STRUCT *startDataUnitNo, unsigned int startCipherBlockNo, unsigned __int8 *ks, unsigned __int8 *ks2, int cipher);
void DecryptBufferXTS (unsigned __int8 *buffer, TC_LARGEST_COMPILER_UINT length, const UINT64_STRUCT *startDataUnitNo, unsigned int startCipherBlockNo, unsigned __int8 *ks, unsigned __int8 *ks2, int cipher);
#ifndef EFI
static void EncryptBufferXTSParallel (unsigned __int8 *buffer, TC_LARGEST_COMPILER_UINT length, const UINT64_STRUCT *startDataUnitNo, unsigned int startCipherBlockNo, unsigned __int8 *ks, unsigned __int8 *ks2, int cipher);
static void EncryptBufferXTSNonParallel (unsigned __int8 *buffer, TC_LARGEST_COMPILER_UINT length, const UINT64_STRUCT *startDataUnitNo, unsigned int startCipherBlockNo, unsigned __int8 *ks, unsigned __int8 *ks2, int cipher);
static void DecryptBufferXTSParallel (unsigned __int8 *buffer, TC_LARGEST_COMPILER_UINT length, const UINT64_STRUCT *startDataUnitNo, unsigned int startCipherBlockNo, unsigned __int8 *ks, unsigned __int8 *ks2, int cipher);
static void DecryptBufferXTSNonParallel (unsigned __int8 *buffer, TC_LARGEST_COMPILER_UINT length, const UINT64_STRUCT *startDataUnitNo, unsigned int startCipherBlockNo, unsigned __int8 *ks, unsigned __int8 *ks2, int cipher);
#endif

#ifdef __cplusplus
}
#endif

#endif	// #ifndef XTS_H
