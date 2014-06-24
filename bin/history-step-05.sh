#!/bin/bash

FILE=truecrypt-2.1a-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-2.1a-source-code.zip'' -d .'
DATE='Fri, 01 Oct 2004 16:53:36 +0100'
VER=2.1a
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=true
VERE=true
STEP=05;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

