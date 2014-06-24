#!/bin/bash

FILE=truecrypt-4.3-source-code.tar.gz
EXTCMD='tar xf files/''truecrypt-4.3-source-code.tar.gz'' -C .'
DATE='Mon, 19 Mar 2007 15:33:16 +0100'
VER=4.3
FIXPATHCMD='rsync -a truecrypt-4.3-source-code/ src; rm -rf truecrypt-4.3-source-code/'
VERS=true
VERE=false
STEP=18;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

