#!/bin/bash

FILE=TrueCrypt\ 6.2\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 6.2\ Source.zip'' -d src/'
DATE='Sun, 10 May 2009 22:06:46 +0200'
VER=6.2
FIXPATHCMD=true
VERS=false
VERE=true
STEP=35;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

