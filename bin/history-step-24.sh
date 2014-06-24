#!/bin/bash

FILE=TrueCrypt\ 5.1a\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 5.1a\ Source.tar.gz'' -C .'
DATE='Sun, 16 Mar 2008 19:39:08 +0100'
VER=5.1a
FIXPATHCMD='rsync -a truecrypt-5.1a-source/ src; rm -rf truecrypt-5.1a-source/'
VERS=true
VERE=false
STEP=24;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

