#!/bin/bash

FILE=truecrypt-4.3-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-4.3-source-code.zip'' -d .'
DATE='Mon, 19 Mar 2007 16:37:00 +0100'
VER=4.3
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=false
VERE=true
STEP=19;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

