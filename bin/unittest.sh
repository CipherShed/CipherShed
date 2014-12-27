#!/bin/bash -x
DIR="$(dirname "$0")/../"
cd "$DIR"/src/unit-tests/

g++ unittesting.cpp                                         \
    -DCS_UNITTESTING                                        \
    -fprofile-arcs -ftest-coverage                          \
                                                            \
    ../Common/Crc.c                                         \
    ../Common/Endian.c                                      \
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
-o unittesting &&\
\
./unittesting && for i in *.gcda; do gcov `basename $i .gcda`; done
