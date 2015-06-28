#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd "$(dirname "$0")/../src"
"$DEVENVcon" CipherShed.sln /build "All Debug"
"$DEVENVcon" CipherShed.sln /build "All"
