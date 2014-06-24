#!/bin/bash

FILE=TrueCrypt\ 7.0\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 7.0\ Source.zip'' -d src/'
DATE='Mon, 19 Jul 2010 20:51:38 +0200'
VER=7.0
FIXPATHCMD=true
VERS=false
VERE=true
STEP=43;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

