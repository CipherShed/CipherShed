#!/bin/bash


FILE=TrueCrypt\ 6.3a\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 6.3a\ Source.tar.gz'' -C .'
DATE='Mon, 23 Nov 2009 17:42:48 +0100'
VER=6.3a
FIXPATHCMD='rsync -a truecrypt-6.3a-source/ src; rm -rf truecrypt-6.3a-source/'
VERS=false
VERE=true
STEP=41;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh
