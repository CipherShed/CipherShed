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
HANDLE hDriver = INVALID_HANDLE_VALUE;
int CurrentOSMajor = 0;
int CurrentOSMinor = 0;
int CurrentOSServicePack = 0;
HINSTANCE hInst = NULL;
BOOL bPreserveTimestamp = TRUE;
wchar_t *lpszTitle = NULL;
int RemoveFakeDosName (char *lpszDiskFile, char *lpszDosDevice){return 0;}
BOOL GetPartitionInfo (const char *deviceName, PPARTITION_INFORMATION rpartInfo){return 0;}
int FakeDosNameForDevice (const char *lpszDiskFile, char *lpszDosDevice, char *lpszCFDevice, BOOL bNameOnly){return 0;}
void UserEnrichRandomPool (HWND hwndDlg){}

// secondary...
void HandleDriveNotReadyError(){}
BOOL CheckCapsLock (HWND hwnd, BOOL quiet){return 0;}
BOOL IsDiskError (DWORD error){return 0;}
BOOL KeyFilesEnable = FALSE;
HWND MainDlg = NULL;
OSVersionEnum nCurrentOS = WIN_UNKNOWN;


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

//Aescrypt.c
uint_32t t_fn[4][256];
uint_32t t_fl[4][256];
uint_32t t_in[4][256];
uint_32t t_il[4][256];
uint_32t t_im[4][256];
uint_32t t_rc[(5 * (16 / 4 - 2))];

//csstrinutil.cpp
void ToUNICODE (char *lpszText){}
void UpperCaseCopy (char *lpszDest, const char *lpszSource){}
std::string WideToSingleString (const std::wstring &wideString){return 0;}
std::wstring SingleStringToWide (const std::string &singleString){return 0;}

//fsutil.cpp
BOOL ResolveSymbolicLink (const wchar_t *symLinkName, PWSTR targetName){return 0;}

#endif
