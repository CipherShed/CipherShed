#!/bin/bash -x

ODIR="$(pwd)"
cd "$(dirname "$0")/../"
BDIR="$(pwd)"

# TODO: Move version rendering to external caller script so when
# Jenkins calls this script on multiple machines, the Jenkins machine
# can provide the same version number to all machines, so they all have
# the same version number.

# TODO: Collect output files and clean up after we've uploaded them

# If there is less than 2 arguments, show error as requirements are not met
# (if there is more, they will be silently ignored)
if [ "$#" -lt 1 ] ; then
echo 'This script takes 2 manditory aguments.
        1: gpg key-id to sign with
        2: destdir (made if needed)'
exit 1
fi

pushd "$ODIR" > /dev/null
if ! mkdir -p "$2"; then
	echo could not access $2
	exit 2
fi
cd "$2"
DESTDIR="$(pwd)"
popd > /dev/null

#sudo apt-get install nasm libfuse-dev libwxgtk2.8-dev pkgconf

# Renders package base version
DEBVERSION="0.7.3.0.0~git-$(date -u +%Y%m%d.%H%M%S)-$(git log -n 1 --pretty=format:"%h")"

# Renders initial package revision
DEBREVISION=1

# Renders source archive name
SRCDIR="$DESTDIR/ciphershed_${DEBVERSION}"
SRCZIP="$SRCDIR.orig.tar.gz"

# Archives source code for distribution
git archive HEAD --format tgz -o "${SRCZIP}"

# Creates clean ciphershed-$DEBVERSION folder for compilation
mkdir -p "$SRCDIR"
pushd "$SRCDIR" > /dev/null
tar xzf "${SRCZIP}"
#popd > /dev/null

#SCM version
dch -v "${DEBVERSION}-${DEBREVISION}" --changelog "$SRCDIR/etc/debian/changelog" --empty -D unstable "bite me"

#pushd "$SRCDIR" > /dev/null
mv etc/debian/ .
debuild -j$(nproc) -k"${1}" --source-option=--diff-ignore=etc/debian/ --source-option=--diff-ignore=debian/

popd > /dev/null

# Removes the no-longer-necessary ciphershed-$DEBVERSION folder
rm -rf "$SRCDIR/"
