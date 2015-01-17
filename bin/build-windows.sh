#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd src
#/cygdrive/c/programs.i386/Microsoft\ Visual\ Studio\ 9.0/Common7/IDE/devenv.com CipherShed.sln /build
#cygstart devenv.com CipherShed.sln /build
cygstart -vw devenv.exe CipherShed.sln /build
