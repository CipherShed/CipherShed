#!/bin/bash

FILE=truecrypt-4.1-source-code.tar.gz
EXTCMD='tar xf files/''truecrypt-4.1-source-code.tar.gz'' -C .'
DATE='Sat, 26 Nov 2005 00:08:28 +0100'
VER=4.1
FIXPATHCMD='rsync -a truecrypt-4.1-source-code/ src; rm -rf truecrypt-4.1-source-code/'
VERS=true
VERE=false
STEP=12;STEPFV=21
AUTOFIXEOL=false

. $(dirname $0)/history-common-steps.sh

