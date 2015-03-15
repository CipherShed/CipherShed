#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd src
"$DEVENVcon" CipherShed.sln /build "All Debug"
"$DEVENVcon" CipherShed.sln /build "All"
