#!/bin/bash

FILE=truecrypt-1.0a-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-1.0a-source-code.zip'' -d src/'
DATE='Tue, 03 Feb 2004 10:41:48 +0100'
VER=1.0a
FIXPATHCMD=true
VERS=true
VERE=true
STEP=02;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

