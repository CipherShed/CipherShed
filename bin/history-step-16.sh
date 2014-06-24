#!/bin/bash

FILE=truecrypt-4.2a-source-code.tar.gz
EXTCMD='tar xf files/''truecrypt-4.2a-source-code.tar.gz'' -C .'
DATE='Mon, 03 Jul 2006 12:34:10 +0100'
VER=4.2a
FIXPATHCMD='rsync -a truecrypt-4.2a/ src; rm -rf truecrypt-4.2a/'
VERS=true
VERE=false
STEP=16;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

