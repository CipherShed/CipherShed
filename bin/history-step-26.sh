#!/bin/bash

FILE=TrueCrypt\ 6.0\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 6.0\ Source.zip'' -d src/'
DATE='Fri, 04 Jul 2008 23:15:32 +0400'
VER=6.0
FIXPATHCMD=true
VERS=true
VERE=false
STEP=26;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

