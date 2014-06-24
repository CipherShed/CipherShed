#!/bin/bash

FILE=truecrypt-4.0-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-4.0-source-code.zip'' -d .'
DATE='Tue, 01 Nov 2005 23:16:18 +0100'
VER=4.0
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=false
VERE=true
STEP=11;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

