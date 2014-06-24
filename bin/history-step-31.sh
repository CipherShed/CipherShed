#!/bin/bash

FILE=TrueCrypt\ 6.1\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 6.1\ Source.zip'' -d src/'
DATE='Fri, 31 Oct 2008 23:31:22 +0100'
VER=6.1
FIXPATHCMD=true
VERS=false
VERE=true
STEP=31;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

