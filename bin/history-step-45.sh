#!/bin/bash

FILE=TrueCrypt\ 7.0a\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 7.0a\ Source.zip'' -d src/'
DATE='Sun, 05 Sep 2010 16:35:22 +0200'
VER=7.0a
FIXPATHCMD=true
VERS=false
VERE=true
STEP=45;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

