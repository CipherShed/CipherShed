/*  cs_crypto.h - CipherShed EFI boot loader
 *
 *	Copyright (c) 2015-2016  Falk Nedwal
 *
 *	Governed by the Apache 2.0 License 3.0 the full text of which is contained in
 *	the file License.txt included in CipherShed binary and source code distribution
 *	packages.
 */

#ifndef _CS_CRYPTO_H_
#define _CS_CRYPTO_H_

#include "../../Common/Tcdefs.h"
#include "../../Common/Endian.h"
#include "../../Common/GfMul.h"
#include "../../Common/Xts.h"
#include "../../Common/Pkcs5.h"
#include "../../Common/Crc.h"
#include "../../Common/Password.h"
#include "../../Crypto/Twofish.h"
#include "../../Crypto/Aes.h"
#include "../../Crypto/Serpent.h"
#include "../../Crypto/Rmd160.h"

#define TC_NO_COMPILER_INT64 //??

// Volume header version
#define VOLUME_HEADER_VERSION					0x0005

// Sector sizes
#define TC_MIN_VOLUME_SECTOR_SIZE				512
#define TC_MAX_VOLUME_SECTOR_SIZE				4096
#define TC_SECTOR_SIZE_FILE_HOSTED_VOLUME		512
#define TC_SECTOR_SIZE_LEGACY					512

#define TC_VOLUME_HEADER_EFFECTIVE_SIZE			512
#define HEADER_ENCRYPTED_DATA_SIZE			(TC_VOLUME_HEADER_EFFECTIVE_SIZE - HEADER_ENCRYPTED_DATA_OFFSET)

// Size of the salt (in bytes)
#define PKCS5_SALT_SIZE				64

// Size of the volume header area containing concatenated master key(s) and secondary key(s) (XTS mode)
#define MASTER_KEYDATA_SIZE			256

// Volume header field offsets
#define	HEADER_SALT_OFFSET					0
#define HEADER_ENCRYPTED_DATA_OFFSET		PKCS5_SALT_SIZE
#define	HEADER_MASTER_KEYDATA_OFFSET		256

#define TC_HEADER_OFFSET_MAGIC					64
#define TC_HEADER_OFFSET_VERSION				68
#define TC_HEADER_OFFSET_REQUIRED_VERSION		70
#define TC_HEADER_OFFSET_KEY_AREA_CRC			72
#define TC_HEADER_OFFSET_VOLUME_CREATION_TIME	76
#define TC_HEADER_OFFSET_MODIFICATION_TIME		84
#define TC_HEADER_OFFSET_HIDDEN_VOLUME_SIZE		92
#define TC_HEADER_OFFSET_VOLUME_SIZE			100
#define TC_HEADER_OFFSET_ENCRYPTED_AREA_START	108
#define TC_HEADER_OFFSET_ENCRYPTED_AREA_LENGTH	116
#define TC_HEADER_OFFSET_FLAGS					124
#define TC_HEADER_OFFSET_SECTOR_SIZE			128
#define TC_HEADER_OFFSET_HEADER_CRC				252

#define RIPEMD160_BLOCKSIZE		64
#define RIPEMD160_DIGESTSIZE	20
#define SHA512_BLOCKSIZE		128
#define SHA512_DIGESTSIZE		64
#define SHA1_BLOCKSIZE			64
#define SHA1_DIGESTSIZE			20
#define WHIRLPOOL_BLOCKSIZE		64
#define WHIRLPOOL_DIGESTSIZE	64


// Maxium length of scheduled key
#define AES_KS				(sizeof(aes_encrypt_ctx) + sizeof(aes_decrypt_ctx))
#define SERPENT_KS			(140 * 4)

#define MAX_EXPANDED_KEY	(AES_KS + SERPENT_KS + TWOFISH_KS)

// The mode of operation used for newly created volumes and first to try when mounting
#define FIRST_MODE_OF_OPERATION_ID		1

// The first PRF to try when mounting
#define FIRST_PRF_ID		1


typedef UINTN size_t;


#if 1
// Modes of operation
enum
{
	/* If you add/remove a mode, update the following: GetMaxPkcs5OutSize(), EAInitMode() */

	XTS = FIRST_MODE_OF_OPERATION_ID,
	MODE_ENUM_END_ID
};

// Cipher IDs
enum
{
	NONE = 0,
	AES,
	SERPENT,
	TWOFISH,
};

// Hash algorithms (pseudorandom functions).
enum
{
	RIPEMD160 = FIRST_PRF_ID,
	HASH_ENUM_END_ID
};

// The last mode of operation to try when mounting and also the number of implemented modes
#define LAST_MODE_OF_OPERATION		(MODE_ENUM_END_ID - 1)

#endif

typedef struct
{
	int Id;					// Cipher ID
	char *Name;				// Name
	int BlockSize;			// Block size (bytes)
	int KeySize;			// Key size (bytes)
	int KeyScheduleSize;	// Scheduled key size (bytes)
} Cipher;

typedef struct
{
	int Ciphers[4];			// Null terminated array of ciphers used by encryption algorithm
	int Modes[LAST_MODE_OF_OPERATION + 1];			// Null terminated array of modes of operation
	int FormatEnabled;
} EncryptionAlgorithm;

typedef struct
{
	int Id;					// Hash ID
	char *Name;				// Name
	BOOL Deprecated;
	BOOL SystemEncryption;	// Available for system encryption
} Hash;

typedef struct CRYPTO_INFO_t
{
	int ea;									/* Encryption algorithm ID */
	int mode;								/* Mode of operation (e.g., XTS) */
	unsigned __int8 ks[MAX_EXPANDED_KEY];	/* Primary key schedule (if it is a cascade, it conatins multiple concatenated keys) */
	unsigned __int8 ks2[MAX_EXPANDED_KEY];	/* Secondary key schedule (if cascade, multiple concatenated) for XTS mode. */

	BOOL hiddenVolume;						/* Indicates whether the volume is mounted/mountable as hidden volume */

	UINT64_STRUCT VolumeSize;				/* partition size in byte, taken from the volume header */

	UINT64_STRUCT EncryptedAreaStart;		/* in byte, relative to the start of the device (not the partition),
	 	 	 	 	 	 	 	 	 	 	 	 taken from the volume header */
	UINT64_STRUCT EncryptedAreaLength;		/* in byte, taken from the volume header */

	uint32 HeaderFlags;
} CRYPTO_INFO, *PCRYPTO_INFO;


int cs_update_volume_header(char *header, PCRYPTO_INFO cryptoInfo, Password *password);
int cs_read_volume_header(BOOL bBoot, char *header, Password *password,
		CRYPTO_INFO *retHeaderCryptoInfo);

void EncipherBlock(int cipher, void *data, void *ks);	/* used to handle volume header */
void DecipherBlock(int cipher, void *data, void *ks);	/* used to handle volume header */

void EncryptDataUnits (UINT8 *buf, const UINT64 *structUnitNo, UINT64 nbrUnits,
		int ea, int mode, UINT8 *ks, UINT8 *ks2);

void DecryptDataUnits (UINT8 *buf, const UINT64 *structUnitNo, UINT64 nbrUnits,
		int ea, int mode, UINT8 *ks, UINT8 *ks2);

#endif /* _CS_CRYPTO_H_ */
