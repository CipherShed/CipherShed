#!/bin/bash -x
DIR="$(dirname "$0")/../"
cd "$DIR"
. etc/env.sh
doxygen etc/Doxyfile




