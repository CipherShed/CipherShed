#!/bin/bash

FILE=TrueCrypt\ 7.1\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 7.1\ Source.zip'' -d src/'
DATE='Thu, 01 Sep 2011 15:00:24 +0200'
VER=7.1
FIXPATHCMD=true
VERS=true
VERE=false
STEP=46;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

