MKDIR_OUT_SETUPFILES = mkdir -p ${OutDir}/SetupFiles

VCPostBuildEventTool: ${OutDir}/SetupFiles/License.txt ${OutDir}/SetupFiles/CipherShed\ User\ Guide.pdf ${OutDir}/SetupFiles/CipherShed-Setup

${OutDir}/SetupFiles/CipherShed-Setup: ${OutDir}/CipherShedSetup
	${MKDIR_OUT_SETUPFILES}
	cp ${OutDir}/CipherShedSetup ${OutDir}/SetupFiles/CipherShed-Setup

${OutDir}/SetupFiles/CipherShed\ User\ Guide.pdf: ../../doc/userdocs/guide/CipherShed-User-Guide.pdf
	${MKDIR_OUT_SETUPFILES}
	cp ../../doc/userdocs/guide/CipherShed-User-Guide.pdf ${OutDir}/SetupFiles/CipherShed\ User\ Guide.pdf

${OutDir}/SetupFiles/License.txt: ../License.txt
	${MKDIR_OUT_SETUPFILES}
	cp ../License.txt ${OutDir}/SetupFiles/

.PHONY: VCPostBuildEventTool

