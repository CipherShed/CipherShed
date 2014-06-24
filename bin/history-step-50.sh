#!/bin/bash

FILE=TrueCrypt-7.2-source.zip
EXTCMD='unzip -qo files/''TrueCrypt-7.2-source.zip'' -d src/'
DATE='Tue, 27 May 2014 17:33:50 +0100'
VER=7.2
FIXPATHCMD=true
VERS=true
VERE=false
STEP=50;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

