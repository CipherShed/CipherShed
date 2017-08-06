# Sets some output file names for less repetition
DYNAMIC_VOLUME=${LIB}/${DYNAMIC_LIBOUTPREFIX}Volume${DYNAMIC_LIBOUTSUFFIX}
STATIC_VOLUME=${LIB}/${STATIC_LIBOUTPREFIX}Volume${STATIC_LIBOUTSUFFIX}

# Sets compiler flags
${DYNAMIC_VOLUME} ${STATIC_VOLUME}: C_CXXFLAGS=${BASE_C_CXXFLAGS} -IVolume
${DYNAMIC_VOLUME} ${STATIC_VOLUME}: CFLAGS=${BASE_CFLAGS}
${DYNAMIC_VOLUME} ${STATIC_VOLUME}: CXXFLAGS=${BASE_CXXFLAGS}
${DYNAMIC_VOLUME} ${STATIC_VOLUME}: ASFLAGS=${BASE_ASFLAGS}
${DYNAMIC_VOLUME} ${STATIC_VOLUME}: LIBS=${BASE_LIBS}

# List of needed object files (from list of source files)
VOLUME_OBJS=
VOLUME_OBJS+=${BUILD_TMP}/Volume/Cipher.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/EncryptionAlgorithm.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/EncryptionMode.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/EncryptionModeCBC.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/EncryptionModeLRW.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/EncryptionModeXTS.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/EncryptionTest.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/EncryptionThreadPool.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/Hash.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/Keyfile.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/Pkcs5Kdf.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/Volume.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/VolumeException.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/VolumeHeader.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/VolumeInfo.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/VolumeLayout.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/VolumePassword.o
VOLUME_OBJS+=${BUILD_TMP}/Volume/VolumePasswordCache.o
ifeq "${TARGET_ARCH}" "x86"
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Aes_x86.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Aes_hw_cpu.o
ifeq "${TARGET_PLATFORM}" "mac"
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Aescrypt.o
endif
else ifeq "${TARGET_ARCH}" "x86_64"
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Aes_x64.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Aes_hw_cpu.o
else
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Aescrypt.o
endif
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Aeskey.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Aestab.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Blowfish.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Cast.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Des.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Rmd160.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Serpent.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Sha1.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Sha2.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Twofish.o
VOLUME_OBJS+=${BUILD_TMP}/Crypto/Whirlpool.o
VOLUME_OBJS+=${BUILD_TMP}/Common/Crc.o
VOLUME_OBJS+=${BUILD_TMP}/Common/Endian.o
VOLUME_OBJS+=${BUILD_TMP}/Common/GfMul.o
VOLUME_OBJS+=${BUILD_TMP}/Common/Pkcs5.o
VOLUME_OBJS+=${BUILD_TMP}/Common/SecurityToken.o
${DYNAMIC_VOLUME} ${STATIC_VOLUME}: OBJS=${VOLUME_OBJS}

# Dynamic library target
${DYNAMIC_VOLUME}: $${OBJS}
	$(DYNLIB_RECIPE)

# Static library target
${STATIC_VOLUME}: $${OBJS}
# (Static libs handled by common.mk)
