#!/bin/bash -x
DIR="$(dirname "$0")/../"
cd "$DIR"/src/unit-tests/

g++ unittesting.cpp                                         \
    -DCS_UNITTESTING                                        \
    -fprofile-arcs -ftest-coverage                          \
                                                            \
    ../Common/Crc.c                                         \
    ../Common/Endian.c                                      \
    ../Common/Password.c                                    \
    ../Common/strcpys.c                                     \
    ../Common/util/unicode/ConvertUTF.c                     \
                                                            \
    faux/windows/CloseHandle.cpp                            \
    faux/windows/CreateFile.cpp                             \
    faux/windows/DeviceIoControl.cpp                        \
    faux/windows/EnableWindow.cpp                           \
    faux/windows/FlushFileBuffers.cpp                       \
    faux/windows/GetFileSize.cpp                            \
    faux/windows/GetFileTime.cpp                            \
    faux/windows/GetLastError.cpp                           \
    faux/windows/GetWindowText.cpp                          \
    faux/windows/GetWindowTextLength.cpp                    \
    faux/windows/SetFilePointer.cpp                         \
    faux/windows/SetFileTime.cpp                            \
    faux/windows/SetLastError.cpp                           \
    faux/windows/MessageBox.c                               \
    faux/ciphershed/wip.cpp                                 \
                                                            \
        -I .                                                \
        -I ../../var/opt/cpptest-code/cpptest/src/          \
../../var/opt/cpptest-code/cpptest/src/source.cpp           \
../../var/opt/cpptest-code/cpptest/src/suite.cpp            \
../../var/opt/cpptest-code/cpptest/src/textoutput.cpp       \
../../var/opt/cpptest-code/cpptest/src/time.cpp             \
../../var/opt/cpptest-code/cpptest/src/utils.cpp            \
../../var/opt/cpptest-code/cpptest/src/collectoroutput.cpp  \
../../var/opt/cpptest-code/cpptest/src/compileroutput.cpp   \
../../var/opt/cpptest-code/cpptest/src/htmloutput.cpp       \
../../var/opt/cpptest-code/cpptest/src/missing.cpp          \
-o unittesting

exit $?

