#!/bin/bash

FILE=TrueCrypt\ 6.0\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 6.0\ Source.tar.gz'' -C .'
DATE='Fri, 04 Jul 2008 19:41:18 +0400'
VER=6.0
FIXPATHCMD='rsync -a truecrypt-6.0-source/ src; rm -rf truecrypt-6.0-source/'
VERS=false
VERE=true
STEP=27;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

