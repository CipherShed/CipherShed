#!/bin/bash

FILE=truecrypt-3.1a-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-3.1a-source-code.zip'' -d .'
DATE='Mon, 07 Feb 2005 00:49:46 +0100'
VER=3.1a
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=true
VERE=true
STEP=09;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

