#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd "$(dirname "$0")/../"
cd src
"$DEVENVcon" unit-tests/unit-tests.vcproj /build "All Debug"
"$MSTEST" /testcontainer:Debug/unit-tests.dll
