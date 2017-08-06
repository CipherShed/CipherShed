# Sets some output file names for less repetition
DYNAMIC_CORE=${LIB}/${DYNAMIC_LIBOUTPREFIX}Core${DYNAMIC_LIBOUTSUFFIX}
STATIC_CORE=${LIB}/${STATIC_LIBOUTPREFIX}Core${STATIC_LIBOUTSUFFIX}

# Sets compiler flags
${DYNAMIC_CORE} ${STATIC_CORE}: C_CXXFLAGS=${BASE_C_CXXFLAGS} -ICore
${DYNAMIC_CORE} ${STATIC_CORE}: CFLAGS=${BASE_CFLAGS}
${DYNAMIC_CORE} ${STATIC_CORE}: CXXFLAGS=${BASE_CXXFLAGS}
${DYNAMIC_CORE} ${STATIC_CORE}: ASFLAGS=${BASE_ASFLAGS}
${DYNAMIC_CORE} ${STATIC_CORE}: LIBS=${BASE_LIBS} -L${LIB} -lfusedrv

# List of needed object files (from list of source files)
CORE_OBJS=
CORE_OBJS+=${BUILD_TMP}/Core/CoreBase.o
CORE_OBJS+=${BUILD_TMP}/Core/CoreException.o
CORE_OBJS+=${BUILD_TMP}/Core/FatFormatter.o
CORE_OBJS+=${BUILD_TMP}/Core/HostDevice.o
CORE_OBJS+=${BUILD_TMP}/Core/MountOptions.o
CORE_OBJS+=${BUILD_TMP}/Core/RandomNumberGenerator.o
CORE_OBJS+=${BUILD_TMP}/Core/VolumeCreator.o
CORE_OBJS+=${BUILD_TMP}/Core/Unix/CoreService.o
CORE_OBJS+=${BUILD_TMP}/Core/Unix/CoreServiceRequest.o
CORE_OBJS+=${BUILD_TMP}/Core/Unix/CoreServiceResponse.o
CORE_OBJS+=${BUILD_TMP}/Core/Unix/CoreUnix.o
ifeq "${TARGET_PLATFORM}" "linux"
CORE_OBJS+=${BUILD_TMP}/Core/Unix/Linux/CoreLinux.o
endif
ifeq "${TARGET_PLATFORM}" "mac"
CORE_OBJS+=${BUILD_TEMP}/Core/Unix/FreeBSD/CoreFreeBSD.o
endif
${DYNAMIC_CORE} ${STATIC_CORE}: OBJS=${CORE_OBJS}

# Dynamic library target
${DYNAMIC_CORE}: $${OBJS}
	$(DYNLIB_RECIPE)

# Static library target
${STATIC_CORE}: $${OBJS}
# (Static libs handled by common.mk)
