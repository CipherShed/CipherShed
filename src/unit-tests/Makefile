#ODIR = ../../tmp/unittests
CC = g++
CommonSDIR = ../Common

CFLAGS = -DCS_UNITTESTING -fprofile-arcs -ftest-coverage -I . -I ../../var/opt/cpptest-code/cpptest/src/
CXXFLAGS = $(CFLAGS)

#sort as "LC_ALL=C sort -u"

SRC_c = \
../Common/Crc.c \
../Common/Endian.c \
../Common/GfMul.c \
../Common/Password.c \
../Common/dialog/cursor.c \
../Common/strcpys.c \
../Common/util/unicode/ConvertUTF.c \
../Common/util/unicode/strcmpw.c \
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
../Common/dialog/errors.cpp \
../Common/dialog/userperms.cpp \
../Main/System.cpp \
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
faux/windows/SetFilePointer.cpp \
faux/windows/SetFileTime.cpp \
faux/windows/SetLastError.cpp \
unittesting.cpp \
# end of SRC_cpp



.PHONY: clean gcov coverage.xml run unittests

coverage.xml: ../../doc/devdocs/generated/reports/coverage.xml

../../doc/devdocs/generated/reports/coverage.xml: run
	mkdir -p ../../doc/devdocs/generated/reports/
	gcovr --xml -b -k -r ../../ -o ../../doc/devdocs/generated/reports/coverage.xml -v -s -e '^src/unit-tests/' -e '^var/'

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
