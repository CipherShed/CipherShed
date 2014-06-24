#!/bin/bash

FILE=truecrypt-1.0-source-code.zip
EXTCMD='unzip -qo files/''truecrypt-1.0-source-code.zip'' -d src/'
DATE='Mon, 02 Feb 2004 11:10:28 +0100'
VER=1.0
FIXPATHCMD=true
VERS=false
VERE=true
STEP=01;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

