# Sets some output file names for less repetition
DYNAMIC_PLATFORM=${LIB}/${DYNAMIC_LIBOUTPREFIX}Platform${DYNAMIC_LIBOUTSUFFIX}
STATIC_PLATFORM=${LIB}/${STATIC_LIBOUTPREFIX}Platform${STATIC_LIBOUTSUFFIX}

# Set compiler flags
${DYNAMIC_PLATFORM} ${STATIC_PLATFORM}: C_CXXFLAGS=${BASE_C_CXXFLAGS} -IPlatform
${DYNAMIC_PLATFORM} ${STATIC_PLATFORM}: CFLAGS=${BASE_CFLAGS}
${DYNAMIC_PLATFORM} ${STATIC_PLATFORM}: CXXFLAGS=${BASE_CXXFLAGS}
${DYNAMIC_PLATFORM} ${STATIC_PLATFORM}: ASFLAGS=${BASE_ASFLAGS}
${DYNAMIC_PLATFORM} ${STATIC_PLATFORM}: LIBS=${BASE_LIBS}

# List of needed object files (from list of source files)
PLATFORM_OBJS=
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Buffer.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Exception.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Event.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/FileCommon.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/MemoryStream.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Memory.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/PlatformTest.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Serializable.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Serializer.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/SerializerFactory.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/StringConverter.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/TextReader.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/Directory.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/File.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/FilesystemPath.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/Mutex.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/Pipe.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/Poller.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/Process.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/SyncEvent.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/SystemException.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/SystemInfo.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/SystemLog.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/Thread.o
PLATFORM_OBJS+=${BUILD_TMP}/Platform/Unix/Time.o
${DYNAMIC_PLATFORM} ${STATIC_PLATFORM}: OBJS=${PLATFORM_OBJS}

# Dynamic library target
${DYNAMIC_PLATFORM}: $${OBJS}
	$(DYNLIB_RECIPE)

# Static library target
${STATIC_PLATFORM}: $${OBJS}
# (Static libs handled by common.mk)
