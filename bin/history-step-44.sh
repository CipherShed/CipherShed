#!/bin/bash

FILE=TrueCrypt\ 7.0a\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 7.0a\ Source.tar.gz'' -C .'
DATE='Sun, 05 Sep 2010 14:32:52 +0200'
VER=7.0a
FIXPATHCMD='rsync -a truecrypt-7.0a-source/ src; rm -rf truecrypt-7.0a-source/'
VERS=true
VERE=false
STEP=44;STEPFV=21
AUTOFIXEOL=false

. $(dirname $0)/history-common-steps.sh

