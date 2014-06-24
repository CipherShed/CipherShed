#!/bin/bash

FILE=TrueCrypt\ 5.1\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 5.1\ Source.tar.gz'' -C .'
DATE='Mon, 10 Mar 2008 20:32:44 -0500'
VER=5.1
FIXPATHCMD='rsync -a truecrypt-5.1-source/ src; rm -rf truecrypt-5.1-source/'
VERS=false
VERE=true
STEP=23;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

