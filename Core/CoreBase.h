/*
 Copyright (c) 2008-2010 TrueCrypt Developers Association. All rights reserved.

 Governed by the TrueCrypt License 3.0 the full text of which is contained in
 the file License.txt included in TrueCrypt binary and source code distribution
 packages.
*/

#ifndef TC_HEADER_Core_CoreBase
#define TC_HEADER_Core_CoreBase

#include "Platform/Platform.h"
#include "Platform/Functor.h"
#include "Platform/User.h"
#include "Common/Crypto.h"
#include "Volume/Keyfile.h"
#include "Volume/VolumeInfo.h"
#include "Volume/Volume.h"
#include "Volume/VolumePassword.h"
#include "CoreException.h"
#include "HostDevice.h"
#include "MountOptions.h"

namespace CipherShed
{
	class CoreBase
	{
	public:
		virtual ~CoreBase ();

		virtual void ChangePassword (shared_ptr <Volume> openVolume, shared_ptr <VolumePassword> newPassword, shared_ptr <KeyfileList> newKeyfiles, shared_ptr <Pkcs5Kdf> newPkcs5Kdf = shared_ptr <Pkcs5Kdf> ()) const;
		virtual void ChangePassword (shared_ptr <VolumePath> volumePath, bool preserveTimestamps, shared_ptr <VolumePassword> password, shared_ptr <KeyfileList> keyfiles, shared_ptr <VolumePassword> newPassword, shared_ptr <KeyfileList> newKeyfiles, shared_ptr <Pkcs5Kdf> newPkcs5Kdf = shared_ptr <Pkcs5Kdf> ()) const;
		virtual void CheckFilesystem (shared_ptr <VolumeInfo> mountedVolume, bool repair = false) const = 0; 
		virtual void CoalesceSlotNumberAndMountPoint (MountOptions &options) const;
		virtual void CreateKeyfile (const FilePath &keyfilePath) const;
		virtual void DismountFilesystem (const DirectoryPath &mountPoint, bool force) const = 0;
		virtual shared_ptr <VolumeInfo> DismountVolume (shared_ptr <VolumeInfo> mountedVolume, bool ignoreOpenFiles = false, bool syncVolumeInfo = false) = 0;
		virtual bool FilesystemSupportsLargeFiles (const FilePath &filePath) const = 0;
		virtual DirectoryPath GetDeviceMountPoint (const DevicePath &devicePath) const = 0;
		virtual uint32 GetDeviceSectorSize (const DevicePath &devicePath) const = 0;
		virtual uint64 GetDeviceSize (const DevicePath &devicePath) const = 0;
		virtual VolumeSlotNumber GetFirstFreeSlotNumber (VolumeSlotNumber startFrom = 0) const;
		virtual VolumeSlotNumber GetFirstSlotNumber () const { return 1; }
		virtual VolumeSlotNumber GetLastSlotNumber () const { return 64; }
		virtual HostDeviceList GetHostDevices (bool pathListOnly = false) const = 0;
		virtual FilePath GetApplicationExecutablePath () const { return ApplicationExecutablePath; }
		virtual uint64 GetMaxHiddenVolumeSize (shared_ptr <Volume> outerVolume) const;
		virtual int GetOSMajorVersion () const = 0;
		virtual int GetOSMinorVersion () const = 0;
		virtual shared_ptr <VolumeInfo> GetMountedVolume (const VolumePath &volumePath) const;
		virtual shared_ptr <VolumeInfo> GetMountedVolume (VolumeSlotNumber slot) const;
		virtual VolumeInfoList GetMountedVolumes (const VolumePath &volumePath = VolumePath()) const = 0;
		virtual bool HasAdminPrivileges () const = 0;
		virtual void Init () { }
		virtual bool IsDeviceChangeInProgress () const { return DeviceChangeInProgress; }
		virtual bool IsDevicePresent (const DevicePath &device) const = 0;
		virtual bool IsInPortableMode () const = 0;
		virtual bool IsMountPointAvailable (const DirectoryPath &mountPoint) const = 0;
		virtual bool IsOSVersion (int major, int minor) const = 0;
		virtual bool IsOSVersionLower (int major, int minor) const = 0;
		virtual bool IsPasswordCacheEmpty () const = 0;
		virtual bool IsSlotNumberAvailable (VolumeSlotNumber slotNumber) const;
		virtual bool IsSlotNumberValid (VolumeSlotNumber slotNumber) const { return slotNumber >= GetFirstSlotNumber() && slotNumber <= GetLastSlotNumber(); }
		virtual bool IsVolumeMounted (const VolumePath &volumePath) const;
		virtual VolumeSlotNumber MountPointToSlotNumber (const DirectoryPath &mountPoint) const = 0;
		virtual shared_ptr <VolumeInfo> MountVolume (MountOptions &options) = 0;
		virtual shared_ptr <Volume> OpenVolume (shared_ptr <VolumePath> volumePath, bool preserveTimestamps, shared_ptr <VolumePassword> password, shared_ptr <KeyfileList> keyfiles, VolumeProtection::Enum protection = VolumeProtection::None, shared_ptr <VolumePassword> protectionPassword = shared_ptr <VolumePassword> (), shared_ptr <KeyfileList> protectionKeyfiles = shared_ptr <KeyfileList> (), bool sharedAccessAllowed = false, VolumeType::Enum volumeType = VolumeType::Unknown, bool useBackupHeaders = false, bool partitionInSystemEncryptionScope = false) const;
		virtual void RandomizeEncryptionAlgorithmKey (shared_ptr <EncryptionAlgorithm> encryptionAlgorithm) const;
		virtual void ReEncryptVolumeHeaderWithNewSalt (const BufferPtr &newHeaderBuffer, shared_ptr <VolumeHeader> header, shared_ptr <VolumePassword> password, shared_ptr <KeyfileList> keyfiles) const;
		virtual void SetAdminPasswordCallback (shared_ptr <GetStringFunctor> functor) { }
		virtual void SetApplicationExecutablePath (const FilePath &path) { ApplicationExecutablePath = path; }
		virtual void SetFileOwner (const FilesystemPath &path, const UserId &owner) const = 0;
		virtual DirectoryPath SlotNumberToMountPoint (VolumeSlotNumber slotNumber) const = 0;
		virtual void WipePasswordCache () const = 0;

		Event VolumeDismountedEvent;
		Event VolumeMountedEvent;
		Event WarningEvent;

	protected:
		CoreBase ();

		static const int SecureWipePassCount = PRAND_DISK_WIPE_PASSES;
		bool DeviceChangeInProgress;
		FilePath ApplicationExecutablePath;

	private:
		CoreBase (const CoreBase &);
		CoreBase &operator= (const CoreBase &);
	};

	struct VolumeEventArgs : EventArgs
	{
		VolumeEventArgs (shared_ptr <VolumeInfo> volume) : mVolume (volume) { }
		shared_ptr <VolumeInfo> mVolume;
	};
}

#endif // TC_HEADER_Core_CoreBase
