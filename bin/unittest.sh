#!/bin/bash -x
DIR="$(dirname "$0")/../"
cd "$DIR"/src/unit-tests/
make coverage.xml
