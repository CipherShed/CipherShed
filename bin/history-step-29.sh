#!/bin/bash

FILE=TrueCrypt\ 6.0a\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 6.0a\ Source.zip'' -d src/'
DATE='Tue, 08 Jul 2008 17:25:24 +0200'
VER=6.0a
FIXPATHCMD=true
VERS=false
VERE=true
STEP=29;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

