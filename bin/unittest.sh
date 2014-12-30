#!/bin/bash -x
DIR="$(dirname "$0")/../"
cd "$DIR"/src/unit-tests/

../../bin/unittest-compile.sh && \
\
./unittesting && for i in *.gcda; do gcov `basename $i .gcda`; done
