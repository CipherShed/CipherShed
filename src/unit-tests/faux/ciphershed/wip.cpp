#include "../../../Common/Crypto.h"

#include "../../../Common/Password.h"
#include "../../../Common/Dlgcode.h"
#include "../../../Common/Language.h"
#include "../../../Common/Pkcs5.h"
#include "../../../Common/Endian.h"
#include "../../../Common/Random.h"

#if defined(_MSC_VER) || defined(__CYGWIN__)
#include <io.h>
#endif

#ifdef CS_UNITTESTING
//Dlgcode.c
BOOL bPreserveTimestamp = TRUE;
BOOL UacElevated = FALSE;
wchar_t *lpszTitle = NULL;
void WaitCursor (){}
BOOL IsUacSupported (){return 0;}
void EnableElevatedCursorChange (HWND parent){}
void CreateFullVolumePath (char *lpszDiskFile, const char *lpszFileName, BOOL * bDevice){}
void NormalCursor (){}
void handleError (HWND hwndDlg, int code){}
int RemoveFakeDosName (char *lpszDiskFile, char *lpszDosDevice){return 0;}
BOOL GetPartitionInfo (const char *deviceName, PPARTITION_INFORMATION rpartInfo){return 0;}
int FakeDosNameForDevice (const char *lpszDiskFile, char *lpszDosDevice, char *lpszCFDevice, BOOL bNameOnly){return 0;}
void UserEnrichRandomPool (HWND hwndDlg){}
//Volumes.c
BOOL ReadEffectiveVolumeHeader (BOOL device, HANDLE fileHandle, byte *header, DWORD *bytesRead){return 0;}
int ReadVolumeHeader (BOOL bBoot, char *encryptedHeader, Password *password, PCRYPTO_INFO *retInfo, CRYPTO_INFO *retHeaderCryptoInfo){return 0;}
BOOL WriteEffectiveVolumeHeader (BOOL device, HANDLE fileHandle, byte *header){return 0;}
int CreateVolumeHeaderInMemory (BOOL bBoot, char *header, int ea, int mode, Password *password, int pkcs5_prf, char *masterKeydata, PCRYPTO_INFO *retInfo, unsigned __int64 volumeSize, unsigned __int64 hiddenVolumeSize, unsigned __int64 encryptedAreaStart, unsigned __int64 encryptedAreaLength, uint16 requiredProgramVersion, uint32 headerFlags, uint32 sectorSize, BOOL bWipeMode){return 0;}
int WriteRandomDataToReservedHeaderAreas (HANDLE dev, CRYPTO_INFO *cryptoInfo, uint64 dataAreaSize, BOOL bPrimaryOnly, BOOL bBackupOnly){return 0;}
//Language.c
wchar_t *GetString (const char *stringId){return 0;}
//Random.c
int Randinit(){return 0;}
void RandSetHashFunction (int hash_algo_id){}
void RandStop (BOOL freePool){}
//Crypto.c
void crypto_close (PCRYPTO_INFO cryptoInfo){}
#endif
