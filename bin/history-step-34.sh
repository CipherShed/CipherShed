#!/bin/bash

FILE=TrueCrypt\ 6.2\ Source.tar.gz
EXTCMD='tar xf files/''TrueCrypt\ 6.2\ Source.tar.gz'' -C .'
DATE='Sun, 10 May 2009 20:00:20 +0200'
VER=6.2
FIXPATHCMD='rsync -a truecrypt-6.2-source/ src; rm -rf truecrypt-6.2-source/'
VERS=true
VERE=false
STEP=34;STEPFV=21
AUTOFIXEOL=true

. $(dirname $0)/history-common-steps.sh

