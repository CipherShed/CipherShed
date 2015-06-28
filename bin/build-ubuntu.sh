#!/bin/bash -x
. "$(dirname "$0")/../etc/env.sh"
cd src
make -j $(nproc)
