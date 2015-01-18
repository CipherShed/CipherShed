#!/bin/bash
DIR="$(dirname "$0")/../"
cd "$DIR"/
(git clean -ndX ; git clean -nd) | sort -u | sed 's/^Would remove //' | xargs -rd '\n' tar c
