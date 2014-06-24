#!/bin/bash

FILE=TrueCrypt\ 6.2a\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 6.2a\ Source.zip'' -d src/'
DATE='Mon, 15 Jun 2009 12:17:50 +0200'
VER=6.2a
FIXPATHCMD=true
VERS=false
VERE=true
STEP=37;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

