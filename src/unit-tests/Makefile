#ODIR = ../../tmp/unittests
CC = g++
CommonSDIR = ../Common

CFLAGS = -DCS_UNITTESTING -fprofile-arcs -ftest-coverage -I . -I ../../var/opt/cpptest-code/cpptest/src/ -lpthread -ldl
CXXFLAGS = $(CFLAGS)

CPPFLAGS += -MD -MP

GCOVR = ../../var/opt/gcovr/scripts/gcovr

#sort as "LC_ALL=C sort -u"

SRC_c = \
../Common/Crc.c \
../Common/Endian.c \
../Common/GfMul.c \
../Common/Password.c \
../Common/Pkcs5.c \
../Common/dialog/cursor.c \
../Common/strcpys.c \
../Common/util/unicode/ConvertUTF.c \
../Common/util/unicode/strcmpw.c \
../Crypto/Aescrypt.c \
../Crypto/Aeskey.c \
../Crypto/Blowfish.c \
../Crypto/Cast.c \
../Crypto/Des.c \
../Crypto/Rmd160.c \
../Crypto/Serpent.c \
../Crypto/Sha1.c \
../Crypto/Sha2.c \
../Crypto/Twofish.c \
../Crypto/Whirlpool.c \
faux/windows/CreateWindowEx.c \
faux/windows/DefWindowProc.c \
faux/windows/DestroyWindow.c \
faux/windows/FormatMessage.c \
faux/windows/GetCurrentProcess.c \
faux/windows/GetModuleHandle.c \
faux/windows/GetProcAddress.c \
faux/windows/GetSystemMetrics.c \
faux/windows/GetTokenInformation.c \
faux/windows/GetVersionEx.c \
faux/windows/IsUserAnAdmin.c \
faux/windows/IsWellKnownSid.c \
faux/windows/LoadCursor.c \
faux/windows/LocalFree.c \
faux/windows/MessageBox.c \
faux/windows/OpenProcessToken.c \
faux/windows/RegOpenKeyEx.c \
faux/windows/RegisterClassEx.c \
faux/windows/SetCursor.c \
faux/windows/SetLayeredWindowAttributes.c \
faux/windows/ShellNotifyIcon.c \
faux/windows/ShowWindow.c \
faux/windows/UnregisterClass.c \
# end of SRC_c

SRC_cpp = \
../../var/opt/cpptest-code/cpptest/src/collectoroutput.cpp \
../../var/opt/cpptest-code/cpptest/src/compileroutput.cpp \
../../var/opt/cpptest-code/cpptest/src/htmloutput.cpp \
../../var/opt/cpptest-code/cpptest/src/missing.cpp \
../../var/opt/cpptest-code/cpptest/src/source.cpp \
../../var/opt/cpptest-code/cpptest/src/suite.cpp \
../../var/opt/cpptest-code/cpptest/src/textoutput.cpp \
../../var/opt/cpptest-code/cpptest/src/time.cpp \
../../var/opt/cpptest-code/cpptest/src/utils.cpp \
../Common/SecurityToken.cpp \
../Common/dialog/errors.cpp \
../Common/dialog/userperms.cpp \
../Common/volume/volutil.cpp \
../Core/CoreBase.cpp \
../Core/CoreException.cpp \
../Core/FatFormatter.cpp \
../Core/HostDevice.cpp \
../Core/MountOptions.cpp \
../Core/RandomNumberGenerator.cpp \
../Main/System.cpp \
../Platform/Buffer.cpp \
../Platform/Exception.cpp \
../Platform/FileCommon.cpp \
../Platform/Memory.cpp \
../Platform/MemoryStream.cpp \
../Platform/Serializable.cpp \
../Platform/Serializer.cpp \
../Platform/SerializerFactory.cpp \
../Platform/StringConverter.cpp \
../Platform/Unix/Directory.cpp \
../Platform/Unix/File.cpp \
../Platform/Unix/FilesystemPath.cpp \
../Platform/Unix/Mutex.cpp \
../Platform/Unix/Pipe.cpp \
../Platform/Unix/SyncEvent.cpp \
../Platform/Unix/SystemException.cpp \
../Platform/Unix/SystemLog.cpp \
../Platform/Unix/Thread.cpp \
../Platform/Unix/Time.cpp \
../Volume/Cipher.cpp \
../Volume/EncryptionAlgorithm.cpp \
../Volume/EncryptionMode.cpp \
../Volume/EncryptionModeCBC.cpp \
../Volume/EncryptionModeLRW.cpp \
../Volume/EncryptionModeXTS.cpp \
../Volume/EncryptionTest.cpp \
../Volume/EncryptionThreadPool.cpp \
../Volume/Hash.cpp \
../Volume/Keyfile.cpp \
../Volume/Pkcs5Kdf.cpp \
../Volume/Volume.cpp \
../Volume/VolumeException.cpp \
../Volume/VolumeHeader.cpp \
../Volume/VolumeInfo.cpp \
../Volume/VolumeLayout.cpp \
../Volume/VolumePassword.cpp \
../Volume/VolumePasswordCache.cpp \
faux/ciphershed/wip.cpp \
faux/windows/CloseHandle.cpp \
faux/windows/CreateFile.cpp \
faux/windows/DeviceIoControl.cpp \
faux/windows/EnableWindow.cpp \
faux/windows/FlushFileBuffers.cpp \
faux/windows/GetFileSize.cpp \
faux/windows/GetFileTime.cpp \
faux/windows/GetLastError.cpp \
faux/windows/GetWindowText.cpp \
faux/windows/GetWindowTextLength.cpp \
faux/windows/OutputDebugString.cpp \
faux/windows/QueryDosDevice.cpp \
faux/windows/SetFilePointer.cpp \
faux/windows/SetFileTime.cpp \
faux/windows/SetLastError.cpp \
faux/windows/_wcsicmp.cpp \
faux/windows/swprintf_s.cpp \
unittesting.cpp \
# end of SRC_cpp

SRCS=$(SRC_c) $(SRC_cpp)

.PHONY: clean gcov coverage.xml run unittests

coverage.xml: ../../doc/devdocs/generated/reports/coverage.xml

../../doc/devdocs/generated/reports/coverage.xml: run
	mkdir -p ../../doc/devdocs/generated/reports/
	$(GCOVR) --xml -b -d -r ../../ -o ../../doc/devdocs/generated/reports/coverage.xml -v -s \
	-e '^src/unit-tests/' -e '^var/' -e '^.git/' -e '^doc/' -e '^tmp/' -e '^etc/' -i '.*\.(c|cpp)$$'

run: unittests
	./unittesting

unittests: unittesting

OBJ=$(SRC_c:.c=.o) $(SRC_cpp:.cpp=.o)

compile: compile_c compile_cpp

compile_c: $(SRC_c:.c=.o)

compile_cpp: $(SRC_cpp:.cpp=.o)

unittesting: $(OBJ)
	$(CC) $(CFLAGS) --disable-stdcall-fixup -o $@ $^

clean:
#	rm -rf $(ODIR)/

-include $(SRC_c:%.c=%.d) $(SRC_cpp:%.cpp=%.d)

