#!/bin/bash

FILE=truecrypt-2.0-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-2.0-source-code.zip'' -d .'
DATE='Mon, 07 Jun 2004 12:42:08 +0100'
VER=2.0
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=true
VERE=true
STEP=03;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

