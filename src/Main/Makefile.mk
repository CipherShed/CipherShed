# Sets some output file names for less repetition
MAIN=${BIN}/ciphershed${EXEOUTSUFFIX}

# Sets compiler flags
${MAIN}: C_CXXFLAGS=${BASE_C_CXXFLAGS} -IMain -I. $(shell pkg-config --cflags fuse)
${MAIN}: CFLAGS=${BASE_C_CXXFLAGS} $(shell wx-config --cflags)
${MAIN}: CXXFLAGS=${BASE_CXXFLAGS} $(shell wx-config --cxxflags)
${MAIN}: ASFLAGS=${BASE_ASFLAGS}
${MAIN}: LIBS=${BASE_LIBS} -L${LIB} -lCore -lVolume -lPlatform -lfusedrv $(shell wx-config --libs) $(shell pkg-config --libs fuse) -ldl

# List of needed object files (from list of source files)
MAIN_OBJS=
MAIN_OBJS+=${BUILD_TMP}/Main/Application.o
MAIN_OBJS+=${BUILD_TMP}/Main/CommandLineInterface.o
MAIN_OBJS+=${BUILD_TMP}/Main/FavoriteVolume.o
MAIN_OBJS+=${BUILD_TMP}/Main/LanguageStrings.o
MAIN_OBJS+=${BUILD_TMP}/Main/StringFormatter.o
MAIN_OBJS+=${BUILD_TMP}/Main/TextUserInterface.o
MAIN_OBJS+=${BUILD_TMP}/Main/UserInterface.o
MAIN_OBJS+=${BUILD_TMP}/Main/UserPreferences.o
MAIN_OBJS+=${BUILD_TMP}/Main/Xml.o
MAIN_OBJS+=${BUILD_TMP}/Main/Unix/Main.o
MAIN_OBJS+=${BUILD_TMP}/Main/Resources.o
ifndef NOGUI
MAIN_OBJS+=${BUILD_TMP}/Main/FatalErrorHandler.o
MAIN_OBJS+=${BUILD_TMP}/Main/GraphicUserInterface.o
MAIN_OBJS+=${BUILD_TMP}/Main/VolumeHistory.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/AboutDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/BenchmarkDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/ChangePasswordDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/DeviceSelectionDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/EncryptionOptionsWizardPage.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/EncryptionTestDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/FavoriteVolumesDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/Forms.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/InfoWizardPage.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/KeyfileGeneratorDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/KeyfilesDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/KeyfilesPanel.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/LegalNoticesDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/MainFrame.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/MountOptionsDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/NewSecurityTokenKeyfileDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/PreferencesDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/ProgressWizardPage.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/RandomPoolEnrichmentDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/SecurityTokenKeyfilesDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/SelectDirectoryWizardPage.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/VolumePasswordPanel.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/VolumePropertiesDialog.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/VolumeCreationProgressWizardPage.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/VolumeCreationWizard.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/VolumeFormatOptionsWizardPage.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/VolumeLocationWizardPage.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/VolumePasswordWizardPage.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/VolumeSizeWizardPage.o
MAIN_OBJS+=${BUILD_TMP}/Main/Forms/WizardFrame.o
endif
${MAIN}: OBJS=${MAIN_OBJS}

# List of resources that need to be generated before objects
MAIN_RESOURCES=
MAIN_RESOURCES+=${BUILD_TMP}/License.txt.h
MAIN_RESOURCES+=${BUILD_TMP}/Common/Language.xml.h
ifndef NOGUI
MAIN_RESOURCES+=${BUILD_TMP}/Common/Textual_logo_96dpi.bmp.h
MAIN_RESOURCES+=${BUILD_TMP}/Format/CipherShed_Wizard.bmp.h
MAIN_RESOURCES+=${BUILD_TMP}/Mount/Drive_icon_96dpi.bmp.h
MAIN_RESOURCES+=${BUILD_TMP}/Mount/Drive_icon_mask_96dpi.bmp.h
MAIN_RESOURCES+=${BUILD_TMP}/Mount/Logo_96dpi.bmp.h
endif
${MAIN_OBJS}: ${MAIN_RESOURCES}

# Build artifact target
${MAIN}: $${OBJS}
	$(EXE_RECIPE)
