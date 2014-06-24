#!/bin/bash

FILE=truecrypt-4.1-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-4.1-source-code.zip'' -d .'
DATE='Sat, 26 Nov 2005 01:13:04 +0100'
VER=4.1
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=false
VERE=true
STEP=13;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

