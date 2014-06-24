#!/bin/bash

FILE=TrueCrypt\ 6.3\ Source.zip
EXTCMD='unzip -qo files/''TrueCrypt\ 6.3\ Source.zip'' -d src/'
DATE='Wed, 21 Oct 2009 15:47:36 +0200'
VER=6.3
FIXPATHCMD=true
VERS=false
VERE=true
STEP=39;STEPFV=20
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

