#!/bin/bash

FILE=truecrypt-4.2-source-code.tar.gz
EXTCMD='tar xf files/''truecrypt-4.2-source-code.tar.gz'' -C .'
DATE='Sun, 16 Apr 2006 21:19:30 +0100'
VER=4.2
FIXPATHCMD='rsync -a truecrypt-4.2/ src; rm -rf truecrypt-4.2/'
VERS=true
VERE=false
STEP=14;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

