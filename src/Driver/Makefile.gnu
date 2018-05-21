ifneq (${ConfigurationName},)
	PROFILE = ${ConfigurationName}
else
	PROFILE = ${Configuration}
endif

ifeq "${PROFILE}" "Debug"
	PROFILE = Debug
	ARCHTARGETS = x86 x64
else ifeq "${PROFILE}" "Debug x86"
	PROFILE = Debug
	ARCHTARGETS = x86
else ifeq "${PROFILE}" "Debug x64"
	PROFILE = Debug
	ARCHTARGETS = x64
else ifeq "${PROFILE}" "Release"
	PROFILE = Release
	ARCHTARGETS = x86 x64
else ifeq "${PROFILE}" "Release x86"
	PROFILE = Release
	ARCHTARGETS = x86
else ifeq "${PROFILE}" "Release x64"
	PROFILE = Release
	ARCHTARGETS = x64
else
	PROFILE =
	ARCHTARGETS =
endif

#set the build driver switch profile (debug vs release)
ifeq "${PROFILE}" "Debug"
	BDCMDSWPROFILE=-debug
else
	BDCMDSWPROFILE=-release
endif

MountExecutableSrc = ../Mount/${PROFILE}/CipherShed
MountExecutableDst =
ifeq "$(filter x86,${ARCHTARGETS})" "x86"
	MountExecutableDst += obj_driver_debug/i386/CipherShed
endif
ifeq "$(filter x64,${ARCHTARGETS})" "x64"
	MountExecutableDst += obj_driver_debug/amd64/CipherShed
endif


BuildCommandLine: ${MountExecutableDst}
ifeq "$(filter x86,${ARCHTARGETS})" "x86"
	cmd /c BuildDriver.cmd -build ${BDCMDSWPROFILE} -x86 ../Common ../Crypto .
endif
ifeq "$(filter x64,${ARCHTARGETS})" "x64"
	cmd /c BuildDriver.cmd -build ${BDCMDSWPROFILE} -x64 ../Common ../Crypto .
endif

CleanCommandLine:
ifeq "$(filter x86,${ARCHTARGETS})" "x86"
	cmd /c BuildDriver.cmd -clean ${BDCMDSWPROFILE} -x86 ../Common ../Crypto .
endif
ifeq "$(filter x64,${ARCHTARGETS})" "x64"
	cmd /c BuildDriver.cmd -clean ${BDCMDSWPROFILE} -x64 ../Common ../Crypto .
endif

ReBuildCommandLine: ${MountExecutableDst}
ifeq "$(filter x86,${ARCHTARGETS})" "x86"
	cmd /c BuildDriver.cmd -rebuild ${BDCMDSWPROFILE} -x86 ../Common ../Crypto .
endif
ifeq "$(filter x64,${ARCHTARGETS})" "x64"
	cmd /c BuildDriver.cmd -rebuild ${BDCMDSWPROFILE} -x64 ../Common ../Crypto .
endif

obj_driver_debug/i386/CipherShed:
	mkdir -p obj_driver_debug/i386/
	cp ${MountExecutableSrc} obj_driver_debug/i386/

obj_driver_debug/amd64/CipherShed:
	mkdir -p obj_driver_debug/amd64/
	cp ${MountExecutableSrc} obj_driver_debug/amd64/

.PHONY: CleanCommandLine BuildCommandLine ReBuildCommandLine
