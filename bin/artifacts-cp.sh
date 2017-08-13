#!/bin/bash
DIR="$(dirname "$0")/../"
TDIR="$1"
if [ ! -d "$TDIR" ]; then
	echo $TDIR does not exist or is not a directory
	exit 1
fi

if [[ ! "$TDIR" = /* ]]; then
	TDIR="$(pwd)"/"$TDIR"
fi

cd "$DIR"/

./bin/artifacts-tar.sh | (cd "$TDIR" && tar xv)
