#!/bin/bash

FILE=truecrypt-4.0-source-code.tar.gz
EXTCMD='tar xf files/''truecrypt-4.0-source-code.tar.gz'' -C .'
DATE='Tue, 01 Nov 2005 22:14:16 +0100'
VER=4.0
FIXPATHCMD='rsync -a truecrypt-4.0/ src; rm -rf truecrypt-4.0/'
VERS=true
VERE=false
STEP=10;STEPFV=21
AUTOFIXEOL=false

. $(dirname $0)/history-common-steps.sh

