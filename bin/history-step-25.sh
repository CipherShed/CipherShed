#!/bin/bash

FILE=TrueCrypt\ 5.1a\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 5.1a\ Source.zip'' -d src/'
DATE='Sun, 16 Mar 2008 20:52:50 +0100'
VER=5.1a
FIXPATHCMD=true
VERS=false
VERE=true
STEP=25;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

