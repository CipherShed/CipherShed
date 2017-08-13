#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd "$(dirname "$0")/../src"
make -j $(nproc)
