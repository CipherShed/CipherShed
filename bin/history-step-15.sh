#!/bin/bash

FILE=truecrypt-4.2-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-4.2-source-code.zip'' -d .'
DATE='Sun, 16 Apr 2006 23:24:46 +0100'
VER=4.2
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=false
VERE=true
STEP=15;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

