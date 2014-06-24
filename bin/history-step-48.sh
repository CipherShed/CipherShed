#!/bin/bash

FILE=TrueCrypt\ 7.1a\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 7.1a\ Source.tar.gz'' -C .'
DATE='Tue, 07 Feb 2012 10:37:04 +0100'
VER=7.1a
FIXPATHCMD='rsync -a truecrypt-7.1a-source/ src; rm -rf truecrypt-7.1a-source/'
VERS=true
VERE=false
STEP=48;STEPFV=21
AUTOFIXEOL=false

. $(dirname $0)/history-common-steps.sh

