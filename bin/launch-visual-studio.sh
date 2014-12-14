#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd src
cygstart devenv.exe CipherShed.sln
