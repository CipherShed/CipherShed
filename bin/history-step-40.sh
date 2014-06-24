#!/bin/bash

FILE=TrueCrypt\ 6.3a\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 6.3a\ Source.zip'' -d src/'
DATE='Mon, 23 Nov 2009 18:30:24 +0100'
VER=6.3a
FIXPATHCMD=true
VERS=true
VERE=false
STEP=40;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

