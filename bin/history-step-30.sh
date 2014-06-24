#!/bin/bash

FILE=TrueCrypt\ 6.1\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 6.1\ Source.tar.gz'' -C .'
DATE='Fri, 31 Oct 2008 22:19:24 +0100'
VER=6.1
FIXPATHCMD='rsync -a truecrypt-6.1-source/ src; rm -rf truecrypt-6.1-source/'
VERS=true
VERE=false
STEP=30;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

