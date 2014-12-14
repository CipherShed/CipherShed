#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd src
devenv.exe CipherShed.sln /build
