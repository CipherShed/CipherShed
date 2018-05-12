#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
DIR="$(dirname "$0")/.."
cd "$DIR"/src
cygstart "$DEVENV" CipherShed.sln
