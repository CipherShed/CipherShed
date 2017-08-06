#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd "$(dirname "$0")/../src"
declare -a BUILDS
[ $# -eq 0 ] && BUILDS=("All Debug" "All") || BUILDS=("$@")
for BUILD in "${BUILDS[@]}";
do
	"$DEVENVcon" CipherShed.sln /build "$BUILD" || exit $?
done
