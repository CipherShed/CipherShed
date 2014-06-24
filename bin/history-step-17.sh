#!/bin/bash

FILE=truecrypt-4.2a-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-4.2a-source-code.zip'' -d .'
DATE='Mon, 03 Jul 2006 14:37:02 +0100'
VER=4.2a
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=false
VERE=true
STEP=17;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

