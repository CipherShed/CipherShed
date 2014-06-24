#!/bin/bash

FILE=truecrypt-3.0a-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-3.0a-source-code.zip'' -d .'
DATE='Sat, 11 Dec 2004 19:55:36 +0100'
VER=3.0a
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=true
VERE=true
STEP=07;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

