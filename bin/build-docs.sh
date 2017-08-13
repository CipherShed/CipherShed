#!/bin/bash -x
DIR="$(dirname "$0")/../"
cd "$DIR"
. etc/env.sh
doxygen.exe etc/Doxyfile




