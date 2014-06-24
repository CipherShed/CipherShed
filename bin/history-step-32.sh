#!/bin/bash

FILE=TrueCrypt\ 6.1a\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 6.1a\ Source.tar.gz'' -C .'
DATE='Mon, 01 Dec 2008 17:37:38 +0100'
VER=6.1a
FIXPATHCMD='rsync -a truecrypt-6.1a-source/ src; rm -rf truecrypt-6.1a-source/'
VERS=true
VERE=false
STEP=32;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

