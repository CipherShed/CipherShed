#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd src
cygstart -vw devenv.exe CipherShed.sln /build
