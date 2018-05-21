MKDIR_OUT_SETUPFILES = mkdir -p ../Setup/${OutDir}/SetupFiles

VCPostBuildEventTool: ../${OutDir}/SetupFiles/CipherShed

../${OutDir}/SetupFiles/CipherShed: ${OutDir}/CipherShed
	${MKDIR_OUT_SETUPFILES}
	cp ${OutDir}/CipherShed ../${OutDir}/SetupFiles/CipherShed

.PHONY: VCPostBuildEventTool

