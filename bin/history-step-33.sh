#!/bin/bash

FILE=TrueCrypt\ 6.1a\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 6.1a\ Source.zip'' -d src/'
DATE='Mon, 01 Dec 2008 18:39:44 +0100'
VER=6.1a
FIXPATHCMD=true
VERS=false
VERE=true
STEP=33;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

