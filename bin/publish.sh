#!/bin/bash -x

usage()
{
  set +x
  echo "Usage:"
  echo -e "\t$0 {publishDir} {builder-id}"
}

if [ "x$1" == "x" ]; then
	1>&2 echo missing publish dir
	1>&2 usage
	exit 1
fi

if [ ! -d "$1" ]; then
	1>&2 echo publish not a dir or does not exist
	1>&2 usage
	exit 1
fi

if [ "x$2" == "x" ]; then
	1>&2 echo builder-id is missing
	1>&2 usage
	exit 1
fi	

BUILDERID="$2"

PUBDIRROOT="$(cd "$1"; pwd)"

cd "$(dirname "$0")/../"

HASH="$(git rev-parse --verify HEAD)"

VERSION=$(echo '_CS_VERSION_DOTTED_5STR' | cpp -P -x c -include "src/include/version.h")

DATE=$(date +%s)

BRANCH=v$VERSION-$(date -d @$DATE +%Y%m%d)


pushd "$PUBDIRROOT"

git checkout --orphan $BRANCH || exit $?

git rm --cached -r . || exit $?

popd 

PUBDIR="$PUBDIRROOT/builds/$HASH/$BUILDERID/$DATE/"

mkdir -p "$PUBDIR" || exit $?

./bin/artifacts-cp.sh "$PUBDIR"

pushd "$PUBDIRROOT"

git add "$PUBDIR"

git commit -m 'pre-build baseline - not a signed commit'

popd

./bin/build-windows.sh "Debug"

./bin/artifacts-cp.sh "$PUBDIR"

pushd "$PUBDIRROOT"

git add "$PUBDIR"

git commit -m 'debug build - not a signed commit'

popd

./bin/build-windows.sh "All"

./bin/artifacts-cp.sh "$PUBDIR"

pushd "$PUBDIRROOT"

git add "$PUBDIR"

git commit -m 'release build - not a signed commit'

popd

./bin/sign.sh

./bin/artifacts-cp.sh "$PUBDIR"

pushd "$PUBDIRROOT"

git add "$PUBDIR"

git commit -m 'signed EXEs - not a signed commit'

popd
