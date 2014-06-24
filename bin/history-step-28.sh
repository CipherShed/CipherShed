#!/bin/bash

FILE=TrueCrypt\ 6.0a\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 6.0a\ Source.tar.gz'' -C .'
DATE='Tue, 08 Jul 2008 15:24:06 +0200'
VER=6.0a
FIXPATHCMD='rsync -a truecrypt-6.0a-source/ src; rm -rf truecrypt-6.0a-source/'
VERS=true
VERE=false
STEP=28;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

