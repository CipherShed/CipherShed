#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd "$(dirname "$0")/../"
cd src
"$DEVENVcon" CipherShed.sln /build "All Debug" /project unit-tests
"$MSTEST" /testcontainer:Debug/unit-tests.dll
