#!/bin/bash

FILE=TrueCrypt-7.2-source-unix.tar.gz
EXTCMD='tar xf files/''TrueCrypt-7.2-source-unix.tar.gz'' -C .'
DATE='Tue, 27 May 2014 16:36:03 +0100'
VER=7.2
FIXPATHCMD='rsync -a truecrypt-7.2-source/ src; rm -rf truecrypt-7.2-source/'
VERS=false
VERE=true
STEP=51;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

