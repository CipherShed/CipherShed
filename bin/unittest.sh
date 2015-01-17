#!/bin/bash -x
DIR="$(dirname "$0")/../"
cd "$DIR"/src/unit-tests/

../../bin/unittest-compile.sh && \
set +x && \
./unittesting || exit $?

(
 find ../ -iname '*.cpp' -o -iname '*.c' -o -iname '*.h' | grep -v ^../unit-tests/ | while read line; do echo -e "File '$line'\nLines executed:0.00% of -1\n"; done
 for i in *.gcda; do gcov `basename $i .gcda`; done
) | ../../bin/unittest-coverage-parse.pl 
