#!/bin/bash

FILE=TrueCrypt\ 5.0\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 5.0\ Source.zip'' -d .'
DATE='Tue, 05 Feb 2008 10:18:42 +0100'
VER=5.0
FIXPATHCMD='rsync -a TrueCrypt/ src; rm -rf TrueCrypt/'
VERS=true
VERE=false
STEP=20;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

