#!/bin/bash

FILE=TrueCrypt\ 7.0\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 7.0\ Source.tar.gz'' -C .'
DATE='Mon, 19 Jul 2010 18:34:56 +0200'
VER=7.0
FIXPATHCMD='rsync -a truecrypt-7.0-source/ src; rm -rf truecrypt-7.0-source/'
VERS=true
VERE=false
STEP=42;STEPFV=21
AUTOFIXEOL=false

. $(dirname $0)/history-common-steps.sh

