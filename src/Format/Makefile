MKDIR_OUT_SETUPFILES = mkdir -p ../Setup/${OutDir}/SetupFiles

VCPostBuildEventTool: ../${OutDir}/SetupFiles/CipherShed-Format

../${OutDir}/SetupFiles/CipherShed-Format: ${OutDir}/CipherShedFormat
	${MKDIR_OUT_SETUPFILES}
	cp ${OutDir}/CipherShedFormat ../${OutDir}/SetupFiles/CipherShed-Format

.PHONY: VCPostBuildEventTool

