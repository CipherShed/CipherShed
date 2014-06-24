#!/bin/bash

FILE=TrueCrypt\ 5.0\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 5.0\ Source.tar.gz'' -C .'
DATE='Wed, 06 Feb 2008 12:19:14 +0100'
VER=5.0
FIXPATHCMD='rsync -a truecrypt-5.0-source/ src; rm -rf truecrypt-5.0-source/'
VERS=false
VERE=true
STEP=21;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

