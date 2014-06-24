#!/bin/bash

FILE=TrueCrypt\ 5.1\ source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 5.1\ source.zip'' -d src/'
DATE='Mon, 10 Mar 2008 15:27:30 -0500'
VER=5.1
FIXPATHCMD=true
VERS=true
VERE=false
STEP=22;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

