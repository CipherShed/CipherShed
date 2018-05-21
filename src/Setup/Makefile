TARGETDIR = ../${OutDir}/SetupFiles
MKDIR_OUT_SETUPFILES = mkdir -p ${TARGETDIR}

VCPostBuildEventTool: ${TARGETDIR}/License.txt ${TARGETDIR}/CipherShed\ User\ Guide.pdf ${TARGETDIR}/CipherShed-Setup

${TARGETDIR}/CipherShed-Setup: ${OutDir}/CipherShedSetup
	${MKDIR_OUT_SETUPFILES}
	cp ${OutDir}/CipherShedSetup ${TARGETDIR}/CipherShed-Setup

${TARGETDIR}/CipherShed\ User\ Guide.pdf: ../../doc/userdocs/guide/CipherShed-User-Guide.pdf
	${MKDIR_OUT_SETUPFILES}
	cp ../../doc/userdocs/guide/CipherShed-User-Guide.pdf ${TARGETDIR}/CipherShed\ User\ Guide.pdf

${TARGETDIR}/License.txt: ../License.txt
	${MKDIR_OUT_SETUPFILES}
	cp ../License.txt ${TARGETDIR}/

.PHONY: VCPostBuildEventTool

