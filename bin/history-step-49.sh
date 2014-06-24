#!/bin/bash

FILE=TrueCrypt\ 7.1a\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 7.1a\ Source.zip'' -d src/'
DATE='Tue, 07 Feb 2012 11:37:10 +0100'
VER=7.1a
FIXPATHCMD=true
VERS=false
VERE=true
STEP=49;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

