#!/bin/bash

FILE=TrueCrypt\ 7.1\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 7.1\ Source.tar.gz'' -C .'
DATE='Thu, 01 Sep 2011 13:16:06 +0200'
VER=7.1
FIXPATHCMD='rsync -a truecrypt-7.1-source/ src; rm -rf truecrypt-7.1-source/'
VERS=false
VERE=true
STEP=47;STEPFV=21
AUTOFIXEOL=false

. $(dirname $0)/history-common-steps.sh

