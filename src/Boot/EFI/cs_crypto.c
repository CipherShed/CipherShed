/*  cs_crypto.c - CipherShed EFI boot loader
 *  collection of relevant crypto functions used by the EFI loader and the driver
 *
 *
 *
 */

#include <efi.h>
#include <efilib.h>
#include <efibind.h>

#include "cs_common.h"
#include "cs_debug.h"
#include "cs_crypto.h"
#include "cs_controller.h"
#include "../../Common/Tcdefs.h"
#include "../../Common/Password.h"

// Cipher configuration
static Cipher Ciphers[] =
{
//								Block Size	Key Size	Key Schedule Size
//	  ID		Name			(Bytes)		(Bytes)		(Bytes)
	{ AES,		"AES",			16,			32,			AES_KS				},
	{ SERPENT,	"Serpent",		16,			32,			140*4				},
	{ TWOFISH,	"Twofish",		16,			32,			TWOFISH_KS			},
	{ 0,		0,				0,			0,			0					}
};


// Encryption algorithm configuration
// The following modes have been deprecated (legacy): LRW, CBC, INNER_CBC, OUTER_CBC
static EncryptionAlgorithm EncryptionAlgorithms[] =
{
	//  Cipher(s)                     Modes						FormatEnabled

	// Encryption algorithms available for boot drive encryption
	{ { 0,						0 }, { 0, 0 },		0 },	// Must be all-zero
	{ { AES,					0 }, { XTS, 0 },	1 },
	{ { SERPENT,				0 }, { XTS, 0 },	1 },
	{ { TWOFISH,				0 }, { XTS, 0 },	1 },
	{ { TWOFISH, AES,			0 }, { XTS, 0 },	1 },
	{ { SERPENT, TWOFISH, AES,	0 }, { XTS, 0 },	1 },
	{ { AES, SERPENT,			0 }, { XTS, 0 },	1 },
	{ { AES, TWOFISH, SERPENT,	0 }, { XTS, 0 },	1 },
	{ { SERPENT, TWOFISH,		0 }, { XTS, 0 },	1 },
	{ { 0,						0 }, { 0, 0 },		0 },	// Must be all-zero

};

#if 0
// Hash algorithms
static Hash Hashes[] =
{	// ID			Name			Deprecated		System Encryption
	{ RIPEMD160,	"RIPEMD-160",	FALSE,			TRUE },
	{ 0, 0, 0 }
};
#endif


void EncipherBlock(int cipher, void *data, void *ks)
{
	switch (cipher)
	{
	case AES:			aes_encrypt (data, data, ks); break;
	case TWOFISH:		twofish_encrypt (ks, data, data); break;
	case SERPENT:		serpent_encrypt (data, data, ks); break;
	default:
		cs_exception(L"EncipherBlock(): Invalid algorithm (0x%x)\n", cipher);
		return;
	}
}

void DecipherBlock(int cipher, void *data, void *ks)
{
	switch (cipher)
	{
	case SERPENT:	serpent_decrypt(data, data, ks); break;
	case TWOFISH:	twofish_decrypt(ks, data, data); break;
	case AES:		aes_decrypt(data, data, (void *) ((char *) ks + sizeof(aes_encrypt_ctx))); break;
	default:
					cs_exception(L"DecipherBlock(): Invalid algorithm (0x%x)\n", cipher);
					return;
	}
}

static uint16 GetHeaderField16 (byte *header, int offset)
{
	return BE16 (*(uint16 *) (header + offset));
}


static uint32 GetHeaderField32 (byte *header, int offset)
{
	return BE32 (*(uint32 *) (header + offset));
}

static UINT64_STRUCT GetHeaderField64 (byte *header, int offset)
{
	UINT64_STRUCT uint64Struct;

#ifndef TC_NO_COMPILER_INT64
	uint64Struct.Value = BE64 (*(uint64 *) (header + offset));
#else
	uint64Struct.HighPart = BE32 (*(uint32 *) (header + offset));
	uint64Struct.LowPart = BE32 (*(uint32 *) (header + offset + 4));
#endif
	return uint64Struct;
}

// Encryption algorithms support
static int EAGetFirst ()
{
	return 1;
}

static int EAGetNext (int previousEA)
{
	int id = previousEA + 1;
	if (EncryptionAlgorithms[id].Ciphers[0] != 0) return id;
	return 0;
}

// Returns number of ciphers in EA
int EAGetCipherCount (int ea)
{
	int i = 0;
	while (EncryptionAlgorithms[ea].Ciphers[i++]);

	return i - 1;
}

static int EAGetFirstCipher (int ea)
{
	return EncryptionAlgorithms[ea].Ciphers[0];
}

static int EAGetNextCipher (int ea, int previousCipherId)
{
	int c, i = 0;
	while ((c = EncryptionAlgorithms[ea].Ciphers[i++]))
	{
		if (c == previousCipherId)
			return EncryptionAlgorithms[ea].Ciphers[i];
	}

	return 0;
}

static int EAGetPreviousCipher(int ea, int previousCipherId)
{
	int c, i = 0;

	if (EncryptionAlgorithms[ea].Ciphers[i++] == previousCipherId)
		return 0;

	while ((c = EncryptionAlgorithms[ea].Ciphers[i++]))
	{
		if (c == previousCipherId)
			return EncryptionAlgorithms[ea].Ciphers[i - 2];
	}

	return 0;
}

static int EAGetLastCipher (int ea)
{
	int c, i = 0;
	while ((c = EncryptionAlgorithms[ea].Ciphers[i++]));

	return EncryptionAlgorithms[ea].Ciphers[i - 2];
}

// Ciphers support
static Cipher *CipherGet (int id)
{
	int i;
	for (i = 0; Ciphers[i].Id != 0; i++)
		if (Ciphers[i].Id == id)
			return &Ciphers[i];

	return NULL;
}

static int CipherGetKeySize (int cipherId)
{
	return CipherGet (cipherId) -> KeySize;
}

static int CipherGetKeyScheduleSize (int cipherId)
{
	return CipherGet (cipherId) -> KeyScheduleSize;
}

// Returns sum of key schedule sizes of all ciphers of the EA
static int EAGetKeyScheduleSize (int ea)
{
	int i = EAGetFirstCipher(ea);
	int size = CipherGetKeyScheduleSize(i);

	while ((i = EAGetNextCipher(ea, i)))
	{
		size += CipherGetKeyScheduleSize(i);
	}

	return size;
}

// Returns sum of key sizes of all ciphers of the EA (in bytes)
static int EAGetKeySize (int ea)
{
	int i = EAGetFirstCipher (ea);
	int size = CipherGetKeySize (i);

	while ((i = EAGetNextCipher (ea, i)))
	{
		size += CipherGetKeySize (i);
	}

	return size;
}

int CipherGetBlockSize(int cipherId)
{
	return CipherGet (cipherId) -> BlockSize;
}

/* Return values: 0 = success, ERR_CIPHER_INIT_FAILURE (fatal), ERR_CIPHER_INIT_WEAK_KEY (non-fatal) */
int CipherInit (int cipher, unsigned char *key, unsigned __int8 *ks)
{
	int retVal = ERR_SUCCESS;

	switch (cipher)
	{
	case AES:
		aes_init();

		if (aes_encrypt_key256 (key, (aes_encrypt_ctx *) ks) != EXIT_SUCCESS)
			return ERR_CIPHER_INIT_FAILURE;

		if (aes_decrypt_key256 (key, (aes_decrypt_ctx *) (ks + sizeof(aes_encrypt_ctx))) != EXIT_SUCCESS)
			return ERR_CIPHER_INIT_FAILURE;

		break;

	case SERPENT:
		serpent_set_key (key, CipherGetKeySize(SERPENT) * 8, ks);
		break;

	case TWOFISH:
		twofish_set_key ((TwofishInstance *)ks, (const u4byte *)key, CipherGetKeySize(TWOFISH) * 8);
		break;

	default:
		// Unknown/wrong cipher ID
		return ERR_CIPHER_INIT_FAILURE;
	}

	return retVal;
}

// Return values: 0 = success, ERR_CIPHER_INIT_FAILURE (fatal), ERR_CIPHER_INIT_WEAK_KEY (non-fatal)
static int EAInit(int ea, unsigned char *key, unsigned __int8 *ks)
{
	int c, retVal = ERR_SUCCESS;
	if (ea == 0)
		return ERR_CIPHER_INIT_FAILURE;

	for (c = EAGetFirstCipher(ea); c != 0; c = EAGetNextCipher(ea, c))
	{
		switch (CipherInit(c, key, ks))
		{
		case ERR_CIPHER_INIT_FAILURE:
			return ERR_CIPHER_INIT_FAILURE;

		case ERR_CIPHER_INIT_WEAK_KEY:
			retVal = ERR_CIPHER_INIT_WEAK_KEY;		// Non-fatal error
			break;
		}

		key += CipherGetKeySize(c);
		ks += CipherGetKeyScheduleSize(c);
	}
	return retVal;
}

/*
 * 	\brief	decrypt the content of a given buffer
 *
 * 	This function decrypts the content of the given buffer with the given buffer size using
 * 	an algorithm and cipher data as provided by the argument. Only the XTS mode is supported.
 *
 *	\param	buf		data to be decrypted; the start of the buffer is assumed to be aligned
 *					with the start of a data unit.
 *	\param	len		number of bytes to decrypt; must be divisible by the block size
 *					(for cascaded ciphers, divisible by the largest block size used within the cascade)
 */
static void DecryptBuffer(unsigned __int8 *buf, TC_LARGEST_COMPILER_UINT len, PCRYPTO_INFO cryptoInfo)
{

	switch (cryptoInfo->mode)
	{
	case XTS:
		{
			unsigned __int8 *ks = cryptoInfo->ks + EAGetKeyScheduleSize (cryptoInfo->ea);
			unsigned __int8 *ks2 = cryptoInfo->ks2 + EAGetKeyScheduleSize (cryptoInfo->ea);
			UINT64_STRUCT dataUnitNo;
			int cipher;

			// When encrypting/decrypting a buffer (typically a volume header) the sequential number
			// of the first XTS data unit in the buffer is always 0 and the start of the buffer is
			// always assumed to be aligned with the start of the data unit 0.
			dataUnitNo.LowPart = 0;
			dataUnitNo.HighPart = 0;

			for (cipher = EAGetLastCipher(cryptoInfo->ea);
				cipher != 0;
				cipher = EAGetPreviousCipher(cryptoInfo->ea, cipher))
			{
				ks -= CipherGetKeyScheduleSize(cipher);
				ks2 -= CipherGetKeyScheduleSize(cipher);

				DecryptBufferXTS(buf, len, &dataUnitNo, 0, ks, ks2, cipher);
			}
		}
		break;

	default:
		// Unknown/wrong ID
		cs_exception(L"EncryptBuffer(): Invalid Crypto Mode (0x%x)\n", cryptoInfo->mode);
		return;
	}
}

/*
 * 	\brief	encrypt the content of a given buffer
 *
 * 	This function encrypts the content of the given buffer with the given buffer size using
 * 	an algorithm and cipher data as provided by the argument. Only the XTS mode is supported.
 *
 *	\param	buf		data to be encrypted; the start of the buffer is assumed to be aligned
 *					with the start of a data unit.
 *	\param	len		number of bytes to encrypt; must be divisible by the block size
 *					(for cascaded ciphers, divisible by the largest block size used within the cascade)
 */
void EncryptBuffer (unsigned __int8 *buf, TC_LARGEST_COMPILER_UINT len, PCRYPTO_INFO cryptoInfo)
{
	switch (cryptoInfo->mode)
	{
	case XTS:
		{
			unsigned __int8 *ks = cryptoInfo->ks;
			unsigned __int8 *ks2 = cryptoInfo->ks2;
			UINT64_STRUCT dataUnitNo;
			int cipher;

			// When encrypting/decrypting a buffer (typically a volume header) the sequential number
			// of the first XTS data unit in the buffer is always 0 and the start of the buffer is
			// always assumed to be aligned with the start of a data unit.
			dataUnitNo.LowPart = 0;
			dataUnitNo.HighPart = 0;

			for (cipher = EAGetFirstCipher(cryptoInfo->ea);
				cipher != 0;
				cipher = EAGetNextCipher(cryptoInfo->ea, cipher))
			{
				EncryptBufferXTS (buf, len, &dataUnitNo, 0, ks, ks2, cipher);

				ks += CipherGetKeyScheduleSize(cipher);
				ks2 += CipherGetKeyScheduleSize(cipher);
			}
		}
		break;

	default:
		// Unknown/wrong ID
		cs_exception(L"EncryptBuffer(): Invalid Crypto Mode (0x%x)\n", cryptoInfo->mode);
		return;
	}
}

/*
 * 	\brief	update the CRC values in the volume header
 *
 * 	This function calculates the CRC values in the volume header and updates its
 * 	values in the corresponding addresses in the volume header. The volume header must
 * 	be unencrypted!
 *
 *	\param	header		buffer containing the 512 byte volume header
 *
 */
static void cs_update_volume_header_crc(IN OUT char *header) {
	ASSERT(header != NULL);

	(*(uint32 *)(header + TC_HEADER_OFFSET_KEY_AREA_CRC)) =
			BE32(GetCrc32((unsigned char *)header + HEADER_MASTER_KEYDATA_OFFSET, MASTER_KEYDATA_SIZE));
	(*(uint32 *)(header + TC_HEADER_OFFSET_HEADER_CRC)) =
			BE32(GetCrc32((unsigned char *)header + TC_HEADER_OFFSET_MAGIC,
					TC_HEADER_OFFSET_HEADER_CRC - TC_HEADER_OFFSET_MAGIC));
}

/*
 * 	\brief	update the volume header with disk encryption data or new password
 *
 * 	This function first decrypts the given volume header. If the volume size of the
 * 	given crypto data is > 0, then the volume EncryptedAreaStart and EncryptedAreaLength
 * 	is updated with the new values. If password is given, then the decrypted volume
 * 	header is encrypted with the new password, otherwise the volume header is encrypted
 * 	with the old password.
 *
 *	\param	header		buffer containing the 512 byte volume header
 *	\param	cryptoInfo	data to be updated in volume header
 *	\param	password	pointer to the user password
 *
 *	\return		the success state of the function
 */
int cs_update_volume_header(IN OUT char *header, IN PCRYPTO_INFO cryptoInfo, OPTIONAL Password *password) {

	ASSERT(header != NULL);
	ASSERT(cryptoInfo != NULL);

	DecryptBuffer((unsigned __int8 *)header + HEADER_ENCRYPTED_DATA_OFFSET,
			HEADER_ENCRYPTED_DATA_SIZE, cryptoInfo);

	if (GetHeaderField32((byte *)header, TC_HEADER_OFFSET_MAGIC) != 0x54525545
		|| (GetHeaderField16((byte *)header, TC_HEADER_OFFSET_VERSION) >= 4 && GetHeaderField32((byte *)header, TC_HEADER_OFFSET_HEADER_CRC) != GetCrc32((byte *)header + TC_HEADER_OFFSET_MAGIC, TC_HEADER_OFFSET_HEADER_CRC - TC_HEADER_OFFSET_MAGIC))
		|| GetHeaderField32((byte *)header, TC_HEADER_OFFSET_KEY_AREA_CRC) != GetCrc32((unsigned char *)header + HEADER_MASTER_KEYDATA_OFFSET, MASTER_KEYDATA_SIZE))
	{
		EncryptBuffer((unsigned __int8 *)header + HEADER_ENCRYPTED_DATA_OFFSET, HEADER_ENCRYPTED_DATA_SIZE, cryptoInfo);
		CS_DEBUG((D_ERROR, L"unable to update volume header: decryption failed\n"));
		return ERR_PASSWORD_WRONG;
	}
	CS_DEBUG((D_INFO, L"cs_update_volume_header(): decryption Ok, passwd: 0x%x\n", password));

	/* if the password need to be changed... */
	if (password) {
		char dk[32 * 2 * 3];
		int status;

		derive_key_ripemd160((char *)password->Text, (int) password->Length, header + HEADER_SALT_OFFSET,
			PKCS5_SALT_SIZE, 1000, dk, sizeof (dk));

		status = EAInit(cryptoInfo->ea, (unsigned char *)dk, cryptoInfo->ks);
		if (status == ERR_CIPHER_INIT_FAILURE) {
			SetMem(&dk, 0, sizeof(dk));
			EncryptBuffer((unsigned __int8 *)header + HEADER_ENCRYPTED_DATA_OFFSET, HEADER_ENCRYPTED_DATA_SIZE, cryptoInfo);
			return status;
		}
		EAInit(cryptoInfo->ea, (unsigned char *)dk + EAGetKeySize (cryptoInfo->ea), cryptoInfo->ks2);
	}

	/* update EncryptedAreaStart and EncryptedAreaLength */
	if (cryptoInfo->VolumeSize.Value > 0) {
#ifndef TC_NO_COMPILER_INT64
		(*(uint64 *)(header + TC_HEADER_OFFSET_ENCRYPTED_AREA_START)) = BE64(cryptoInfo->EncryptedAreaStart.Value);
		(*(uint64 *)(header + TC_HEADER_OFFSET_ENCRYPTED_AREA_LENGTH)) = BE64(cryptoInfo->EncryptedAreaLength.Value);
		(*(uint64 *)(header + TC_HEADER_OFFSET_VOLUME_SIZE)) = BE64(cryptoInfo->VolumeSize.Value);
#else
		(*(uint32 *)(header + TC_HEADER_OFFSET_ENCRYPTED_AREA_START)) = BE32(cryptoInfo->EncryptedAreaStart.HighPart);
		(*(uint32 *)(header + 4 + TC_HEADER_OFFSET_ENCRYPTED_AREA_START)) = BE32(cryptoInfo->EncryptedAreaStart.LowPart);
		(*(uint32 *)(header + TC_HEADER_OFFSET_ENCRYPTED_AREA_LENGTH)) = BE32(cryptoInfo->EncryptedAreaLength.HighPart);
		(*(uint32 *)(header + 4 + TC_HEADER_OFFSET_ENCRYPTED_AREA_LENGTH)) = BE32(cryptoInfo->EncryptedAreaLength.LowPart);
		(*(uint32 *)(header + TC_HEADER_OFFSET_VOLUME_SIZE)) = BE32(cryptoInfo->VolumeSize.HighPart);
		(*(uint32 *)(header + 4 + TC_HEADER_OFFSET_VOLUME_SIZE)) = BE32(cryptoInfo->VolumeSize.LowPart);

		cs_update_volume_header_crc(header);
#endif
	}

	EncryptBuffer((unsigned __int8 *)header + HEADER_ENCRYPTED_DATA_OFFSET, HEADER_ENCRYPTED_DATA_SIZE, cryptoInfo);

	return ERR_SUCCESS;
}

/*
 * 	\brief	decrypt the volume header with the given user password
 *
 * 	this function tries to decrypt the volume header using the given user password.
 * 	In case of success, several fields are read out from the decrypted volume header
 * 	and stored in the given data structure.
 * 	When finished with that, the volume header is encrypted again and the cipher context
 * 	for the media encryption/decryption is initialized using the master key from the
 * 	volume header.
 * 	This function corresponds to the ReadVolumeHeader() function of TrueCrypt.
 *
 *	\param	bBoot		flag that indicates whether the header belongs to a hidden volume (FALSE) or not (TRUE)
 *	\param	header		buffer containing the 512 byte volume header
 *	\param	password	pointer to the user password
 *	\param	retHeaderCryptoInfo	pointer to buffer for the resulting data from the volume header
 *
 *	\return		the success state of the function (EFI_SUCCESS or EFI_ACCESS_DENIED)
 */
int cs_read_volume_header(IN BOOL bBoot, IN char *header, IN Password *password,
		OUT CRYPTO_INFO *retHeaderCryptoInfo) {

	char dk[32 * 2 * 3];		// 6 * 256-bit key
	char masterKey[32 * 2 * 3];

	PCRYPTO_INFO cryptoInfo;
	int status;
#ifdef EFI_DEBUG
	UINT8 dbg_header[32];

	ASSERT(header != NULL);
	CopyMem(&dbg_header[0], header, sizeof(dbg_header));
#endif
	ASSERT(password != NULL);
	ASSERT(retHeaderCryptoInfo != NULL);

	cryptoInfo = retHeaderCryptoInfo;
	SetMem(cryptoInfo, sizeof (CRYPTO_INFO), 0);
	cryptoInfo->ea = -1;

	// PKCS5 PRF
	derive_key_ripemd160((char *)password->Text, (int) password->Length, header + HEADER_SALT_OFFSET,
		PKCS5_SALT_SIZE, bBoot ? 1000 : 2000, dk, sizeof (dk));

	// Mode of operation
	cryptoInfo->mode = FIRST_MODE_OF_OPERATION_ID;

	// Test all available encryption algorithms
	for (cryptoInfo->ea = EAGetFirst(); cryptoInfo->ea != 0; cryptoInfo->ea = EAGetNext(cryptoInfo->ea))
	{
		status = EAInit(cryptoInfo->ea, (unsigned char *)dk, cryptoInfo->ks);
		if (status == ERR_CIPHER_INIT_FAILURE)
			goto ret;

		// Secondary key schedule
		EAInit(cryptoInfo->ea, (unsigned char *)dk + EAGetKeySize (cryptoInfo->ea), cryptoInfo->ks2);

		// Try to decrypt header
		DecryptBuffer((unsigned __int8 *)header + HEADER_ENCRYPTED_DATA_OFFSET,
				HEADER_ENCRYPTED_DATA_SIZE, cryptoInfo);

		// Check magic 'TRUE' and CRC-32 of header fields and master keydata
		if (GetHeaderField32((byte *)header, TC_HEADER_OFFSET_MAGIC) != 0x54525545
			|| (GetHeaderField16((byte *)header, TC_HEADER_OFFSET_VERSION) >= 4 && GetHeaderField32((byte *)header, TC_HEADER_OFFSET_HEADER_CRC) != GetCrc32((byte *)header + TC_HEADER_OFFSET_MAGIC, TC_HEADER_OFFSET_HEADER_CRC - TC_HEADER_OFFSET_MAGIC))
			|| GetHeaderField32((byte *)header, TC_HEADER_OFFSET_KEY_AREA_CRC) != GetCrc32((unsigned char *)header + HEADER_MASTER_KEYDATA_OFFSET, MASTER_KEYDATA_SIZE))
		{
			EncryptBuffer((unsigned __int8 *)header + HEADER_ENCRYPTED_DATA_OFFSET, HEADER_ENCRYPTED_DATA_SIZE, cryptoInfo);
			ASSERT(CompareMem(&dbg_header[0], header, sizeof(dbg_header)) == 0);
			continue;
		}

#if 1
		Print(L"## header decrypted (ea = 0x%x)!! \n", cryptoInfo->ea);
		DumpHex(2, 0, 128, header);
#endif

		// Header decrypted
		status = 0;

		// Hidden volume status
		cryptoInfo->VolumeSize = GetHeaderField64((byte *)header, TC_HEADER_OFFSET_HIDDEN_VOLUME_SIZE);
		cryptoInfo->hiddenVolume = (cryptoInfo->VolumeSize.LowPart != 0 || cryptoInfo->VolumeSize.HighPart != 0);

		// Volume size
		cryptoInfo->VolumeSize = GetHeaderField64((byte *)header, TC_HEADER_OFFSET_VOLUME_SIZE);

		// Encrypted area size and length
		cryptoInfo->EncryptedAreaStart = GetHeaderField64((byte *)header, TC_HEADER_OFFSET_ENCRYPTED_AREA_START);
		cryptoInfo->EncryptedAreaLength = GetHeaderField64((byte *)header, TC_HEADER_OFFSET_ENCRYPTED_AREA_LENGTH);

		// Flags
		cryptoInfo->HeaderFlags = GetHeaderField32((byte *)header, TC_HEADER_OFFSET_FLAGS);

		CS_DEBUG((D_INFO, L"VolumeSize 0x%x, EncryptedAreaStart 0x%x, EncryptedAreaLength 0x%x\n",
				cryptoInfo->VolumeSize.Value, cryptoInfo->EncryptedAreaStart.Value,
				cryptoInfo->EncryptedAreaLength.Value));
		CS_DEBUG((D_INFO, L"hiddenVolumePresent %x, Flags 0x%x, Algorithm 0x%x, Mode 0x%x\n",
				cryptoInfo->hiddenVolume, cryptoInfo->HeaderFlags, cryptoInfo->ea, cryptoInfo->mode));

		CopyMem(masterKey, header + HEADER_MASTER_KEYDATA_OFFSET, sizeof (masterKey));
		/* encrypte the buffer again when finished with the data acquisition */
		EncryptBuffer((unsigned __int8 *)header + HEADER_ENCRYPTED_DATA_OFFSET, HEADER_ENCRYPTED_DATA_SIZE, cryptoInfo);

		/* save cipher information for later usage */
		set_volume_header_cipher(cryptoInfo);

		/* Init the encryption algorithm with the decrypted master key */
		status = EAInit(cryptoInfo->ea, (unsigned char *)masterKey, cryptoInfo->ks);
		if (status == ERR_CIPHER_INIT_FAILURE)
			goto ret;

		/* The secondary master key (if cascade, multiple concatenated) */
		EAInit(cryptoInfo->ea, (unsigned char *)masterKey + EAGetKeySize (cryptoInfo->ea), cryptoInfo->ks2);
		goto ret;
	}

	status = ERR_PASSWORD_WRONG;

ret:
	SetMem(dk, sizeof(dk), 0);
	SetMem(masterKey, sizeof(masterKey), 0);

	return status;
}

/*
 * 	\brief	encrypt a given data unit (means: a sector from a media)
 *
 * 	This function performs the sector encryption of the EFI driver. In TrueCrypt this
 * 	function is called in BootEncryptedio.cpp:
 * 	   EncryptDataUnits (SectorBuffer, &dataUnitNo, 1, BootCryptoInfo);
 *
 *	\param	buf		pointer to the buffer containing the data, used for the result data as well
 *	\param	structUnitNo	the data unit number (sector number) of the data to be encrypted;
 *							this is needed for the XTS cipher mode
 *	\param	nbrUnits	number of data units in the buffer to be encrypted
 *	\param	ea		ID of the cipher algorithm to use
 *	\param	mode	mode of the cipher algorithm to use (only XTS is supported)
 *	\param	ks, ks2	the cipher context to use
 */
void EncryptDataUnits (IN OUT UINT8 *buf, IN const UINT64 *structUnitNo, IN UINT64 nbrUnits,
		IN int ea, IN int mode, IN UINT8 *ks, IN UINT8 *ks2)
{
	int cipher;

#ifndef TC_NO_COMPILER_INT64
	void *iv = ci->k2;									// Deprecated/legacy
	unsigned __int64 unitNo = structUnitNo->Value;
	unsigned __int64 *iv64 = (unsigned __int64 *) iv;	// Deprecated/legacy
	unsigned __int32 sectorIV[4];						// Deprecated/legacy
	unsigned __int32 secWhitening[2];					// Deprecated/legacy
#endif

	switch (mode)
	{
	case XTS:
		for (cipher = EAGetFirstCipher(ea); cipher != 0; cipher = EAGetNextCipher(ea, cipher))
		{
			EncryptBufferXTS(buf, nbrUnits * ENCRYPTION_DATA_UNIT_SIZE, (const UINT64_STRUCT *)structUnitNo,
					0, ks, ks2, cipher);

			ks += CipherGetKeyScheduleSize(cipher);
			ks2 += CipherGetKeyScheduleSize(cipher);
		}
		break;


	default:
		// Unknown/wrong ID
		cs_exception(L"EncryptDataUnits: wrong cipher mode: 0x%x\n", mode);
		break;
	}
}

/* sector decryption used by the driver:
 * in TrueCrypt this is called in BootEncryptedio.cpp:
 *   -> DecryptDataUnits (SectorBuffer, &sector, 1, BootCryptoInfo) */

/*
 * 	\brief	decrypt a given data unit (means: a sector from a media)
 *
 * 	This function performs the sector decryption of the EFI driver. In TrueCrypt this
 * 	function is called in BootEncryptedio.cpp:
 * 	   DecryptDataUnits(SectorBuffer, &sector, 1, BootCryptoInfo);
 *
 *	\param	buf		pointer to the buffer containing the data, used for the result data as well
 *	\param	structUnitNo	the data unit number (sector number) of the data to be decrypted;
 *							this is needed for the XTS cipher mode
 *	\param	nbrUnits	number of data units in the buffer to be decrypted
 *	\param	ea		ID of the cipher algorithm to use
 *	\param	mode	mode of the cipher algorithm to use (only XTS is supported)
 *	\param	ks, ks2	the cipher context to use
 */
void DecryptDataUnits (IN OUT UINT8 *buf, IN const UINT64 *structUnitNo, IN UINT64 nbrUnits,
		IN int ea, IN int mode, IN UINT8 *ks, IN UINT8 *ks2)
{
	int cipher;

#ifndef TC_NO_COMPILER_INT64
	void *iv = ci->k2;									// Deprecated/legacy
	unsigned __int64 unitNo = structUnitNo->Value;
	unsigned __int64 *iv64 = (unsigned __int64 *) iv;	// Deprecated/legacy
	unsigned __int32 sectorIV[4];						// Deprecated/legacy
	unsigned __int32 secWhitening[2];					// Deprecated/legacy
#endif	// #ifndef TC_NO_COMPILER_INT64

	switch (mode)
	{
	case XTS:
		ks += EAGetKeyScheduleSize (ea);
		ks2 += EAGetKeyScheduleSize (ea);

		for (cipher = EAGetLastCipher(ea); cipher != 0; cipher = EAGetPreviousCipher(ea, cipher))
		{
			ks -= CipherGetKeyScheduleSize (cipher);
			ks2 -= CipherGetKeyScheduleSize (cipher);

			DecryptBufferXTS(buf, nbrUnits * ENCRYPTION_DATA_UNIT_SIZE, (const UINT64_STRUCT *)structUnitNo,
					0, ks, ks2, cipher);
		}
		break;

	default:
		// Unknown/wrong ID
		cs_exception(L"DecryptDataUnits: wrong cipher mode: 0x%x\n", mode);
		break;
	}
}
